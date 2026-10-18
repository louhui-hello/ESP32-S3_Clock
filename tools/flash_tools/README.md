# 烧录工具 · Flashing Tools

ESP32-S3 的烧录与调试辅助脚本。
Flashing and debugging helpers for the ESP32-S3.

---

## 常用命令 · Common Commands

```bash
# 查看芯片信息
esptool.py --port COM4 flash_id

# 擦除整片
esptool.py --port COM4 erase_flash

# 烧录固件
esptool.py --port COM4 --baud 460800 write_flash 0x0 bootloader.bin

# ESP-IDF 原生
idf.py -p COM4 flash monitor
```

---

## 注意

```
⚠️ ESP32-S3 进入下载模式：按住 BOOT，按一下 RST，松开 BOOT
⚠️ 烧录波特率从 115200 起试，稳了再加到 460800
⚠️ 模块 ESP-01 类没有自动下载电路，需要手动拉 GPIO0
```

---

## TODO

- [ ] 自动检测串口
- [ ] 一键烧录脚本
- [ ] 烧录后的自检
