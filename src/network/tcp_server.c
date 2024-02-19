#include "tcp_server.h"

#include "utils/logging.h"

err_t tcp_client_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    return err;
}

err_t tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}

err_t tcp_server_result(void *arg, int status) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    LogInfo(("Close TCP server, status %d", status));
    state->complete = true;
    return tcp_server_close(arg);
}

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    LogDebug(("tcp_server_sent %u", len));
    state->sent_len += len;

    if (state->sent_len >= MAX_BUF_SIZE) {
        // We should get the data back from the client
        state->recv_len = 0;
        LogDebug(("Waiting for buffer from client"));
    }

    return ERR_OK;
}

err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;

    state->sent_len = 0;
    for (int i = 0; i < state->packages_send_len; i++) {
        // this method is callback from lwIP, so cyw43_arch_lwip_begin is not
        // required, however you can use this method to cause an assertion in
        // debug mode, if this method is called when cyw43_arch_lwip_begin IS
        // needed
        cyw43_arch_lwip_check();
        if (xSemaphoreTake(state->buffer_mutex, (TickType_t)100) == pdTRUE) {
            err_t err =
                tcp_write(tpcb, state->buffer_sent[i],
                          state->buffer_send_len[i], TCP_WRITE_FLAG_COPY);
            xSemaphoreGive(state->buffer_mutex);
            if (err != ERR_OK) {
                LogError(("Failed to write data %d", err));
                return tcp_server_result(arg, -1);
            }
        }
    }
    return ERR_OK;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                      err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    if (!p) {
        return tcp_server_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not
    // required, however you can use this method to cause an assertion in debug
    // mode, if this method is called when cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        LogDebug(
            ("tcp_server_recv %d/%d err %d", p->tot_len, state->recv_len, err));

        // Receive the buffer
        const uint16_t buffer_left = MAX_BUF_SIZE - state->recv_len;
        state->recv_len += pbuf_copy_partial(
            p, state->buffer_recv + state->recv_len,
            p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // Have we have received the whole buffer
    if (state->recv_len == MAX_BUF_SIZE) {
        // check it matches
        if (memcmp(state->buffer_sent, state->buffer_recv, MAX_BUF_SIZE) != 0) {
            LogError(("buffer mismatch"));
            return tcp_server_result(arg, -1);
        }
        LogDebug(("tcp_server_recv buffer ok"));

        // Test complete?
        state->run_count++;
        if (state->run_count >= TEST_ITERATIONS) {
            tcp_server_result(arg, 0);
            return ERR_OK;
        }

        // Send another buffer
        return tcp_server_send_data(arg, state->client_pcb);
    }
    return ERR_OK;
}

void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        LogError(("tcp_client_err_fn %d", err));
        tcp_server_result(arg, err);
    }
}

err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        LogError(("Failure in accept"));
        tcp_server_result(arg, err);
        return ERR_VAL;
    }

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);

    err_t result = tcp_server_send_data(arg, state->client_pcb);
    return tcp_client_close(state);
}
