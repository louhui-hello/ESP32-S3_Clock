// ESP32-S3 Clock: u8g2 + 0.91" SSD1306(128x32) + 内部 RTC + 按键调闹钟 + AT24C02 保存
// 引脚见 docs/pinout.md
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>
#include "esp_sntp.h"
#include <WebServer.h>
#include <ESPmDNS.h>

#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
#else
#define WIFI_SSID ""
#define WIFI_PASS ""
#endif

#define PIN_SDA 9
#define PIN_SCL 8
#define BUZZER_PIN 13

#define KEY_LEFT 21
#define KEY_RIGHT 47
#define KEY_ENTER 48

#define EE_ADDR 0x50

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE);

// ---------- AT24C02 ----------
static void ee_write(uint8_t addr, uint8_t val) {
    Wire.beginTransmission(EE_ADDR);
    Wire.write(addr);
    Wire.write(val);
    Wire.endTransmission();
    delay(6);
}

static uint8_t ee_read(uint8_t addr) {
    Wire.beginTransmission(EE_ADDR);
    Wire.write(addr);
    if (Wire.endTransmission(false) != 0) return 0xFF;
    if (Wire.requestFrom(EE_ADDR, 1) != 1) return 0xFF;
    return Wire.available() ? Wire.read() : 0xFF;
}

// ---------- 时间 ----------
static void set_time_from_build(void) {
    static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char mon[4] = {0};
    int day = 0, year = 0, hh = 0, mm = 0, ss = 0;
    sscanf(__DATE__, "%3s %d %d", mon, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hh, &mm, &ss);
    const char *p = strstr(months, mon);
    int month = p ? (int)(p - months) / 3 + 1 : 1;

    struct tm t = {};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hh;
    t.tm_min = mm;
    t.tm_sec = ss;
    t.tm_isdst = 0;
    // 必须先设时区，mktime 才会把"本地时间"正确换算成 UTC 时间戳
    setenv("TZ", "CST-8", 1);
    tzset();
    time_t epoch = mktime(&t);
    struct timeval tv = {epoch, 0};
    settimeofday(&tv, nullptr);
}

// ---------- 按键 ----------
enum Key { K_NONE, K_LEFT, K_RIGHT, K_ENTER };

static Key read_key(void) {
    static uint32_t lastMs = 0;
    static Key lastKey = K_NONE;
    static int stable = -1;
    Key raw = K_NONE;
    if (digitalRead(KEY_LEFT) == LOW) raw = K_LEFT;
    else if (digitalRead(KEY_RIGHT) == LOW) raw = K_RIGHT;
    else if (digitalRead(KEY_ENTER) == LOW) raw = K_ENTER;

    if ((int)raw != stable) {
        stable = (int)raw;
        lastMs = millis();
        return K_NONE;
    }
    if (millis() - lastMs < 30) return K_NONE;     // 消抖
    if (raw != K_NONE && raw != lastKey) {
        lastKey = raw;
        return raw;
    }
    if (raw == K_NONE) lastKey = K_NONE;
    return K_NONE;
}

static void beep(int ms) {
    tone(BUZZER_PIN, 2730, ms);
}

// ---------- 闹钟配置 ----------
struct Alarm {
    uint8_t hour;
    uint8_t minute;
    uint8_t enabled;
};

static Alarm alarm_cfg = {7, 30, 0};

static void alarm_load(void) {
    if (ee_read(0xF0) != 0xA5) {          // 未初始化过
        alarm_cfg = {7, 30, 0};
        return;
    }
    alarm_cfg.hour = ee_read(0xF1) % 24;
    alarm_cfg.minute = ee_read(0xF2) % 60;
    alarm_cfg.enabled = ee_read(0xF3) ? 1 : 0;
}

static void alarm_save(void) {
    ee_write(0xF1, alarm_cfg.hour);
    ee_write(0xF2, alarm_cfg.minute);
    ee_write(0xF3, alarm_cfg.enabled);
    ee_write(0xF0, 0xA5);
}

