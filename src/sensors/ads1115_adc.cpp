#include "ads1115_adc.h"

#include <algorithm>
#include <limits>

#include "utils/logging.h"

Ads1115Adc::Ads1115Adc()
    : m_adc_state{},
      m_min_value{18000},
      m_max_value{18500},
      m_inverse_measurement{false},
      m_calibration_complete{false},
      m_calibration_run{false},
      m_calibration_samples{0},
      m_name("") {}

bool Ads1115Adc::init(i2c_inst_t *i2c, uint8_t address) {
    LogDebug(("Initialise ads1115"));
    // Initialise ADC
    ads1115_init(i2c, address, &m_adc_state);

    LogDebug(("Set ads1115 config"));
    // Modify the default configuration as needed. In this example the
    // signal will be differential, measured between pins A0 and A3,
    // and the full-scale voltage range is set to +/- 4.096 V.
    ads1115_set_input_mux(ADS1115_MUX_SINGLE_0, &m_adc_state);
    ads1115_set_pga(ADS1115_PGA_4_096, &m_adc_state);
    ads1115_set_data_rate(ADS1115_RATE_475_SPS, &m_adc_state);

    LogDebug(("Write ads1115 config"));
    // Write the configuration for this to have an effect.
    ads1115_write_config(&m_adc_state);

    return true;
}

void Ads1115Adc::set_min_value(std::uint16_t value) { m_min_value = value; }
void Ads1115Adc::set_max_value(std::uint16_t value) { m_max_value = value; }
void Ads1115Adc::set_inverse_measurement(bool inverse_measurement) {
    m_inverse_measurement = inverse_measurement;
}
void Ads1115Adc::set_name(std::string name) { m_name = name; }

// Add pin somehow
void Ads1115Adc::read(int pin_id, float &return_value,
                      std::string &return_name) {
    return_name = m_name;

    switch (pin_id) {
        case 1:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_0, &m_adc_state);
            break;
        case 2:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_1, &m_adc_state);
            break;
        case 3:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_2, &m_adc_state);
            break;
        case 4:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_3, &m_adc_state);
            break;
    }

    return_value = 0.0f;
    std::uint16_t adc_value;

    ads1115_read_adc(&adc_value, &m_adc_state);

    if (m_calibration_run && !m_calibration_complete) {
        m_min_value = std::min(m_min_value, adc_value);
        m_max_value = std::max(m_max_value, adc_value);
        m_calibration_samples++;
        LogDebug(("Sample %u, Value %u", m_calibration_samples, adc_value));
        if (m_calibration_samples >= SAMPLES_TO_COMPLETE_CALIBRATION) {
            m_calibration_complete = true;
            m_calibration_run = false;
            LogDebug(
                ("New max value %u, min value %u", m_max_value, m_min_value));
        }
    } else {
        return_value = static_cast<float>(adc_value - m_min_value) * 100 /
                       (m_max_value - m_min_value);
        if (m_inverse_measurement) {
            return_value = (return_value - 100.0f) * (-1.0f);
        }
    }
}

void Ads1115Adc::start_calibration() {
    LogDebug(("Run sensor calibration"));
    m_calibration_run = true;
    m_calibration_complete = false;
    m_calibration_samples = 0;
    m_max_value = std::numeric_limits<std::uint16_t>::min();
    m_min_value = std::numeric_limits<std::uint16_t>::max();
}
