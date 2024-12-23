cc_library(
    name = "sensors",
    srcs = [
        "ads1115_adc.cpp",
        "sensor_factory.cpp",
        "temp_adc.cpp",
    ],
    hdrs = [
        "ads1115_adc.h",
        "sensor.h",
        "sensor_config.h",
        "sensor_factory.h",
        "temp_adc.h",
    ],
    deps = [
        "@pico-sdk//src/rp2_common/hardware_adc:hardware_adc",
        "@pico-sdk//src/rp2_common/hardware_gpio:hardware_gpio",
        "@pico-sdk//src/rp2_common/hardware_i2c:hardware_i2c",
        "//libs/pico-ads1115/lib:pico-ads1115",
        "//src/patterns:patterns",
        "//src/utils:utils",
        "//src/utils/button:button",
    ],
)