// ---------- 界面状态 ----------
enum Mode { MODE_CLOCK, MODE_SET, MODE_RING };
static Mode mode = MODE_CLOCK;
static bool editing_hour = true;
static uint8_t ring_start_min = 255;

static void draw_clock(const struct tm &t) {
    char big[8], info[26];
    snprintf(big, sizeof(big), "%02d:%02d", t.tm_hour, t.tm_min);
    if (alarm_cfg.enabled)
        snprintf(info, sizeof(info), "%04d-%02d-%02d A%02d:%02d",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, alarm_cfg.hour, alarm_cfg.minute);
    else
        snprintf(info, sizeof(info), "%04d-%02d-%02d alarm off",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

    u8g2.clearBuffer();
    // 第一行：日期 + 闹钟（4x6，占 0~7 像素）
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 7, info);
    // 第二行：大号时间（20 号字，基线 30，占 10~30 像素，不重叠）
    u8g2.setFont(u8g2_font_logisoso20_tn);
    u8g2.drawStr(0, 30, big);
    // 秒：右下角小字
    char sec[4];
    snprintf(sec, sizeof(sec), "%02d", t.tm_sec);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(104, 30, sec);
    u8g2.sendBuffer();
}

static void draw_set(void) {
    char big[8];
    snprintf(big, sizeof(big), "%02d:%02d", alarm_cfg.hour, alarm_cfg.minute);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tf);
    if (WiFi.status() == WL_CONNECTED) {
        u8g2.drawStr(0, 8, WiFi.localIP().toString().c_str());
    } else {
        u8g2.drawStr(0, 8, "no wifi");
    }
    u8g2.setFont(u8g2_font_logisoso20_tn);
    u8g2.drawStr(0, 30, big);
    int x = editing_hour ? 0 : 50;
    u8g2.drawBox(x, 30, 44, 2);
    u8g2.sendBuffer();
}

static void draw_ring(void) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(4, 14, "ALARM!");
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(4, 28, "press any key");
    u8g2.sendBuffer();
}


// ---------- 旋律存储与播放 ----------
#include <Preferences.h>

#define TUNE_MAX 1024
static uint16_t tHz[TUNE_MAX];
static uint16_t tMs[TUNE_MAX];
static int  tLen = 0;
static int  tIdx = 0;
static uint32_t tStart = 0;
static bool tPlaying = false;

static uint16_t midi2hz(int m) { return (uint16_t)(440.0 * pow(2.0, (m - 69) / 12.0)); }

static void tune_stop() {
    tPlaying = false;
    noTone(BUZZER_PIN);
}

static void tune_start() {
    if (tLen <= 0) return;
    tIdx = 0;
    tStart = millis();
    tPlaying = true;
    tone(BUZZER_PIN, tHz[0]);
    Serial.printf("play tune: %d notes\n", tLen);
}

static void tune_tick() {
    if (!tPlaying) return;
    if (millis() - tStart < tMs[tIdx]) return;
    tIdx++;
    if (tIdx >= tLen) { tune_stop(); Serial.println("tune done"); return; }
    tStart = millis();
    tone(BUZZER_PIN, tHz[tIdx]);
}

// ---------- MIDI 解析 ----------
static uint32_t rd_varlen(const uint8_t *d, size_t n, size_t &p) {
    uint32_t v = 0;
    while (p < n) {
        uint8_t b = d[p++];
        v = (v << 7) | (b & 0x7F);
        if (!(b & 0x80)) break;
    }
    return v;
}

struct MidiEv { uint32_t tick; uint8_t note; bool on; };

