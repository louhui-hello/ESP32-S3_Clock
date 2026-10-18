#include <stdio.h>
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "hal/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include "freertos/FreeRTOS.h"
void app_main(void)
{
    i2c_master_bus_config_t  bus_cfg = 
    {
        .i2c_port = I2C_NUM_0,
        .scl_io_num = GPIO_NUM_8,
        .sda_io_num = GPIO_NUM_9,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .intr_priority = 0
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));
    printf("项目地址:https://github.com/louhui-hello/ESP32-S3_Clock\nI2C设备扫描\nsda=GPIO_NUM_8,scl=GPIO_NUM_9\n");
    printf("扫描中\n");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (i2c_master_probe(bus, addr, 50) == ESP_OK) {
            printf( "  found 0x%02X\n", addr);
            found++;
        }
    }
    printf( "done, %d device(s)\n", found);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
