#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include "Config.h"
#include "Style.h"
#include "ColdDripEngine.h"
#include "UIComponents.h"

// ============================================================
// 冰滴咖啡機控制器 v2.0
// 平台: ESP32-S3 N16R8
// 顯示: 1.28" GC9A01 圓形 LCD
// 操控: EC11 旋轉編碼器
// ============================================================

TFT_eSPI tft = TFT_eSPI();
ESP32Encoder encoder;
ColdDripEngine engine;

// --- UI 狀態 ---
enum PageIndex { PAGE_MAIN = 0, PAGE_SETTINGS = 1 };
PageIndex currentPage = PAGE_MAIN;
unsigned long lastDisplayUpdate = 0;
int animFrame = 0;
bool needFullRedraw = true;

// --- 設定參數 (暫存) ---
int settingDripInterval;
int settingPumpDuration;
int settingsFocus = 0; // 0: 間隔, 1: 持續時間, 2: 返回

// --- 編碼器狀態 ---
long lastEncoderPos = 0;
unsigned long lastButtonPressTime = 0;

// --- 前向宣告 ---
void drawMainPage();
void updateMainPage();
void drawSettingsPage();
void updateSettingsPage();
void handleControls();

void setup() {
    Serial.begin(115200);
    
    // 初始化螢幕
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(CD_BG);
    
    // 初始化編碼器
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder.attachFullQuad(ENCODER_CLK_PIN, ENCODER_DT_PIN);
    pinMode(ENCODER_SW_PIN, INPUT); // 外部有分壓電阻，S3 GPIO35 在某些開發板是 ADC
    
    // 初始化引擎
    engine.begin();
    
    // 預設讀取參數
    BrewStatus st = engine.getStatus();
    settingDripInterval = st.dripIntervalSec;
    settingPumpDuration = st.pumpDurationMs;

    Serial.println("System v2.0 Initialized (S3 + GC9A01)");
}

void loop() {
    unsigned long now = millis();
    engine.update();
    
    // 處理編碼器與按鈕
    handleControls();

    // 更新顯示
    if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
        lastDisplayUpdate = now;
        animFrame++;
        
        if (currentPage == PAGE_MAIN) {
            if (needFullRedraw) {
                drawMainPage();
                needFullRedraw = false;
            }
            updateMainPage();
        } else if (currentPage == PAGE_SETTINGS) {
            if (needFullRedraw) {
                drawSettingsPage();
                needFullRedraw = false;
            }
            updateSettingsPage();
        }
    }
    
    delay(5);
}

void handleControls() {
    unsigned long now = millis();
    
    // 1. 旋轉處理
    long currentPos = encoder.getCount() / 2; // EC11 通常是 2 或 4 pulse per detent
    if (currentPos != lastEncoderPos) {
        int delta = currentPos - lastEncoderPos;
        lastEncoderPos = currentPos;
        
        if (currentPage == PAGE_MAIN) {
            // 主頁面：微調滴灌間隔
            int interval = engine.getStatus().dripIntervalSec + delta;
            engine.setDripInterval(constrain(interval, 10, 300));
            needFullRedraw = true; // 更新刻度
        } else if (currentPage == PAGE_SETTINGS) {
            // 設定頁面：調整數值
            if (settingsFocus == 0) {
                settingDripInterval = constrain(settingDripInterval + delta, 10, 300);
            } else if (settingsFocus == 1) {
                settingPumpDuration = constrain(settingPumpDuration + (delta * 50), 500, 5000);
            }
            updateSettingsPage();
        }
    }
    
    // 2. 按鈕處理 (Analog 讀取判斷 GPIO35)
    bool isPressed = analogRead(ENCODER_SW_PIN) < 500; // 按下時接近 0V
    static bool lastPressed = false;
    
    if (isPressed && !lastPressed && (now - lastButtonPressTime > 300)) {
        lastButtonPressTime = now;
        
        if (currentPage == PAGE_MAIN) {
            // 長按進入設定，短按切換開始/暫停
            // 簡化版：短按切換狀態，雙擊或未來邏輯可進設定
            // 目前先用短按切換狀態
            BrewState state = engine.getState();
            if (state == BREW_IDLE || state == BREW_COMPLETE) engine.startBrew();
            else if (state == BREW_RUNNING) engine.pauseBrew();
            else if (state == BREW_PAUSED) engine.resumeBrew();
            needFullRedraw = true;
        } else if (currentPage == PAGE_SETTINGS) {
            settingsFocus++;
            if (settingsFocus > 2) {
                // 保存並返回
                engine.setDripInterval(settingDripInterval);
                engine.setPumpDuration(settingPumpDuration);
                currentPage = PAGE_MAIN;
                settingsFocus = 0;
            }
            needFullRedraw = true;
        }
    }
    lastPressed = isPressed;
}