static bool parse_midi(const uint8_t *d, size_t n) {
    if (n < 14 || memcmp(d, "MThd", 4) != 0) return false;
    uint16_t division = (d[12] << 8) | d[13];
    if (division == 0) return false;

    uint32_t tempo = 500000;          // 默认 120 BPM
    static MidiEv evs[4096];   // 必须在静态区，放栈上会撑爆 loopTask
    int nev = 0;

    size_t p = 8 + ((d[4] << 24) | (d[5] << 16) | (d[6] << 8) | d[7]);
    while (p + 8 <= n && nev < 4096) {
        if (memcmp(d + p, "MTrk", 4) != 0) break;
        size_t len = ((uint32_t)d[p+4] << 24) | ((uint32_t)d[p+5] << 16) | ((uint32_t)d[p+6] << 8) | d[p+7];
        size_t q = p + 8, end = p + 8 + len;
        if (end > n) end = n;
        uint32_t tick = 0;
        uint8_t status = 0;
        while (q < end) {
            tick += rd_varlen(d, end, q);
            if (q >= end) break;
            uint8_t b = d[q];
            if (b & 0x80) { status = b; q++; }
            if (status == 0xFF) {                      // meta
                if (q >= end) break;
                uint8_t type = d[q++];
                uint32_t l = rd_varlen(d, end, q);
                if (type == 0x51 && l == 3 && q + 3 <= end)
                    tempo = (d[q] << 16) | (d[q+1] << 8) | d[q+2];
                q += l;
                if (type == 0x2F) break;
                continue;
            }
            uint8_t hi = status & 0xF0;
            if (hi == 0x90 || hi == 0x80) {
                if (q + 1 >= end) break;
                uint8_t note = d[q], vel = d[q+1];
                q += 2;
                bool on = (hi == 0x90 && vel > 0);
                evs[nev++] = { tick, note, on };
            } else if (hi == 0xC0 || hi == 0xD0) {
                q += 1;
            } else {
                q += 2;
            }
        }
        p = end;
    }
    if (nev == 0) return false;

    // 按时间排序（qsort，避免 O(n^2) 卡死看门狗）
    qsort(evs, nev, sizeof(MidiEv), [](const void *a, const void *b) {
        const MidiEv *x = (const MidiEv *)a, *y = (const MidiEv *)b;
        return (x->tick > y->tick) - (x->tick < y->tick);
    });

    // tick → ms，单音化：遇到下一个 note-on 就结束当前音
    double msPerTick = (double)tempo / 1000.0 / (double)division;
    tLen = 0;
    int curNote = -1;
    uint32_t curStartTick = 0;
    for (int i = 0; i < nev && tLen < TUNE_MAX; i++) {
        if (evs[i].on) {
            if (curNote >= 0) {
                int ms = (int)((evs[i].tick - curStartTick) * msPerTick);
                if (ms >= 40) { tHz[tLen] = midi2hz(curNote); tMs[tLen] = (uint16_t)min(ms, 4000); tLen++; }
            }
            curNote = evs[i].note;
            curStartTick = evs[i].tick;
        } else if (evs[i].note == curNote) {
            int ms = (int)((evs[i].tick - curStartTick) * msPerTick);
            if (ms >= 40) { tHz[tLen] = midi2hz(curNote); tMs[tLen] = (uint16_t)min(ms, 4000); tLen++; }
            curNote = -1;
        }
    }
    return tLen > 0;
}

// 文本格式："hz ms" 每行一条
static bool parse_txt(const uint8_t *d, size_t n) {
    tLen = 0;
    char buf[64];
    size_t i = 0;
    while (i < n && tLen < TUNE_MAX) {
        size_t k = 0;
        while (i < n && d[i] != '\n' && k < sizeof(buf) - 1) buf[k++] = (char)d[i++];
        buf[k] = 0;
        if (i < n) i++;
        int hz = 0, ms = 0;
        if (sscanf(buf, "%d %d", &hz, &ms) == 2 && hz >= 50 && hz <= 5000 && ms >= 40) {
            tHz[tLen] = (uint16_t)hz;
            tMs[tLen] = (uint16_t)min(ms, 4000);
            tLen++;
        }
    }
    return tLen > 0;
}

static Preferences nvs;

static void tune_save() {
    static uint16_t buf[TUNE_MAX * 2];
    for (int i = 0; i < tLen; i++) { buf[i * 2] = tHz[i]; buf[i * 2 + 1] = tMs[i]; }
    size_t n = nvs.putBytes("notes", buf, tLen * 4);
    Serial.printf("tune_save: %d notes, %u bytes -> %s\n",
                  tLen, (unsigned)(tLen * 4), n ? "ok" : "FAIL");
}

