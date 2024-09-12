# Install pre-commit
```
pip install pre-commit
pre-commit install
```

# Build
Following script will build the bootloader and rest sensor board application and combine these as a .bin file. It will
also prepend hash to application so it will be launched by the bootloader.
```
./build_and_combine.sh
```

# Flash
Connect Pico Probe and run ./flash_combined.sh

# Config
Enter wifi SSID and password over serial when promted. Once it have connected, the sensors can be configured with REST API.
```
POST /SENSORS HTTP/1.1
Host: 192.168.X.X
Content-Type: application/json
Content-Length: 147

{"config": [{"inversed":true,"max":18214,"min":8387,"pin":1,"type":"moisture"},{"inversed":true,"max":18204,"min":8180,"pin":2,"type":"moisture"}]}
```

# Rest get
Get sensor readings:
```
GET /SENSORS HTTP/1.1
Host: 192.168.50.205
```

Get current sensor config:
```
GET /CONFIG HTTP/1.1
Host: 192.168.50.205
```

# SW download
Run
```
./python/upload_new_binary.py --host 192.168.X.X --app-file build/PICO_REST_SENSOR.bin
```

# Raspberry Pi Pico W Analog sensor board
![picow_plant_moisture_sensor](https://github.com/NdreasNdersson/PicoW-Plant-Moisture-Sensor/blob/main/KiCad/picow_plant_moisture_sensor/picow_plant_moisture_sensor.png?raw=true)