// ============================================================
// UI 繪製邏輯 (圓形優化)
// ============================================================

void drawMainPage() {
    tft.fillScreen(CD_BG);
    // 繪製圓形邊框刻度
    for (int i = 0; i < 360; i += 30) {
        float rad = i * DEG_TO_RAD;
        int x1 = 120 + cos(rad) * 110;
        int y1 = 120 + sin(rad) * 110;
        int x2 = 120 + cos(rad) * 118;
        int y2 = 120 + sin(rad) * 118;
        tft.drawLine(x1, y1, x2, y2, CD_GRAY);
    }
}

void updateMainPage() {
    BrewStatus st = engine.getStatus();
    char buf[32];
    
    // 1. 中心大型數值: 已萃取量
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(CD_CREAM, CD_BG);
    tft.setTextSize(4);
    sprintf(buf, "%.0f", st.estimatedMl);
    tft.drawString(buf, 120, 100);
    
    tft.setTextSize(1);
    tft.setTextColor(CD_GRAY, CD_BG);
    tft.drawString("ml extracted", 120, 135);
    
    // 2. 進度環 (外圈)
    float progress = (st.totalCycles > 0) ? (float)st.cycleCount * 100.0f / st.totalCycles : 0;
    UIComponents::drawArcProgress(&tft, 120, 120, 118, 4, -90, 270, progress, CD_COFFEE);
    
    // 3. 狀態文字
    tft.setTextSize(2);
    uint16_t stateColor = CD_GRAY;
    const char* stateStr = "IDLE";
    if (st.state == BREW_RUNNING) { stateColor = CD_GREEN; stateStr = "BREWING"; }
    else if (st.state == BREW_PURGING) { stateColor = CD_AMBER; stateStr = "PURGING"; }
    else if (st.state == BREW_PAUSED) { stateColor = CD_AMBER; stateStr = "PAUSED"; }
    
    tft.setTextColor(stateColor, CD_BG);
    tft.drawString(stateStr, 120, 60);
    
    // 4. 電池 (底部)
    UIComponents::drawCircularBattery(&tft, 120, 120, st.batteryPercent, st.batteryVoltage);
    
    // 5. 動畫
    if (st.pumpActive || st.airPumpActive) {
        UIComponents::drawCircularDrip(&tft, 120, 120, animFrame);
    }
}

void drawSettingsPage() {
    tft.fillScreen(CD_BG);
    tft.setTextColor(CD_CREAM);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("SETTINGS", 120, 40);
}

void updateSettingsPage() {
    char buf[32];
    tft.setTextDatum(MC_DATUM);
    
    // 滴灌間隔
    UIComponents::drawFocusCircle(&tft, 120, 100, 45, settingsFocus == 0);
    tft.setTextColor(CD_WHITE, CD_BG);
    tft.setTextSize(3);
    sprintf(buf, "%d", settingDripInterval);
    tft.drawString(buf, 120, 100);
    tft.setTextSize(1);
    tft.drawString("sec interval", 120, 130);
    
    // 幫浦時間
    UIComponents::drawFocusCircle(&tft, 120, 180, 35, settingsFocus == 1);
    tft.setTextColor(CD_GRAY, CD_BG);
    tft.setTextSize(2);
    sprintf(buf, "%d", settingPumpDuration);
    tft.drawString(buf, 120, 180);
    tft.setTextSize(1);
    tft.drawString("ms duration", 120, 205);
}