static void tune_load() {
    size_t len = nvs.getBytesLength("notes");
    static uint16_t buf[TUNE_MAX * 2];
    if (len < 4 || len > TUNE_MAX * 4) { tLen = 0; Serial.println("tune_load: empty"); return; }
    if (nvs.getBytes("notes", buf, len) != len) { tLen = 0; Serial.println("tune_load: read fail"); return; }
    tLen = len / 4;
    if (tLen > TUNE_MAX) tLen = TUNE_MAX;
    for (int i = 0; i < tLen; i++) { tHz[i] = buf[i * 2]; tMs[i] = buf[i * 2 + 1]; }
    Serial.printf("tune_load: %d notes\n", tLen);
}
// ---------- 网页 ----------
static WebServer web(80);
static bool web_ready = false;
static String upBuf;

static void handle_upload() {
    HTTPUpload &u = web.upload();
    if (u.status == UPLOAD_FILE_START) {
        upBuf = "";
        upBuf.reserve(24576);
        Serial.printf("upload start: %s\n", u.filename.c_str());
    } else if (u.status == UPLOAD_FILE_WRITE) {
        if (upBuf.length() + u.currentSize < 24576) {
            for (size_t i = 0; i < u.currentSize; i++) upBuf += (char)u.buf[i];
        }
    } else if (u.status == UPLOAD_FILE_END) {
        bool ok = parse_midi((const uint8_t *)upBuf.c_str(), upBuf.length());
        if (!ok) ok = parse_txt((const uint8_t *)upBuf.c_str(), upBuf.length());
        if (ok) {
            tune_save();
            Serial.printf("upload ok: %d notes saved\n", tLen);
        } else {
            Serial.println("upload parse failed");
            tLen = 0;
        }
    }
}

static void web_root() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    char cur[40];
    strftime(cur, sizeof(cur), "%Y-%m-%d %H:%M:%S", &t);

    String html = "<!doctype html><meta charset=utf-8>";
    html += "<meta name=viewport content='width=device-width,initial-scale=1'>";
    html += "<style>body{background:#111;color:#0f8;font-family:sans-serif;text-align:center;padding:16px}";
    html += "input{font-size:18px;width:64px;text-align:center}button{font-size:16px;padding:8px 16px;margin:4px}";
    html += "a{color:#0f8}</style>";
    html += "<h2>ESP32-S3 Clock</h2><h1>" + String(cur) + "</h1>";
    html += "<p>IP " + WiFi.localIP().toString() + " · clock.local</p>";
    html += "<form action=/set method=get>闹钟 <input name=h value=" + String(alarm_cfg.hour);
    html += "> : <input name=m value=" + String(alarm_cfg.minute) + "> ";
    html += "<label><input type=checkbox name=on value=1";
    if (alarm_cfg.enabled) html += " checked";
    html += "> 启用</label> <button>保存</button></form>";
    html += "<hr><p>旋律：<b>" + String(tLen) + "</b> 个音符</p>";
    html += "<form action=/upload method=post enctype=multipart/form-data>";
    html += "<input type=file name=f accept='.mid,.txt' style='font-size:14px;width:auto'> <button>上传</button></form>";
    html += "<p><a href=/play>▶ 播放</a> | <a href=/stop>■ 停止</a> | <a href=/ring>🔔 响铃</a></p>";
    html += "<p style='font-size:13px;opacity:.7'>支持标准 MIDI (.mid, ≤24KB) 或 'hz ms' 文本</p>";
    web.send(200, "text/html; charset=utf-8", html);
}

static void web_set() {
    if (web.hasArg("h")) alarm_cfg.hour = web.arg("h").toInt() % 24;
    if (web.hasArg("m")) alarm_cfg.minute = web.arg("m").toInt() % 60;
    alarm_cfg.enabled = web.hasArg("on") ? 1 : 0;
    alarm_save();
    web.sendHeader("Location", "/");
    web.send(303, "text/plain", "");
}

