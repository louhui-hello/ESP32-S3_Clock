# ESP32-S3 Open-Source Battery Clock

An 81.9 × 72.3 mm four-layer desktop clock built around an **ESP32-S3-WROOM-1-N16R8**:
0.91" OLED, buzzer alarm, AT24C02 configuration storage, TP4056 charging and a 21700 cell.

> Open Source is Love.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware](#hardware)
- [Pinout](#pinout)
- [Firmware](#firmware)
- [Web Interface & HTTP API](#web-interface--http-api)
- [Alarm Melody Pipeline](#alarm-melody-pipeline)
- [Build & Flash](#build--flash)
- [Repository Layout](#repository-layout)
- [Current Status & Known Issues](#current-status--known-issues)
- [Roadmap](#roadmap)
- [License](#license)

---

## Overview

This is a self-contained desk clock project: the PCB is designed in EasyEDA Pro,
fabricated as a 4-layer board, and driven by an Arduino-framework firmware built
with PlatformIO. Time is obtained from the network (SNTP) and kept in the ESP32's
internal RTC; an alarm can be configured with three push buttons or from a web page,
and the alarm plays a user-uploaded melody on a buzzer.

The board was designed for a **DS3231MZ** high-accuracy RTC with CR1220 backup.
That chip on the current prototype turned out to be defective (it held the I²C bus
low) and was removed, so the current firmware runs on the internal RTC plus NTP —
see [Known Issues](#current-status--known-issues).

---

## Features

| Item | Details |
|---|---|
| MCU | ESP32-S3-WROOM-1-N16R8 — 16 MB flash, 8 MB PSRAM, Wi-Fi + BLE |
| Display | 0.91" OLED 128×32, SSD1306, I²C address `0x3C` |
| Time source | SNTP (Alibaba NTP pool) + ESP32 internal RTC, timezone `CST-8` |
| Alarm storage | AT24C02 EEPROM, I²C address `0x50` |
| Alarm melody | up to 1024 notes stored in NVS, uploaded over HTTP as MIDI or text |
| Input | 3 push buttons + RST + BOOT |
| Alert | passive buzzer (QMB-09B-03) driven through an NPN transistor at 2.7 kHz |
| Charging | TP4056 with thermal pad and 6 thermal vias |
| Buck converter | TLV62568 — ~90% efficiency, ~15 µA quiescent current |
| Battery | 21700 Li-ion cell with external protection board |
| Configuration | Wi-Fi credentials in a git-ignored header |

---

## Hardware

| Item | Value |
|---|---|
| Board size | 81.915 × 72.263 mm |
| Layers | 4 |
| Thickness | 1.6 mm |
| Surface finish | OSP (lead-free) |
| Solder mask | Blue |
| Copper | 1 oz outer / 0.5 oz inner |

### Stackup

| Layer | Function |
|---|---|
| L1 (Top) | Components, signals, local ground pours |
| L2 (Inner 1) | **Solid ground plane** |
| L3 (Inner 2) | **3.3 V power plane** |
| L4 (Bottom) | Signals |

### Power chain

```
USB-C 5V ──► TP4056 ──► 21700 cell ──► TLV62568 ──► 3V3 rail
                │                                        │
                └── CHRG / STDBY status LEDs             ├── ESP32-S3
                                                         ├── OLED
                                                         ├── AT24C02
                                                         └── buzzer driver
```

Fabrication files (Gerber, BOM, schematic PDF, EasyEDA project) are under
[`hardware/`](hardware/).

---

## Pinout

Full table with module pin numbers: [`docs/pinout.md`](docs/pinout.md).

### I²C bus

| Signal | GPIO | Module pin | Notes |
|---|---|---|---|
| SCL | GPIO8 | 12 | 400 kHz |
| SDA | GPIO9 | 17 | |

Devices currently present on the bus:

| Address | Device | Status |
|---|---|---|
| `0x3C` | SSD1306 OLED 128×32 | working |
| `0x50` | AT24C02 EEPROM | working |
| `0x68` | DS3231 RTC | **removed** on the current prototype |

### Buttons and buzzer

| Signal | GPIO | Module pin | Notes |
|---|---|---|---|
| KEY_LEFT | GPIO21 | 23 | pad ripped off on the prototype, repaired with a flywire |
| KEY_RIGHT | GPIO47 | 24 | |
| KEY_ENTER | GPIO48 | 25 | |
| DS_WAKE_UP | GPIO12 | 20 | RTC wake input, unused while DS3231 is absent |
| BUZZER | GPIO13 | 21 | passive buzzer, NPN driver, 2.7 kHz PWM |

> **Do not confuse module pin 13 with GPIO13.** Module pin 13 is GPIO19, which is
> USB D− on this board. The native USB-Serial/JTAG console uses GPIO19/GPIO20 and
> those two pins must not be repurposed.

Reserved pins: GPIO19/20 (USB), GPIO43/44 (UART0), GPIO26–37 (flash/PSRAM),
GPIO0/45/46 (strapping).

---

## Firmware

The production firmware lives in [`tools/oled_rtc/`](tools/oled_rtc/) and is built
with **PlatformIO + Arduino framework + U8g2**.

### Architecture

```
                    ┌─────────────────────────────┐
   Wi-Fi (IoT SSID) │           app_main          │
        ┌──────────►│  NTP sync → internal RTC    │
        │           │                             │
        │           │  loop():                    │
        │           │   ├─ tune_tick()  (non-blocking melody player)
        │           │   ├─ web.handleClient()      │
        │           │   ├─ read_key()              │
        │           │   └─ draw_clock() / draw_set() / draw_ring()
        │           └─────────────────────────────┘
        │                        │
        │              I²C ──────┼──► SSD1306 0x3C
        │                        └──► AT24C02 0x50
        │                        │
        └── HTTP :80 ────────────┤
                                 └──► GPIO13 buzzer (NPN driver)
```

### Behaviour

- **Boot sequence**: I²C self-check → OLED splash → Wi-Fi connect → SNTP sync →
  load alarm from EEPROM → load melody from NVS → short beep → normal display.
- **Clock screen**: date and alarm state on the top line (4×6 font), large time
  (`logisoso20`), seconds in the bottom-right corner.
- **Buttons**:
  - `RIGHT` — enter the alarm settings page
  - `LEFT` / `RIGHT` — decrement / increment the selected field
  - `ENTER` — switch hour → minute → save and exit
  - `ENTER` (clock screen) — toggle the alarm on/off
  - `LEFT` (clock screen) — test the alarm immediately
- **Alarm**: fires once when the wall-clock minute matches the configured time,
  loops the uploaded melody until any key is pressed.
- **Serial commands** (115200 baud, USB-Serial/JTAG): `t` = ring now,
  `s` = set the alarm to the next minute, useful for testing.

### Non-blocking melody player

`tune_tick()` is called from `loop()` and advances a note table using `millis()`,
so button handling, the web server and the display stay responsive while a melody
plays. Each note is `{frequency_hz, duration_ms}`.

### Storage layout

| Data | Location | Size |
|---|---|---|
| Alarm hour / minute / enable / magic | AT24C02 `0xF1` / `0xF2` / `0xF3` / `0xF0=0xA5` | 4 bytes |
| Melody note table | NVS namespace `clock`, key `notes` | up to 1024 × 4 bytes |
| Wi-Fi credentials | `src/wifi_secrets.h` (git-ignored) | — |

---

## Web Interface & HTTP API

Once connected, the board is reachable at `http://<ip>/` or `http://clock.local/`
(mDNS hostname `clock`).

| Method | Path | Description |
|---|---|---|
| GET | `/` | HTML control page: clock, alarm form, melody upload, play/stop/ring |
| GET | `/api` | JSON state: `{"time","ip","notes","alarm":{"h","m","on"}}` |
| GET | `/set?h=6&m=30&on=1` | Save alarm (writes EEPROM) |
| GET | `/play` | Play the stored melody |
| GET | `/stop` | Stop playback |
| GET | `/ring` | Enter the alarm-ringing state |
| POST | `/upload` | `multipart/form-data` upload of a `.mid` or a plain-text melody |

Upload body limits: **24 KB**, 4096 MIDI events, 1024 stored notes.

---

## Alarm Melody Pipeline

Two upload formats are accepted:

**1. Standard MIDI (`.mid`)**

The firmware parses `MThd`/`MTrk` chunks, reads tempo events, sorts note events
with `qsort`, converts ticks to milliseconds, and collapses polyphony into a single
voice (a new note-on ends the previous note).

**2. Plain text (`hz ms` per line)**

```
698 480
622 160
831 160
```

A companion Python pipeline was used during development to turn audio into this
format:

```
audio (flac/mp3) ──► Basic Pitch / pyin ──► note list ──► .esp text
XM module        ──► pattern parser     ──► note list ──► .esp text
```

The scripts are outside the repository, but the resulting melodies can be uploaded
to any board running this firmware.

---

## Build & Flash

### Firmware build (PlatformIO)

```bash
cd tools/oled_rtc
cp src/wifi_secrets.example.h src/wifi_secrets.h   # then edit SSID / password
pio run                 # build
pio run -t upload       # flash over the native USB port
pio device monitor -b 115200
```

### Manual flash (esptool)

The ESP32-S3 bootloader, partition table and application must be written at their
own offsets — writing a merged image at `0x0` will not boot after a full erase:

```bash
esptool.py --chip esp32s3 -p COM8 -b 460800 write_flash \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x0     .pio/build/esp32s3/bootloader.bin \
  0x8000  .pio/build/esp32s3/partitions.bin \
  0x10000 .pio/build/esp32s3/firmware.bin
```

If flashing fails with `Write timeout`, hold **BOOT**, tap **RST**, release **BOOT**
to enter download mode and retry.

### Auxiliary tools

| Tool | Purpose |
|---|---|
| [`tools/flash_tools/`](tools/flash_tools/) | standalone Qt6 flasher (`ESP32Flasher_OneFile.exe`) + merged image + merge script |
| [`tools/i2c_scan/`](tools/i2c_scan/) | ESP-IDF project that scans the I²C bus and reads DS3231 registers |
| [`tools/serial_watch.py`](tools/serial_watch.py) | minimal serial monitor helper |

---

## Repository Layout

```
.
├── README.md                 Chinese README
├── README.en.md              this file
├── LICENSE                   CERN-OHL-S-2.0
├── docs/
│   ├── pinout.md             pin map (board-specific)
│   ├── versions.md           board revision history
│   └── soldering.md          soldering order and hot-plate profile
├── hardware/
│   ├── PCB/                  EasyEDA Pro project, Gerber, BOM, schematic PDF
│   ├── Gerber/               fabrication output
│   ├── BOM/                  bill of materials
│   ├── Schematic/            schematic exports
│   └── STEP/                 3D model
├── firmware/                 ESP-IDF experiments, binaries
├── legacy/                   archived V1 / V1.5 revisions
└── tools/
    ├── oled_rtc/             main firmware (PlatformIO)
    ├── i2c_scan/             bus scanner (ESP-IDF)
    └── flash_tools/          flasher utility
```

---

## Current Status & Known Issues

The board works as a Wi-Fi clock with buzzer alarm. The following issues are known
and either worked around in firmware or scheduled for the next revision.

| Issue | Impact | Status |
|---|---|---|
| DS3231 defective (bus held low) | no hardware RTC; time lost on power loss | chip removed, firmware uses internal RTC + NTP |
| Passive buzzer is quiet and harsh | melody sounds thin | driver redesigned for v4.0alpha (MOSFET, flyback diode, 12 mm buzzer) |
| `KEY_LEFT` pad torn off during rework | left button dead | repaired with a flywire to module pin 23 (GPIO21) |
| Charging status LED damaged by hot plate | no visual charging indication | to be replaced; firmware can read `CHRG`/`STDBY` instead |
| 21700 cell not installed yet | runs from USB only | pending |
| `LittleFS` unusable after a full chip erase | earlier firmware hung at boot | replaced with NVS note storage |
| MIDI sort was O(n²) | watchdog reset on large uploads | replaced with `qsort` |

---

## Roadmap

### v4.0alpha — ESP32-S3 validation board

- Buzzer driver: `2N7002` MOSFET, 100 Ω gate resistor, `1N4148` flyback diode
- Buzzer footprint: 12 mm passive type, `100 µF` + `0.1 µF` decoupling
- Optional audio path: `MAX98357A` I²S amplifier + 8 Ω speaker pads (not populated by default)
- Larger button pads (2 × 3 mm) or side-actuated switches
- Exposed test points: 3V3 / GND / SDA / SCL
- Target 2 layers to qualify for free prototyping

### STC8 — low-cost edition

| Item | Choice |
|---|---|
| MCU | STC8H1K08 (SOP16) or STC8H8K64U |
| RTC | **DS1307** + 32.768 kHz crystal + CR2032 backup |
| Display | SSD1306 0.91" 128×32 over I²C |
| Buzzer | **active** buzzer (GPIO on/off, no PWM) |
| Supply | 5 V board — DS1307 requires 4.5–5.5 V |
| Programming | UART header, STC-ISP |

Target cost: ~¥15 in parts.

---

## License

Licensed under **CERN-OHL-S-2.0** — a strongly reciprocal open hardware licence.
See [LICENSE](LICENSE).

---

## Author

**@学 ARM Keil 的初中生** (louhui-hello)

> 开源是种浪漫。
> Open Source is Love.
