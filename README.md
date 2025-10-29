# IR Environment
A simple temperature/humidity/pressure sensor application which uploads readings to [emonpi](https://openenergymonitor.org/), designed for the M5 Stack Atom S3.

# Prerequisites
- [M5 Stack Atom S3](https://docs.m5stack.com/en/core/AtomS3)
- [ENV III Unit with Temperature Humidity Air Pressure Sensor (SHT30+QMP6988)](https://shop.m5stack.com/products/env-iii-unit-with-temperature-humidity-air-pressure-sensor-sht30-qmp6988)
- [PlatformIO](https://platformio.org/)

# Build
In `local.h` set up your node and access information for WiFi and emon, build, and deploy.

# Use
When the device boots it will display the node name, WiFi access point, and if connection is successful. If the devices cannot connect to WiFi it will reboot until it does.

Once the device is connected the display will show three colour bars for the different values, and then turn off the screen. Clicking the screen will turn it on for a short duration.

Press and hold the side button to restart the device.