#include <INA219_WE.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// INA219芯片地址
#define INA219_ADDRESS 0x42
// I2C时钟线引脚
#define S_SCL   33
// I2C数据线引脚
#define S_SDA   32
// OLED屏幕宽度
#define SCREEN_WIDTH 128    
// OLED屏幕高度
#define SCREEN_HEIGHT 32    
// OLED复位引脚（无复位引脚设为 -1 ）
#define OLED_RESET     -1   
// OLED I2C地址
#define SCREEN_ADDRESS 0x3C 
// 蜂鸣器连接引脚
#define BUZZER_PIN 21
// RGB LED控制引脚
#define RGB_LED 26
// 定义最大亮度，取值范围 0 - 255
#define BRIGHTNESS 255
// 控制灯珠的数量
#define NUMPIXELS 2

INA219_WE ina219 = INA219_WE(INA219_ADDRESS);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUMPIXELS, RGB_LED, NEO_GRB + NEO_KHZ800);

float shuntVoltage_mV = 0.0;
float loadVoltage_V = 0.0;
float busVoltage_V = 0.0;
float current_mA = 0.0;
float power_mW = 0.0; 
bool ina219_overflow = false;

// 初始化INA219芯片的IIC接口并设置检测参数
void InitINA219()
{
    ina219.init();
    ina219.setADCMode(BIT_MODE_9);
    ina219.setPGain(PG_320);
    ina219.setBusRange(BRNG_16);
    ina219.setShuntSizeInOhms(0.01); 
}

// 根据检测电压计算电池电量百分比
float calculateBatteryPercentage(float voltage) {
    // 可根据实际电池特性校准这些值
    const float fullVoltageMin = 7.2; 
    const float fullVoltageMax = 7.4; 
    const float emptyVoltage = 6.0; 

    // 限制百分比范围在0-100%
    if (voltage >= fullVoltageMax) return 100.0;
    if (voltage <= emptyVoltage) return 0.0;
    
    return (voltage - emptyVoltage) / (fullVoltageMax - emptyVoltage) * 100.0;
}

// 根据电量百分比改变图标样式
void drawBatteryIcon(int x, int y, float percentage) {
    const int iconWidth = 40;
    const int iconHeight = 12;
    int filledWidth = (int)(iconWidth * percentage / 100.0);
    
    // 绘制电池边框
    display.drawRect(x, y, iconWidth, iconHeight, SSD1306_WHITE);
    // 绘制电池正极
    display.fillRect(x + iconWidth, y + 3, 3, 6, SSD1306_WHITE);
    
    // 根据电量百分比改变填充颜色
    if (percentage > 75) {
        // 绿色 (高电量)
        display.fillRect(x + 1, y + 1, filledWidth - 1, iconHeight - 2, SSD1306_WHITE);
    } else if (percentage > 30) {
        // 黄色 (中等电量)
        // SSD1306只有黑白，这里用点状填充表示
        for (int i = x + 1; i < x + filledWidth; i++) {
            for (int j = y + 1; j < y + iconHeight - 1; j += 2) {
                display.drawPixel(i, j, SSD1306_WHITE);
            }
        }
    } else {
        // 红色 (低电量)
        // 闪烁效果（通过绘制/擦除实现）
        static bool flashState = true;
        if (flashState) {
            display.fillRect(x + 1, y + 1, filledWidth - 1, iconHeight - 2, SSD1306_WHITE);
        }
        flashState = !flashState;
    }
}

// 初始化蜂鸣器引脚
void initBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH); // 初始化为高电平，蜂鸣器不发声
}

// 初始化RGB LED
void initRGBLED() {
    matrix.begin();
    matrix.show(); // 初始化灯珠状态
}

// 控制蜂鸣器发声
void buzzerBeep(int duration) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
    digitalWrite(BUZZER_PIN, HIGH);
}

// 控制RGB LED闪烁红色
void rgbLEDFlashRed(int times, int interval) {
    for (int i = 0; i < times; i++) {
        for (int j = 0; j < NUMPIXELS; j++) {
            matrix.setPixelColor(j, matrix.Color(255, 0, 0));
        }
        matrix.show();
        delay(interval);
        for (int j = 0; j < NUMPIXELS; j++) {
            matrix.setPixelColor(j, matrix.Color(0, 0, 0));
        }
        matrix.show();
        delay(interval);
    }
}

// 使OLED屏幕文字水平滚动
void scrollTextOnOLED(const char* text, int speed) {
    int width = display.width();
    // 手动计算文本宽度，假设单个英文字符宽度为6像素
    int textWidth = 0;
    while (*text != '\0') {
        textWidth += 6;
        text++;
    }
    for (int x = 0; x > -textWidth; x -= speed) {
        display.clearDisplay();
        display.setCursor(x, 10);
        display.print(text);
        display.display();
        delay(50);
    }
}

void InaDataUpdate()
{
    shuntVoltage_mV = ina219.getShuntVoltage_mV();
    busVoltage_V = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getBusPower();
    loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
    ina219_overflow = ina219.getOverflow();

    float batteryPercentage = calculateBatteryPercentage(busVoltage_V);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Battery: ");
    drawBatteryIcon(60, 0, batteryPercentage);
    display.setCursor(105, 0);
    display.print((int)batteryPercentage);
    display.println("%");

    display.setCursor(0, 10);
    display.print("Voltage: ");
    display.print(busVoltage_V, 2);
    display.print("V  Current: ");
    display.print(current_mA, 2);
    display.println("mA");

    display.setCursor(0, 20);
    display.print("Power: ");
    display.print(power_mW, 2);
    display.println("mW");

    if (busVoltage_V < 6.0) {
        // 优化蜂鸣器鸣叫频率
        for (int i = 0; i < 5; i++) {
            buzzerBeep(80);
            delay(80);
        }
        // 优化RGB LED闪烁方式
        rgbLEDFlashRed(5, 150);
        // OLED显示并滚动提示文字
        scrollTextOnOLED("Low voltage", 2);
    }

    display.display();
}

void setup() 
{
    Wire.begin(S_SDA, S_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    InitINA219();
    initBuzzer();
    initRGBLED();
}

void loop() 
{
    InaDataUpdate();
    delay(1000);
}