static void web_ring()  { mode = MODE_RING; web.sendHeader("Location", "/"); web.send(303, "text/plain", ""); }
static void web_play()  { tune_start(); web.sendHeader("Location", "/"); web.send(303, "text/plain", ""); }
static void web_stop()  { tune_stop(); web.sendHeader("Location", "/"); web.send(303, "text/plain", ""); }

static void web_api() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    char cur[40];
    strftime(cur, sizeof(cur), "%Y-%m-%d %H:%M:%S", &t);
    String j = "{\"time\":\"" + String(cur) + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    j += ",\"notes\":" + String(tLen);
    j += ",\"alarm\":{\"h\":" + String(alarm_cfg.hour) + ",\"m\":" + String(alarm_cfg.minute);
    j += ",\"on\":" + String(alarm_cfg.enabled ? "true" : "false") + "}}";
    web.send(200, "application/json", j);
}
void setup() {
    Serial.begin(115200);
    delay(300);

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000);
    u8g2.begin();

    // 自检：总线上有没有 0x3C / 0x50，屏幕上画个测试图
    Wire.beginTransmission(0x3C);
    int oled_ok = (Wire.endTransmission() == 0);
    Wire.beginTransmission(0x50);
    int ee_ok = (Wire.endTransmission() == 0);
    Serial.printf("i2c selfcheck: oled=0x3C %s, eeprom=0x50 %s\n",
                  oled_ok ? "OK" : "MISSING", ee_ok ? "OK" : "MISSING");
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 32);
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(6, 20, oled_ok ? "OLED OK" : "OLED LOST");
    u8g2.sendBuffer();
    delay(3000);

    set_time_from_build();   // 先用编译时刻垫底，NTP 成功后会覆盖成正确时间

    // ===== WiFi + NTP 对时 =====
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(4, 12, "WiFi connecting");
    u8g2.sendBuffer();

    if (strlen(WIFI_SSID) > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        int tries = 0;
        while (WiFi.status() != WL_CONNECTED && tries++ < 30) {   // 最多 15 秒
            delay(500);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("wifi ok: %s  rssi=%d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());

            // 网页 + mDNS
            web.on("/", web_root);
            web.on("/set", web_set);
            web.on("/ring", web_ring);
            web.on("/play", web_play);
            web.on("/stop", web_stop);
            web.on("/upload", HTTP_POST, []() { web.sendHeader("Location", "/"); web.send(303, "text/plain", ""); }, handle_upload);
            web.on("/api", web_api);
            web.begin();
            MDNS.begin("clock");
            MDNS.addService("http", "tcp", 80);
            web_ready = true;
            Serial.println("web: http://" + WiFi.localIP().toString() + "  http://clock.local");            configTzTime("CST-8", "ntp.aliyun.com", "ntp1.aliyun.com", "cn.pool.ntp.org");

            u8g2.clearBuffer();
            u8g2.drawStr(4, 12, "NTP syncing");
            u8g2.sendBuffer();

            int n = 0;
            while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && n++ < 20) delay(500);   // 最多 10 秒
            time_t t2 = time(nullptr);
            if (t2 > 1600000000) {
                Serial.printf("ntp ok: %lld -> %s", (long long)t2, ctime(&t2));
            } else {
                Serial.println("ntp timeout, fallback to build time");
            }
        } else {
            Serial.println("wifi failed, fallback to build time");
        }
    } else {
        Serial.println("no wifi credentials (wifi_secrets.h missing)");
    }
    u8g2.setContrast(180);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(KEY_LEFT, INPUT_PULLUP);
    pinMode(KEY_RIGHT, INPUT_PULLUP);
    pinMode(KEY_ENTER, INPUT_PULLUP);

    if (!nvs.begin("clock", false)) Serial.println("NVS 初始化失败，旋律不可用");
    uint32_t t_tl = millis();
    tune_load();
    Serial.printf("tune_load: %u ms\n", millis() - t_tl);

    alarm_load();

    // 蜂鸣器：无源型，三极管驱动，2.7kHz（虚焊已修复，三种方式均可用）
    pinMode(BUZZER_PIN, OUTPUT);
    tone(BUZZER_PIN, 2730, 120);
    delay(200);
    noTone(BUZZER_PIN);

    Serial.printf("boot: alarm %02d:%02d %s, eeprom marker=%02X\n",
                  alarm_cfg.hour, alarm_cfg.minute,
                  alarm_cfg.enabled ? "on" : "off", ee_read(0xF0));
}

