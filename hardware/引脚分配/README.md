# esp32s3clock_Mini_v4.0 引脚分配
## TFT部份通过esp32s3的IO映射进行配置
* GPIO4 > TFT_BL

* GPIO5 > TFT_CS

* GPIO6 > TFT_DC

* GPIO7 > TFT_RES

* GPIO15 > TFT_MOSI

* GPIO16 > TFT_CLK

## I2C部份 已做4.7k上拉
* GPIO8 > SCL

* GPIO9 > SDA

## USB2.0 Full Speed 部份
* GPIO19 >  D-
* GPIO20 >  D+

## DS3231 WAKE_UP > GPIO12

## 交互部份
* GPIO13 > BUZZER_PWM
* GPIO21 > KEY_LEFT
* GPIO47 > KEY_RIGHT
* GPIO48 > KEY_ENTER

## ENABLE With BOOT(GPIO 0)
* BOOT > 10k 上拉至3.3v
* EN > 10k 上拉至3.3v



# English Powered by GPT6.1 Sol：

# esp32s3clock_Mini_v4.0 Pin Assignment

## TFT Section
The TFT section is configured via ESP32-S3 IO mapping.

* GPIO4 > TFT_BL
* GPIO5 > TFT_CS
* GPIO6 > TFT_DC
* GPIO7 > TFT_RES
* GPIO15 > TFT_MOSI
* GPIO16 > TFT_CLK

## I2C Section
4.7k pull-ups are already fitted.

* GPIO8 > SCL
* GPIO9 > SDA

## USB 2.0 Full Speed Section
* GPIO19 > D-
* GPIO20 > D+

## DS3231 WAKE_UP > GPIO12

## User Interaction Section
* GPIO13 > BUZZER_PWM
* GPIO21 > KEY_LEFT
* GPIO47 > KEY_RIGHT
* GPIO48 > KEY_ENTER

## ENABLE With BOOT (GPIO 0)
* BOOT > 10k pull-up to 3.3V
* EN > 10k pull-up to 3.3V