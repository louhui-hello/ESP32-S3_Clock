# ESP-IDF 固件

[esp32clock/](esp32clock/README.md) 是当前的 DS3231 驱动验证工程，基于 ESP-IDF 6.1 与 ESP32-S3。

已提供时间读写、温度读取、振荡器停振标志处理及串口打印示例。屏幕、按键、闹钟等整机功能仍在开发。

示例默认 SDA=GPIO18、SCL=GPIO8；Mini V4.0 使用 SDA=GPIO9，请在接线与配置时核对。

应用代码采用 GPL-3.0-only，独立驱动采用 MIT，具体见工程内许可证。
