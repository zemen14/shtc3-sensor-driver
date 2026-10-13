#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO    8
#define I2C_MASTER_SDA_IO    7
#define SHTC3_ADDR           0x70

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

static uint8_t crc8(uint8_t *data, int len) {
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    }
    return crc;
}

static void shtc3_wakeup(void) {
    uint8_t cmd[] = {0x35, 0x17};
    i2c_master_transmit(dev_handle, cmd, 2, 100);
    vTaskDelay(pdMS_TO_TICKS(2));
}

static void shtc3_sleep(void) {
    uint8_t cmd[] = {0xB0, 0x98};
    i2c_master_transmit(dev_handle, cmd, 2, 100);
}

static int read_humidity(uint8_t *buf) {
    if (crc8(buf, 2) != buf[2]) {
        printf("Humidity CRC error!\n");
        return -1;
    }
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (int)(100.0f * raw / 65535.0f);
}

static int read_temperature(uint8_t *buf) {
    if (crc8(buf, 2) != buf[2]) {
        printf("Temperature CRC error!\n");
        return -999;
    }
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (int)(-45.0f + 175.0f * raw / 65535.0f);
}

void app_main(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_new_master_bus(&bus_config, &bus_handle);

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SHTC3_ADDR,
        .scl_speed_hz = 100000,
    };
    i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);

    while (1) {
        // Read humidity
        shtc3_wakeup();
        vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t cmd1[] = {0x5C, 0x24};
        i2c_master_transmit(dev_handle, cmd1, 2, 100);
        vTaskDelay(pdMS_TO_TICKS(20));
        uint8_t hbuf[3];
        i2c_master_receive(dev_handle, hbuf, 3, 100);
        shtc3_sleep();

        // Read temperature  
        shtc3_wakeup();
        vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t cmd2[] = {0x78, 0x66};
        i2c_master_transmit(dev_handle, cmd2, 2, 100);
        vTaskDelay(pdMS_TO_TICKS(50));
        uint8_t tbuf[3];
        i2c_master_receive(dev_handle, tbuf, 3, 100);
        shtc3_sleep();

        int humidity = read_humidity(hbuf);
        int temp_c = read_temperature(tbuf);

        int temp_f = (temp_c * 9 / 5) + 32;
        printf("Temperature is %dC (or %dF) with a %d%% humidity\n",
               temp_c, temp_f, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
