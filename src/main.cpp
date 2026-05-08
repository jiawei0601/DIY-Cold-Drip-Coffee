#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP32Encoder.h>
#include <Wire.h>
#include "Config.h"
#include "ColdDripEngine.h"

// ============================================================
// 冰滴咖啡機控制器 v3.3 (一體化面板版)
// 平台: ESP32-WROOM-32 (DevKit V1)
// 顯示: 1.3" OLED (SH1106 128x64 I2C)
// 旋鈕: 整合式 EC11 編碼器
// 驅動: 2x LR7843 MOSFET 模組 (靜音)
// ============================================================

// 初始化 OLED (1.3" 一體化模組通常為 SH1106 驅動)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

ESP32Encoder encoder;
ColdDripEngine engine;

unsigned long lastDisplayUpdate = 0;
const int DISPLAY_REFRESH_MS = 250;

void setup() {
    Serial.begin(115200);
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    u8g2.begin();
    
    ESP32Encoder::useInternalWeakPullResistors = UP;
    encoder.attachFullQuad(ENCODER_CLK_PIN, ENCODER_DT_PIN);
    pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
    
    engine.begin();
    Serial.println("System v3.3 Initialized (Integrated Module + MOSFET)");
}

void loop() {
    unsigned long now = millis();
    engine.update();
    
    // 處理編碼器
    static long lastEncoderPos = 0;
    long currentPos = encoder.getCount() / 2;
    if (currentPos != lastEncoderPos) {
        int delta = currentPos - lastEncoderPos;
        lastEncoderPos = currentPos;
        int interval = engine.getStatus().dripIntervalSec + delta;
        engine.setDripInterval(constrain(interval, 10, 300));
    }
    
    // 處理按鈕
    static bool lastBtn = HIGH;
    bool currentBtn = digitalRead(ENCODER_SW_PIN);
    if (currentBtn == LOW && lastBtn == HIGH) {
        BrewState state = engine.getState();
        if (state == BREW_IDLE || state == BREW_COMPLETE) engine.startBrew();
        else if (state == BREW_RUNNING) engine.pauseBrew();
        else if (state == BREW_PAUSED) engine.resumeBrew();
    }
    lastBtn = currentBtn;

    // 更新顯示 (128x64 經典佈局優化)
    if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
        lastDisplayUpdate = now;
        BrewStatus st = engine.getStatus();
        u8g2.clearBuffer();
        
        // 1. 頂部狀態列
        u8g2.setFont(u8g2_font_6x10_tf);
        const char* stateStr = "READY";
        if (st.state == BREW_RUNNING) stateStr = "BREWING...";
        else if (st.state == BREW_PURGING) stateStr = "CLEANING";
        else if (st.state == BREW_PAUSED) stateStr = "PAUSED";
        u8g2.drawStr(0, 10, stateStr);
        
        char buf[16];
        sprintf(buf, "%d%%", st.batteryPercent);
        u8g2.drawStr(100, 10, buf);
        
        // 2. 中央大數值 (萃取量 ml)
        u8g2.setFont(u8g2_font_logisoso24_tn); 
        sprintf(buf, "%.0f", st.estimatedMl);
        u8g2.drawStr(40, 42, buf);
        u8g2.setFont(u8g2_font_6x12_tf);
        u8g2.drawStr(85, 42, "ml");
        
        // 3. 底部資訊列
        u8g2.setFont(u8g2_font_5x7_tf);
        sprintf(buf, "INTV:%ds", st.dripIntervalSec);
        u8g2.drawStr(0, 56, buf);
        
        sprintf(buf, "VOLT:%.1fV", st.batteryVoltage);
        u8g2.drawStr(75, 56, buf);
        
        // 4. 底部進度條
        float progress = (st.totalCycles > 0) ? (float)st.cycleCount * 100.0f / st.totalCycles : 0;
        u8g2.drawFrame(0, 60, 128, 4);
        u8g2.drawBox(1, 61, (int)(126 * progress / 100.0f), 2);
        
        u8g2.sendBuffer();
    }
}
