#!/usr/bin/env python3

import argparse
import hashlib
import math
import socket
import time

APP_FILE = "build/external/PicoW-Bootloader/example_app/PICO_BOOTLOADER_EXAMPLE_APP.bin"

BINARY_CONTENT_SIZE = 256

HOST = "192.168.50.222"
PORT = 80

headers = """\
POST /SWDW HTTP/1.1\r
Host: {host}\r
Content-Type: application/json\r
Content-Length: {content_length}\r
\r\n"""


def iterate_chunks(string, chunk_size):
    for i in range(0, len(string), chunk_size):
        chunk = string[i : i + chunk_size]
        yield chunk


def send(args, binary, hash="", size=0, send_completed=False):
    body = '{"SWDL": {'
    if hash != "":
        body += f'"hash": "{hash}", '
    if size != 0:
        body += f'"size": {size}, '
    if send_completed:
        body += '"complete": "true"}}'
    else:
        body += f'"binary": "{binary}"' + "}}"
    body_bytes = body.encode("ascii")
    header_bytes = headers.format(
        host=str(args.host) + ":" + str(args.port),
        content_length=len(body_bytes),
    ).encode("iso-8859-1")

    payload = header_bytes + body_bytes

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((args.host, args.port))
    s.sendall(payload)

    if "200 OK" in str(s.recv(1024)):
        return True

    print("Transfer failed")
    return False


def main():
    parser = argparse.ArgumentParser(
        description="Combine bootloader, generated app info and app"
    )
    parser.add_argument(
        "--app-file", default=APP_FILE, help="path to app .bin file"
    )
    parser.add_argument("--host", default=HOST, help="Host IP")
    parser.add_argument("--port", default=PORT, help="Host port")
    args = parser.parse_args()

    with open(args.app_file, "rb") as file:
        app_raw_file = file.read()
        app_hexdata = app_raw_file.hex()

    iterations = math.ceil(len(app_hexdata) / BINARY_CONTENT_SIZE)
    counter = 0
    first_iteration = True
    for chunk in iterate_chunks(app_hexdata, BINARY_CONTENT_SIZE):
        counter += 1
        print(f"Send package {counter}/{iterations}")
        if first_iteration:
            send(
                args,
                chunk,
                hashlib.sha256(app_raw_file).hexdigest(),
                len(app_raw_file),
            )
            first_iteration = False
        else:
            send(args, chunk)
        time.sleep(0.5)
    send(args, "", send_completed=True)


if __name__ == "__main__":
    main()
