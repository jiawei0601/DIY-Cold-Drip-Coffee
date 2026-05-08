#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <Wire.h>
#include "Config.h"
#include "ColdDripEngine.h"

// ============================================================
// 冰滴咖啡機控制器 v3.2 (MOSFET + SH1107 版)
// 平台: ESP32-WROOM-32 (DevKit V1)
// 顯示: 1.5" OLED (SH1107 128x128 I2C)
// 驅動: 2x LR7843 MOSFET 模組 (靜音)
// ============================================================

// 初始化 OLED (SH1107 128x128 正方形螢幕)
U8G2_SH1107_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

ESP32Encoder encoder;
ColdDripEngine engine;

// --- UI 狀態 ---
unsigned long lastDisplayUpdate = 0;
const int DISPLAY_REFRESH_MS = 200;

void setup() {
    Serial.begin(115200);
    
    // 初始化 I2C 與 OLED
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    u8g2.begin();
    
    // 初始化編碼器
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder.attachFullQuad(ENCODER_CLK_PIN, ENCODER_DT_PIN);
    pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
    
    // 初始化引擎
    engine.begin();
    
    Serial.println("System v3.2 Initialized (WROOM + SH1107 + MOSFET)");
}

void loop() {
    unsigned long now = millis();
    engine.update();
    
    // 處理編碼器與按鈕
    static long lastEncoderPos = 0;
    long currentPos = encoder.getCount() / 2;
    if (currentPos != lastEncoderPos) {
        int delta = currentPos - lastEncoderPos;
        lastEncoderPos = currentPos;
        int interval = engine.getStatus().dripIntervalSec + delta;
        engine.setDripInterval(constrain(interval, 10, 300));
    }
    
    static bool lastBtn = HIGH;
    bool currentBtn = digitalRead(ENCODER_SW_PIN);
    if (currentBtn == LOW && lastBtn == HIGH) { // 按下旋鈕
        BrewState state = engine.getState();
        if (state == BREW_IDLE || state == BREW_COMPLETE) engine.startBrew();
        else if (state == BREW_RUNNING) engine.pauseBrew();
        else if (state == BREW_PAUSED) engine.resumeBrew();
    }
    lastBtn = currentBtn;

    // 更新顯示 (128x128 滿版優化)
    if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
        lastDisplayUpdate = now;
        
        BrewStatus st = engine.getStatus();
        u8g2.clearBuffer();
        
        // 1. 頂部狀態列
        u8g2.setFont(u8g2_font_6x10_tf);
        const char* stateStr = "IDLE";
        if (st.state == BREW_RUNNING) stateStr = "BREWING...";
        else if (st.state == BREW_PURGING) stateStr = "PURGING...";
        else if (st.state == BREW_PAUSED) stateStr = "PAUSED";
        u8g2.drawStr(0, 10, stateStr);
        
        char buf[32];
        sprintf(buf, "%d%%", st.batteryPercent);
        u8g2.drawStr(100, 10, buf);
        u8g2.drawLine(0, 14, 128, 14);
        
        // 2. 中央大數值: 已萃取量 (ml)
        u8g2.setFont(u8g2_font_logisoso32_tn); // 使用更大的字體
        sprintf(buf, "%.0f", st.estimatedMl);
        u8g2.drawStr(15, 65, buf);
        
        u8g2.setFont(u8g2_font_6x12_tf);
        u8g2.drawStr(85, 65, "ml");
        u8g2.drawStr(15, 80, "EXTRACTED");
        
        // 3. 底部資訊
        u8g2.drawLine(0, 95, 128, 95);
        sprintf(buf, "Interval: %ds", st.dripIntervalSec);
        u8g2.drawStr(0, 110, buf);
        
        sprintf(buf, "V: %.2fV", st.batteryVoltage);
        u8g2.drawStr(75, 110, buf);
        
        // 進度條
        float progress = (st.totalCycles > 0) ? (float)st.cycleCount * 100.0f / st.totalCycles : 0;
        u8g2.drawFrame(0, 118, 128, 8);
        u8g2.drawBox(2, 120, (int)(124 * progress / 100.0f), 4);
        
        u8g2.sendBuffer();
    }
}
