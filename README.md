# ESP32-C3 SHTC3 Temperature & Humidity Sensor

Firmware for the ESP32-C3 that reads temperature and humidity from an
SHTC3 sensor over I2C and prints a reading every 2 seconds.

## What it does

- Talks to the SHTC3 at I2C address `0x70` using ESP-IDF's `i2c_master`
  driver (SCL on GPIO 8, SDA on GPIO 7, 100 kHz)
- Wakes the sensor, triggers a measurement, reads the raw values, and
  puts the sensor back to sleep between readings to save power
- Validates every reading with the SHTC3's CRC-8 checksum
  (polynomial 0x31, init 0xFF) before converting
- Converts raw values to relative humidity (%) and temperature (°C/°F)
  and prints them over serial

Example output:

```
Temperature is 24C (or 75F) with a 41% humidity
```

## Build and flash

Requires ESP-IDF 6.1.

```bash
idf.py set-target esp32c3
idf.py build flash monitor
```

## Files

All of the code lives in one source file:

- `main/shtc3_sensor.c` — the complete sensor driver and main loop
- `sdkconfig.defaults` — target selection (ESP32-C3)
- The two `CMakeLists.txt` files are the standard ESP-IDF build glue
