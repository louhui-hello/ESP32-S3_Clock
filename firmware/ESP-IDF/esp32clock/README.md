# esp32clock — DS3231 驱动示例

基于 ESP-IDF 6.1 的 ESP32-S3 工程。直接使用 I²C 主机 API 访问 DS3231 寄存器，不依赖第三方 RTC 驱动库。

这是驱动验证示例，完整时钟的屏幕显示、按键和闹钟应用仍在开发。

## 接线

| DS3231 | ESP32-S3 示例默认连接 |
| --- | --- |
| VCC | 3.3 V |
| GND | GND |
| SDA | GPIO18 |
| SCL | GPIO8 |

总线频率为 400 kHz，示例关闭内部上拉，需要外部上拉到 3.3 V。

**Mini V4.0 的 SDA 为 GPIO9。** 在该板上使用时，将 `main/main.c` 的 `PIN_SDA` 从 18 改为 9，并核对原理图；当前示例保持原有测试引脚。

## 目录

```text
esp32clock/
├── CMakeLists.txt
├── LICENSE                  # 固件应用 GPL-3.0，独立驱动另按 MIT
└── main/
    ├── CMakeLists.txt
    ├── main.c               # 总线初始化、对时和串口打印示例
    ├── ds3231.c             # RTC 寄存器访问与数据转换
    ├── ds3231.h             # 驱动接口
    └── LICENSE-MIT          # 独立驱动许可证
```

## 构建与烧录

激活 ESP-IDF 6.1 环境后，在本工程目录运行：

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p COM7 flash monitor
```

`COM7` 是示例端口，请替换为实际串口。构建不需要连接开发板；烧录和运行需要正确接线的设备。

本仓库不分发本机生成的 `sdkconfig`、编译目录、IDE 配置或预编译固件。

## 示例行为

1. 创建 I²C 主机总线，探测地址 `0x68`。
2. 初始化驱动，尝试清除控制寄存器的 EOSC 位；初始化不清除 OSF。
3. 示例应用读取 OSF 和时间，判断需要对时时写入编译时间，再清除 OSF。
4. 每 200 ms 轮询，在秒变化时打印时间，每分钟读取温度。

编译时间对时用于调试，不等同于联网授时；烧录耗时也会使实际时间产生偏差。

## 驱动 API

```c
esp_err_t ds3231_init(ds3231_t *rtc, i2c_master_bus_handle_t bus);
esp_err_t ds3231_deinit(ds3231_t *rtc);
esp_err_t ds3231_read_time(ds3231_t *rtc, struct tm *t);
esp_err_t ds3231_write_time(ds3231_t *rtc, const struct tm *t);
esp_err_t ds3231_read_temp(ds3231_t *rtc, float *celsius);
esp_err_t ds3231_check_osf(ds3231_t *rtc, bool *lost);
esp_err_t ds3231_clear_osf(ds3231_t *rtc);
esp_err_t ds3231_set_string(ds3231_t *rtc, const char *str, const char *format);
esp_err_t ds3231_set_from_build(ds3231_t *rtc);
```

- `ds3231_t` 必须先零初始化，例如 `static ds3231_t rtc;` 或 `ds3231_t rtc = {0};`。
- 时间使用 `struct tm`：年相对 1900，月份为 0–11。
- 当前年份转换以 2000–2099 为范围，未实现世纪位处理。
- 时间写入采用 24 小时格式；读取支持 12 / 24 小时格式。
- OSF 的判断、对时策略和清除由应用层负责。

## 验证

已用 ESP-IDF 6.1、目标 ESP32-S3 从干净配置完成编译检查。本次没有进行真机烧录，完整板级功能仍需实物验证。

## 许可证

`main/main.c` 与工程构建文件采用 **GPL-3.0-only**，见 [LICENSE](LICENSE)。

`main/ds3231.c` 和 `main/ds3231.h` 作为独立驱动采用 **MIT**，见 [main/LICENSE-MIT](main/LICENSE-MIT)。
