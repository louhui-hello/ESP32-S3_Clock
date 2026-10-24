#include <stdio.h>
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "driver/gpio.h"
#include "hal/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SCL_PIN GPIO_NUM_8
#define SDA_PIN GPIO_NUM_9
#define RTC_ADDR 0x68

static int bcd2dec(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }

/* 先把两根脚当输入读电平：都能到 3.3V 才算总线空闲正常 */
static void print_bus_levels(void)
{
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << SCL_PIN) | (1ULL << SDA_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    vTaskDelay(pdMS_TO_TICKS(10));
    printf("总线电平: SCL=%d  SDA=%d  (1=高, 0=低；都是1才正常)\n",
           gpio_get_level(SCL_PIN), gpio_get_level(SDA_PIN));
}

/* 不重配引脚，直接读输入寄存器（避免把脚从 I2C 外设手里抢走） */
static void print_bus_levels_passive(void)
{
    printf("  电平(被动读): SCL=%d  SDA=%d\n",
           gpio_get_level(SCL_PIN), gpio_get_level(SDA_PIN));
}

static void read_rtc(i2c_master_bus_handle_t bus)
{
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RTC_ADDR,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev = NULL;
    if (i2c_master_bus_add_device(bus, &cfg, &dev) != ESP_OK) {
        printf("  RTC: 无法挂载设备\n");
        return;
    }

    uint8_t reg = 0x00, t[7] = {0};
    if (i2c_master_transmit_receive(dev, &reg, 1, t, 7, 100) == ESP_OK) {
        int sec = bcd2dec(t[0] & 0x7F);
        int min = bcd2dec(t[1] & 0x7F);
        int hour = bcd2dec(t[2] & 0x3F);   /* 24h 模式 */
        int date = bcd2dec(t[4] & 0x3F);
        int mon = bcd2dec(t[5] & 0x1F);
        int year = 2000 + bcd2dec(t[6]);
        printf("  RTC 时间: %04d-%02d-%02d %02d:%02d:%02d\n", year, mon, date, hour, min, sec);
        printf("  RTC 状态: OSF=%d (0=时间有效, 1=掉过电需重设)  12/24h=%s\n",
               (t[5] & 0x80) ? 1 : 0, (t[2] & 0x40) ? "12h" : "24h");
    } else {
        printf("  RTC: 读时间失败\n");
    }

    uint8_t tr = 0x11, tb[2] = {0};
    if (i2c_master_transmit_receive(dev, &tr, 1, tb, 2, 100) == ESP_OK) {
        int msb = (int)(int8_t)tb[0];
        int frac = (tb[1] >> 6) * 25;      /* 0.25 °C 步进 */
        printf("  RTC 温度: %d.%02d °C\n", msb, frac);
    }
    i2c_master_bus_rm_device(dev);
}

void app_main(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .scl_io_num = SCL_PIN,
        .sda_io_num = SDA_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .intr_priority = 0,
        .flags.enable_internal_pullup = 0,
    };

    printf("\n项目地址:https://github.com/louhui-hello/ESP32-S3_Clock\n");
    printf("I2C 诊断 (sda=GPIO9, scl=GPIO8)\n");
    print_bus_levels();

    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    int round = 0;
    while (1) {
        printf("---- 第 %d 轮扫描 ----\n", ++round);
        print_bus_levels_passive();
        int found = 0, has_rtc = 0;
        for (uint8_t addr = 1; addr < 127; addr++) {
            if (i2c_master_probe(bus, addr, 10) == ESP_OK) {
                printf("  found 0x%02X%s\n", addr, addr == RTC_ADDR ? "  <- DS3231" : "");
                found++;
                if (addr == RTC_ADDR) has_rtc = 1;
            }
        }
        printf("done, %d device(s)\n", found);

        if (has_rtc) {
            read_rtc(bus);
        }
        if (found == 0) {
            /* 总线被拉死时复位一次，下一轮重新开始 */
            esp_err_t r = i2c_master_bus_reset(bus);
            printf("  总线复位: %s\n", r == ESP_OK ? "OK" : "失败");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
