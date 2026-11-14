// SPDX-License-Identifier: GPL-3.0-only
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ds3231.h"
#define PIN_SDA 18
#define PIN_SCL 8

static ds3231_t s_rtc;

/* 判断 RTC 里的时间像不像"真被设过"。
 * DS3231 掉电复位后寄存器默认是 00:00:00 01/01/00，也就是 2000-01-01，
 * 此时 tm_year 正好等于 100，能混过"年份 >= 2000"这种粗筛，
 * 所以默认值必须单独排掉。 */
static bool time_plausible(const struct tm *t)
{
    if (t->tm_year < 100) return false;            /* < 2000，肯定没设过 */
    if (t->tm_mon  < 0 || t->tm_mon > 11) return false;
    if (t->tm_mday < 1 || t->tm_mday > 31) return false;
    if (t->tm_hour > 23 || t->tm_min > 59 || t->tm_sec > 60) return false;
    if (t->tm_year == 100 && t->tm_mon == 0 && t->tm_mday == 1 &&
        t->tm_hour == 0 && t->tm_min == 0 && t->tm_sec == 0) {
        return false;                              /* 掉电默认值 */
    }
    return true;
}

/* 上电初始化：只在"时间不可信"时才重新对时 */
static void rtc_bringup(i2c_master_bus_handle_t bus)
{
    ESP_ERROR_CHECK(ds3231_init(&s_rtc, bus));

    bool lost = false;
    ds3231_check_osf(&s_rtc, &lost);            /* 芯片掉过电会把 OSF 置 1 */

    struct tm now = {0};
    bool readable = (ds3231_read_time(&s_rtc, &now) == ESP_OK);

    if (lost || !readable || !time_plausible(&now)) {
        printf("RTC 未设时/掉过电（OSF=%d），用编译时间对时\n", (int)lost);
        ESP_ERROR_CHECK(ds3231_set_from_build(&s_rtc));
        ds3231_clear_osf(&s_rtc);               /* 设完再清标志 */
    }
}

void app_main(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,  // 模块上有 4.7k，别开内部弱上拉
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    esp_err_t err = i2c_master_probe(bus, 0x68, 100);
    if (err != ESP_OK) {
        printf("DS3231 没应答（%s）：查接线/上拉/引脚\n", esp_err_to_name(err));
        return;
    }

    rtc_bringup(bus);

    /* 主循环：200ms 轮询，只在"秒"变化时刷新
     *   为什么不是 1000ms：固定 1s 间隔会和 RTC 的秒边界错开，
     *   显示永远慢 0~1 秒；200ms 轮询能在 200ms 内捕捉到秒跳变。*/
    int last_sec = -1;
    int last_temp_min = -1;
    TickType_t next = xTaskGetTickCount();

    for (;;) {
        struct tm now = {0};
        esp_err_t e = ds3231_read_time(&s_rtc, &now);

        if (e != ESP_OK) {
            printf("read error: %s\n", esp_err_to_name(e));
        } else if (now.tm_sec != last_sec) {
            last_sec = now.tm_sec;
            printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                   1900 + now.tm_year, now.tm_mon + 1, now.tm_mday,
                   now.tm_hour, now.tm_min, now.tm_sec);
        }

        /* 每分钟顺手读一次温度：既能看到芯片状态，也顺带验证 I2C 连读。
         * 不用 %f：精简版 printf 可能没开浮点格式化，整数拆开打印最稳。 */
        if (e == ESP_OK && now.tm_min != last_temp_min) {
            float c;
            if (ds3231_read_temp(&s_rtc, &c) == ESP_OK) {
                last_temp_min = now.tm_min;
                int milli = (int)(c * 100.0f + (c >= 0 ? 0.5f : -0.5f));
                int frac = milli % 100;
                if (frac < 0) frac = -frac;
                printf("温度: %d.%02d C\n", milli / 100, frac);
            }
        }

        /* vTaskDelayUntil 保证周期稳定：任务执行耗时会自动补偿，
         * 而 vTaskDelay 是"从现在起再等"，误差会累积 */
        vTaskDelayUntil(&next, pdMS_TO_TICKS(200));
    }
}
