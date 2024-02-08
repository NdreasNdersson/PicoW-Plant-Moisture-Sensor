# This file contains all the device credentials.

# To use this file, set the appropriate credentials then rename it to "device.cmake"
# The program will not compile without correctly naming this file

# Configure WiFi credentials
message("Configuring WiFi Credentials.....")
if(NOT WIFI_SSID_SET)
    set(WIFI_SSID_SET "WiFiName") # Put your own WiFi SSID
    message("WiFi SSID set: ${WIFI_SSID_SET}")
endif()

if(NOT WIFI_PASSWORD_SET)
    set(WIFI_PASSWORD_SET "MyPassword") # Put your own WiFi Password
    message("WiFi PASSWORD set: *******")
endif()