void loop() {
    tune_tick();
    static uint32_t lastTick = 0;
    if (millis() - lastTick > 10000) {
        lastTick = millis();
        time_t tt = time(nullptr);
        struct tm tz;
        localtime_r(&tt, &tz);
        Serial.printf("[tick] epoch=%lld local=%04d-%02d-%02d %02d:%02d:%02d sntp=%d\n",
                      (long long)tt, tz.tm_year + 1900, tz.tm_mon + 1, tz.tm_mday,
                      tz.tm_hour, tz.tm_min, tz.tm_sec, (int)sntp_get_sync_status());
    }
    if (web_ready) web.handleClient();
    static uint32_t lastDraw = 0;
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);

    Key k = read_key();

    switch (mode) {
    case MODE_CLOCK: {
        if (k == K_RIGHT) {                      // → 进入设置页
            mode = MODE_SET;
            editing_hour = true;
            beep(60);
            Serial.println("enter set mode (right key)");
            break;
        }
        if (k == K_LEFT) {                       // ← 立即响铃（测试用）
            mode = MODE_RING;
            Serial.println("TEST RING (left key)");
            break;
        }
        if (k == K_ENTER) {                      // ENTER 开关闹钟
            alarm_cfg.enabled = !alarm_cfg.enabled;
            alarm_save();
            beep(60);
            Serial.printf("alarm %s\n", alarm_cfg.enabled ? "on" : "off");
        }
        // 串口命令：t=立即响铃  s=把闹钟设成下一分钟
        while (Serial.available()) {
            int c = Serial.read();
            if (c == 't' || c == 'T') {
                mode = MODE_RING;
                Serial.println("TEST RING (serial)");
            } else if (c == 's' || c == 'S') {
                time_t n2 = time(nullptr);
                struct tm tt;
                localtime_r(&n2, &tt);
                tt.tm_min += 1;
                mktime(&tt);                     // 归一化
                alarm_cfg.hour = tt.tm_hour;
                alarm_cfg.minute = tt.tm_min;
                alarm_cfg.enabled = 1;
                alarm_save();
                Serial.printf("alarm set to %02d:%02d (+1min)\n", alarm_cfg.hour, alarm_cfg.minute);
            }
        }
        // 到点响铃：整分钟内只触发一次
        static int lastRingMinute = -1;
        int curMin = t.tm_hour * 60 + t.tm_min;
        if (alarm_cfg.enabled && curMin == alarm_cfg.hour * 60 + alarm_cfg.minute
            && lastRingMinute != curMin) {
            lastRingMinute = curMin;
            mode = MODE_RING;
            Serial.println("ALARM RING");
        }
        if (millis() - lastDraw > 200) {
            lastDraw = millis();
            draw_clock(t);
        }
        break;
    }

    case MODE_SET:
        if (k == K_LEFT) {
            if (editing_hour) alarm_cfg.hour = (alarm_cfg.hour + 23) % 24;
            else alarm_cfg.minute = (alarm_cfg.minute + 59) % 60;
            beep(40);
        } else if (k == K_RIGHT) {
            if (editing_hour) alarm_cfg.hour = (alarm_cfg.hour + 1) % 24;
            else alarm_cfg.minute = (alarm_cfg.minute + 1) % 60;
            beep(40);
        } else if (k == K_ENTER) {
            if (editing_hour) {
                editing_hour = false;
            } else {
                alarm_save();
                mode = MODE_CLOCK;
                Serial.printf("saved %02d:%02d\n", alarm_cfg.hour, alarm_cfg.minute);
                beep(120);
            }
        }
        draw_set();
        break;

    case MODE_RING:
        if (!tPlaying) tune_start();
        draw_ring();
        if (k != K_NONE) {
            tune_stop();
            mode = MODE_CLOCK;
            Serial.println("alarm stopped");
        }
        break;
    }

    delay(20);
}

























