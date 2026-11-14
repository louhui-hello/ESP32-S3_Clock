# 固件

当前提供基于 ESP-IDF 6.1 的 DS3231 驱动与示例应用。完整时钟固件仍在开发，Arduino 版本尚未发布。

## 已有内容

- [DS3231 示例工程](ESP-IDF/esp32clock/README.md)：I²C 初始化、RTC 时间读写、温度读取、OSF 检查与清除、编译时间对时。
- [驱动源码](ESP-IDF/esp32clock/main/ds3231.c)与[头文件](ESP-IDF/esp32clock/main/ds3231.h)。
- 示例每 200 ms 轮询，在秒变化时打印时间，每分钟读取一次温度。

## 构建

先激活 ESP-IDF 6.1 环境，再从仓库根目录执行：

```sh
cd firmware/ESP-IDF/esp32clock
idf.py set-target esp32s3
idf.py build
```

接线与烧录方式见[工程说明](ESP-IDF/esp32clock/README.md)。示例默认 SDA=GPIO18、SCL=GPIO8，使用 Mini V4.0 时需要按硬件将 SDA 改为 GPIO9。

当前不包含屏幕显示、按键、闹钟等完整应用，也没有提供预编译固件包。

## 许可证

应用与示例代码：[GPL-3.0-only](LICENSE)。独立 DS3231 驱动：[MIT](ESP-IDF/esp32clock/main/LICENSE-MIT)。
