# Install pre-commit
```
pip install pre-commit
pre-commit install
```

# Build
```
./environment.sh
mkdir -p build/debug
cd build/debug
cmake ../.. -DCMAKE_BUILD_TYPE=DEBUG
ninja
```

# Flash
Connect Pico Probe and run ./flash
  or
press BOOTSEL button when starting and copy build/debug/src/PICO_REST_SENSOR.uf2 to mounted drive.

# Config
Set json data and uncomment write command:
```
    /*·Uncomment to write config to flash */
    std::string json_str{"{\"wifi\":{\"ssid\":\"xxxx\",\"password\":\"xxxx\"},\"sensors\":[{\"type\":\"moisture\",\"pin\":1,\"min\":7648,\"max\":17930,\"inversed\":true},{\"type\":\"moisture\",\"pin\":6,\"min\":7757,\"max\":17888,\"inversed\":true}]}"};
    config_handler.write_json_to_flash(json_str);
```
in main.cpp. Wifi ssid and password are mandatory. Each sensor requires type and pin to be set.

# Raspberry Pi Pico W Analog sensor board
![picow_plant_moisture_sensor](https://github.com/NdreasNdersson/PicoW-Plant-Moisture-Sensor/blob/main/KiCad/picow_plant_moisture_sensor/picow_plant_moisture_sensor.png?raw=true)
