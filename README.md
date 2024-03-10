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
Set
```
#define·configNUMBER_OF_CORES···················1
```
in configs/FreeRTOS-Kernel/FreeRTOSConfig.h.

Set
```
····std::string·json_str{"{\"wifi\":{\"ssid\":\"xxxx\",\"password\":\"xxxx\"},\"sensors\":[{\"type\":\"moisture\",\"pin\":1,\"min\":7648,\"max\":17930,\"inversed\":true},{\"type\":\"moisture\",\"pin\":6,\"min\":7757,\"max\":17888,\"inversed\":true}"};
```
wifi ssid and password are mandatory. Each sensor requires type and pin to be set.

Set configNUMBER_OF_CORES when configuration is done.


# Raspberry Pi Pico W Analog sensor board
![picow_plant_moisture_sensor](https://github.com/NdreasNdersson/PicoW-Plant-Moisture-Sensor/blob/main/KiCad/picow_plant_moisture_sensor/picow_plant_moisture_sensor.png?raw=true)
