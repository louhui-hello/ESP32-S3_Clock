# 固件 · Firmware

ESP32-S3 时钟的固件（ESP-IDF）。
Firmware for the ESP32-S3 clock (ESP-IDF).

---

## 目录 · Contents

```
firmware/
├── ESP-IDF/      主固件（ESP-IDF 工程）
├── Arduino/      Arduino 版本的试验代码
└── Binary/       编译好的固件（用于直接烧录）
```

---

## 状态 · Status

> 🚧 **开发中** — 本目录尚未完成。
> 🚧 **Work in progress** — firmware is not complete yet.

---

## 待实现 · TODO

- [ ] I2C 总线层
- [ ] DS3231 驱动
- [ ] SSD1306 驱动
- [ ] 按键消抖
- [ ] 蜂鸣器 PWM
- [ ] 电池电压检测
- [ ] 闹钟逻辑
- [ ] 深度睡眠 + 定时唤醒
- [ ] WiFi 配网 + NTP 校时

---

## 构建 · Build

```bash
cd ESP-IDF
idf.py set-target esp32s3
idf.py build
idf.py -p COM4 flash monitor
```
