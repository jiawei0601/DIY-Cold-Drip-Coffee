#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Config.h"
#include "Style.h"
#include "ColdDripEngine.h"
#include "UIComponents.h"

// ============================================================
// 冰滴咖啡機控制器 - 主程式
// ESP32-CYD (ESP32-2432S028)
// 全 5V 統一架構
// ============================================================

// --- 硬體物件 ---
TFT_eSPI tft = TFT_eSPI();
#define XPT2046_IRQ   36
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS);

// --- 核心引擎 ---
ColdDripEngine engine;

// --- 頁面管理 ---
enum PageIndex { PAGE_MAIN = 0, PAGE_SETTINGS = 1 };
PageIndex currentPage = PAGE_MAIN;

// --- UI 狀態 ---
unsigned long lastDisplayUpdate = 0;
unsigned long lastAnimFrame = 0;
int animFrame = 0;
bool needFullRedraw = true;

// --- 觸控防抖 ---
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE_MS = 350;

// --- 前向宣告 ---
void drawMainPage();
void updateMainPage();
void drawMainButtons(BrewState state);
void drawSettingsPage();
void updateSettingsPage();
void handleMainTouch(int tx, int ty);
void handleSettingsTouch(int tx, int ty);

// ============================================================
// 開機動畫 - 咖啡杯圖示
// ============================================================
void drawSplashScreen() {
    tft.fillScreen(CD_BG);
    
    // 咖啡杯外框
    int cx = 160, cy = 90;
    tft.fillRoundRect(cx - 25, cy - 15, 50, 40, 8, CD_ESPRESSO);
    tft.drawRoundRect(cx - 25, cy - 15, 50, 40, 8, CD_COFFEE);
    // 杯把
    tft.drawArc(cx + 25, cy + 5, 12, 8, 290, 70, CD_COFFEE, CD_BG);
    // 杯底托盤
    tft.fillRoundRect(cx - 30, cy + 25, 60, 5, 2, CD_LATTE);
    
    // 蒸氣動畫 (靜態版)
    for (int i = 0; i < 3; i++) {
        int sx = cx - 10 + i * 10;
        for (int j = 0; j < 3; j++) {
            int sy = cy - 20 - j * 6;
            tft.fillCircle(sx + (j % 2 ? 2 : -2), sy, 2, CD_GRAY);
        }
    }
    
    // 標題
    tft.setTextColor(CD_CREAM);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("COLD DRIP", cx, cy + 55);
    tft.setTextSize(1);
    tft.setTextColor(CD_LATTE);
    tft.drawString("COFFEE CONTROLLER v1.1", cx, cy + 75);
    
    // 底部進度條動畫
    for (int i = 0; i <= 100; i += 2) {
        UIComponents::drawProgressBar(&tft, 60, cy + 100, 200, 8, (float)i, CD_COFFEE);
        delay(20);
    }
    
    tft.setTextDatum(TL_DATUM);
    delay(500);
}

// ============================================================
// 主控頁面繪製
// ============================================================
void drawMainPage() {
    tft.fillScreen(CD_BG);
    
    // 頂部導航列
    tft.fillRect(0, 0, 320, 26, CD_ESPRESSO);
    tft.setTextColor(CD_CREAM);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    
    // 頁面切換按鈕
    // 主控 (左半)
    tft.fillRoundRect(4, 2, 70, 22, 4, CD_COFFEE);
    tft.drawString("MAIN", 39, 13);
    // 設定 (右半)
    tft.fillRoundRect(78, 2, 70, 22, 4, CD_BG_CARD);
    tft.setTextColor(CD_GRAY);
    tft.drawString("CONFIG", 113, 13);
    
    // 電池資訊區 (右上)
    BrewStatus st = engine.getStatus();
    UIComponents::drawBatteryIcon(&tft, 210, 5, st.batteryPercent, st.batteryVoltage);
    
    tft.setTextDatum(TL_DATUM);
    
    // --- 萃取狀態大卡 ---
    UIComponents::drawCard(&tft, 4, 30, 312, 56);
    
    // --- 數值區塊 (2x2 格局) ---
    // 已萃取 | 已耗時
    UIComponents::drawCard(&tft, 4, 90, 153, 46);
    UIComponents::drawCard(&tft, 163, 90, 153, 46);
    
    // 剩餘量 | 剩餘時間
    UIComponents::drawCard(&tft, 4, 140, 153, 46);
    UIComponents::drawCard(&tft, 163, 140, 153, 46);
    
    // --- 底部控制按鈕 ---
    // 按鈕會在 updateMainPage 中繪製
    
    needFullRedraw = false;
}

void updateMainPage() {
    BrewStatus st = engine.getStatus();
    char buf[32];
    
    // --- 更新電池 (右上) ---
    tft.fillRect(210, 2, 108, 22, CD_ESPRESSO);
    UIComponents::drawBatteryIcon(&tft, 210, 5, st.batteryPercent, st.batteryVoltage);
    
    // --- 狀態大卡 ---
    tft.fillRoundRect(6, 32, 308, 52, 4, CD_BG_CARD);
    
    // 狀態文字
    const char* stateStr;
    uint16_t stateColor;
    switch (st.state) {
        case BREW_IDLE:        stateStr = "STANDBY";   stateColor = CD_GRAY; break;
        case BREW_RUNNING:     stateStr = "BREWING";   stateColor = CD_GREEN; break;
        case BREW_PAUSED:      stateStr = "PAUSED";    stateColor = CD_AMBER; break;
        case BREW_PURGING:     stateStr = "PURGING";   stateColor = CD_AMBER; break;
        case BREW_COMPLETE:    stateStr = "COMPLETE";  stateColor = CD_WATER_BLUE; break;
        case BREW_LOW_BATTERY: stateStr = "LOW BATT!"; stateColor = CD_RED; break;
        default:               stateStr = "---";       stateColor = CD_GRAY; break;
    }
    
    // 狀態指示燈
    tft.fillCircle(20, 50, 6, stateColor);
    if (st.state == BREW_RUNNING || st.state == BREW_PURGING) {
        // 呼吸燈效果
        int brightness = abs((int)(millis() / 10) % 200 - 100);
        if (brightness > 50) {
            tft.drawCircle(20, 50, 8, stateColor);
        }
    }
    
    // 狀態文字 (大)
    tft.setTextColor(stateColor);
    tft.setTextSize(2);
    tft.drawString(stateStr, 34, 38);
    
    // 幫浦/氣泵指示
    if (st.pumpActive) {
        tft.setTextColor(CD_WATER_BLUE);
        tft.setTextSize(1);
        tft.drawString("PUMP ON", 34, 60);
        // 滴水動畫
        UIComponents::drawDripIcon(&tft, 290, 36, animFrame);
    } else if (st.airPumpActive) {
        tft.setTextColor(CD_AMBER);
        tft.setTextSize(1);
        tft.drawString("AIR PUMP ON", 34, 60);
        // 顯示氣泵剩餘時間
        char purgeBuf[16];
        int purgeRemain = (st.purgeTotalMs - st.purgeElapsedMs) / 1000;
        if (purgeRemain < 0) purgeRemain = 0;
        sprintf(purgeBuf, "%ds left", purgeRemain);
        tft.drawString(purgeBuf, 140, 60);
    } else if (st.state == BREW_RUNNING) {
        tft.setTextColor(CD_DARK_GRAY);
        tft.setTextSize(1);
        sprintf(buf, "Next: %ds", 
            st.dripIntervalSec - (int)((millis() / 1000) % st.dripIntervalSec));
        tft.drawString(buf, 34, 60);
    }
    
    // 進度條
    if (st.state == BREW_PURGING) {
        // Air Purge 專用進度條
        float purgeProgress = (float)st.purgeElapsedMs / st.purgeTotalMs * 100.0f;
        if (purgeProgress > 100.0f) purgeProgress = 100.0f;
        UIComponents::drawProgressBar(&tft, 10, 72, 300, 8, purgeProgress, CD_AMBER);
        tft.setTextColor(CD_AMBER);
        tft.setTextSize(1);
        sprintf(buf, "%.0f%%", purgeProgress);
        tft.drawString(buf, 268, 60);
    } else {
        // 一般萃取進度條
        float progress = 0;
        if (st.totalCycles > 0) {
            progress = (float)st.cycleCount / st.totalCycles * 100.0f;
        }
        UIComponents::drawProgressBar(&tft, 10, 72, 300, 8, progress, 
            st.state == BREW_COMPLETE ? CD_GREEN : CD_WATER_BLUE);
        tft.setTextColor(CD_CREAM);
        tft.setTextSize(1);
        sprintf(buf, "%.1f%%", progress);
        tft.drawString(buf, 268, 60);
    }
    
    // --- 已萃取 ---
    tft.fillRoundRect(6, 92, 149, 42, 4, CD_BG_CARD);
    tft.setTextColor(CD_GRAY);
    tft.setTextSize(1);
    tft.drawString("Brewed", 12, 94);
    tft.setTextColor(CD_WATER_BLUE);
    tft.setTextSize(2);
    sprintf(buf, "%.0f ml", st.estimatedMl);
    tft.drawString(buf, 12, 110);
    
    // --- 已耗時 ---
    tft.fillRoundRect(165, 92, 149, 42, 4, CD_BG_CARD);
    tft.setTextColor(CD_GRAY);
    tft.setTextSize(1);
    tft.drawString("Elapsed", 171, 94);
    tft.setTextColor(CD_CREAM);
    tft.setTextSize(2);
    UIComponents::formatTime(st.elapsedMs, buf);
    tft.drawString(buf, 171, 110);
    
    // --- 剩餘量 ---
    tft.fillRoundRect(6, 142, 149, 42, 4, CD_BG_CARD);
    tft.setTextColor(CD_GRAY);
    tft.setTextSize(1);
    tft.drawString("Remaining", 12, 144);
    tft.setTextColor(CD_LATTE);
    tft.setTextSize(2);
    float remaining = TARGET_VOLUME_ML - st.estimatedMl;
    if (remaining < 0) remaining = 0;
    sprintf(buf, "%.0f ml", remaining);
    tft.drawString(buf, 12, 160);
    
    // --- 剩餘時間 ---
    tft.fillRoundRect(165, 142, 149, 42, 4, CD_BG_CARD);
    tft.setTextColor(CD_GRAY);
    tft.setTextSize(1);
    tft.drawString("Time Left", 171, 144);
    tft.setTextColor(CD_LATTE);
    tft.setTextSize(2);
    UIComponents::formatRemaining(st.elapsedMs, st.totalMs, buf);
    tft.drawString(buf, 171, 160);
    
    // --- 週期計數 ---
    tft.fillRect(4, 188, 312, 14, CD_BG);
    tft.setTextColor(CD_DARK_GRAY);
    tft.setTextSize(1);
    sprintf(buf, "Cycle: %d / %d    Interval: %ds  Duration: %dms",
        st.cycleCount, st.totalCycles, st.dripIntervalSec, st.pumpDurationMs);
    tft.drawString(buf, 8, 190);
    
    // --- 底部按鈕 ---
    drawMainButtons(st.state);
}

void drawMainButtons(BrewState state) {
    // 清除按鈕區域
    tft.fillRect(4, 205, 312, 33, CD_BG);
    
    switch (state) {
        case BREW_IDLE:
        case BREW_COMPLETE:
        case BREW_LOW_BATTERY:
            // 顯示「開始」按鈕
            UIComponents::drawButton(&tft, 60, 206, 200, 30, "START", CD_BTN_START);
            break;
            
        case BREW_RUNNING:
            // 顯示「暫停」和「停止」
            UIComponents::drawButton(&tft, 10, 206, 145, 30, "PAUSE", CD_AMBER);
            UIComponents::drawButton(&tft, 165, 206, 145, 30, "STOP", CD_BTN_STOP);
            break;
            
        case BREW_PAUSED:
            // 顯示「繼續」和「停止」
            UIComponents::drawButton(&tft, 10, 206, 145, 30, "RESUME", CD_BTN_START);
            UIComponents::drawButton(&tft, 165, 206, 145, 30, "STOP", CD_BTN_STOP);
            break;
            
        case BREW_PURGING:
            // 顯示「跳過」和「停止」
            UIComponents::drawButton(&tft, 10, 206, 145, 30, "SKIP", CD_AMBER);
            UIComponents::drawButton(&tft, 165, 206, 145, 30, "STOP", CD_BTN_STOP);
            break;
    }
}

// ============================================================
// 設定頁面
// ============================================================
int settingDripInterval;
int settingPumpDuration;

void drawSettingsPage() {
    tft.fillScreen(CD_BG);
    
    // 頂部導航列
    tft.fillRect(0, 0, 320, 26, CD_ESPRESSO);
    tft.setTextColor(CD_GRAY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    
    // 主控 (左半)
    tft.fillRoundRect(4, 2, 70, 22, 4, CD_BG_CARD);
    tft.drawString("MAIN", 39, 13);
    // 設定 (右半)
    tft.fillRoundRect(78, 2, 70, 22, 4, CD_COFFEE);
    tft.setTextColor(CD_CREAM);
    tft.drawString("CONFIG", 113, 13);
    
    tft.setTextDatum(TL_DATUM);
    
    // 讀取當前參數
    BrewStatus st = engine.getStatus();
    settingDripInterval = st.dripIntervalSec;
    settingPumpDuration = st.pumpDurationMs;
    
    updateSettingsPage();
}

void updateSettingsPage() {
    BrewStatus st = engine.getStatus();
    char buf[32];
    
    // --- 電池資訊 ---
    tft.fillRect(210, 2, 108, 22, CD_ESPRESSO);
    UIComponents::drawBatteryIcon(&tft, 210, 5, st.batteryPercent, st.batteryVoltage);
    
    // --- 滴灌間隔設定 ---
    tft.fillRect(4, 30, 312, 80, CD_BG);
    UIComponents::drawCard(&tft, 4, 30, 312, 76);
    
    tft.setTextColor(CD_CREAM);
    tft.setTextSize(1);
    tft.drawString("Drip Interval (seconds)", 12, 36);
    
    // - 按鈕
    UIComponents::drawButton(&tft, 12, 54, 50, 40, "-", CD_ESPRESSO);
    // 數值
    tft.setTextColor(CD_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    sprintf(buf, "%d", settingDripInterval);
    tft.drawString(buf, 160, 74);
    tft.setTextSize(1);
    tft.setTextColor(CD_GRAY);
    tft.drawString("sec", 160, 92);
    // + 按鈕
    UIComponents::drawButton(&tft, 258, 54, 50, 40, "+", CD_ESPRESSO);
    
    tft.setTextDatum(TL_DATUM);
    
    // --- 幫浦持續時間設定 ---
    tft.fillRect(4, 112, 312, 80, CD_BG);
    UIComponents::drawCard(&tft, 4, 112, 312, 76);
    
    tft.setTextColor(CD_CREAM);
    tft.setTextSize(1);
    tft.drawString("Pump Duration (milliseconds)", 12, 118);
    
    // - 按鈕
    UIComponents::drawButton(&tft, 12, 136, 50, 40, "-", CD_ESPRESSO);
    // 數值
    tft.setTextColor(CD_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    sprintf(buf, "%d", settingPumpDuration);
    tft.drawString(buf, 160, 156);
    tft.setTextSize(1);
    tft.setTextColor(CD_GRAY);
    tft.drawString("ms", 160, 174);
    // + 按鈕
    UIComponents::drawButton(&tft, 258, 136, 50, 40, "+", CD_ESPRESSO);
    
    tft.setTextDatum(TL_DATUM);
    
    // --- 預估流量資訊 ---
    tft.fillRect(4, 194, 312, 44, CD_BG);
    UIComponents::drawCard(&tft, 4, 194, 312, 42);
    
    float mlPerCycle = FLOW_RATE_ML_PER_CYCLE * ((float)settingPumpDuration / PUMP_ON_DURATION_MS);
    float mlPerMin = mlPerCycle * (60.0f / settingDripInterval);
    int totalCycles = (TARGET_TIME_HOURS * 3600) / settingDripInterval;
    float totalMl = totalCycles * mlPerCycle;
    
    tft.setTextColor(CD_GRAY);
    tft.setTextSize(1);
    sprintf(buf, "Flow: %.2f ml/min", mlPerMin);
    tft.drawString(buf, 12, 200);
    sprintf(buf, "Cycles: %d", totalCycles);
    tft.drawString(buf, 160, 200);
    
    tft.setTextColor(CD_WATER_BLUE);
    sprintf(buf, "Est. Total: %.0f ml / %dhr", totalMl, TARGET_TIME_HOURS);
    tft.drawString(buf, 12, 218);
}

// ============================================================
// 觸控處理
// ============================================================
void handleMainTouch(int tx, int ty) {
    BrewState state = engine.getState();
    
    // 頁面切換 - 設定頁
    if (ty < 26 && tx >= 78 && tx < 148) {
        currentPage = PAGE_SETTINGS;
        drawSettingsPage();
        return;
    }
    
    // 按鈕區域 (y: 206~236)
    if (ty >= 206 && ty <= 236) {
        switch (state) {
            case BREW_IDLE:
            case BREW_COMPLETE:
            case BREW_LOW_BATTERY:
                // START 按鈕 (60~260)
                if (tx >= 60 && tx <= 260) {
                    engine.startBrew();
                    needFullRedraw = true;
                }
                break;
                
            case BREW_RUNNING:
                // PAUSE (10~155)
                if (tx >= 10 && tx <= 155) {
                    engine.pauseBrew();
                    needFullRedraw = true;
                }
                // STOP (165~310)
                else if (tx >= 165 && tx <= 310) {
                    engine.stopBrew();
                    needFullRedraw = true;
                }
                break;
                
            case BREW_PAUSED:
                // RESUME (10~155)
                if (tx >= 10 && tx <= 155) {
                    engine.resumeBrew();
                    needFullRedraw = true;
                }
                // STOP (165~310)
                else if (tx >= 165 && tx <= 310) {
                    engine.stopBrew();
                    needFullRedraw = true;
                }
                break;
                
            case BREW_PURGING:
                // SKIP (10~155) - 跳過 Air Purge
                if (tx >= 10 && tx <= 155) {
                    engine.stopPurge();
                    needFullRedraw = true;
                }
                // STOP (165~310) - 完全停止
                else if (tx >= 165 && tx <= 310) {
                    engine.stopBrew();
                    needFullRedraw = true;
                }
                break;
        }
    }
}

void handleSettingsTouch(int tx, int ty) {
    // 頁面切換 - 主控頁
    if (ty < 26 && tx >= 4 && tx < 74) {
        // 套用設定
        engine.setDripInterval(settingDripInterval);
        engine.setPumpDuration(settingPumpDuration);
        currentPage = PAGE_MAIN;
        needFullRedraw = true;
        drawMainPage();
        return;
    }
    
    // --- 滴灌間隔 ---
    // - 按鈕 (12~62, 54~94)
    if (tx >= 12 && tx <= 62 && ty >= 54 && ty <= 94) {
        settingDripInterval -= 10;
        if (settingDripInterval < 10) settingDripInterval = 10;
        updateSettingsPage();
        return;
    }
    // + 按鈕 (258~308, 54~94)
    if (tx >= 258 && tx <= 308 && ty >= 54 && ty <= 94) {
        settingDripInterval += 10;
        if (settingDripInterval > 300) settingDripInterval = 300;
        updateSettingsPage();
        return;
    }
    
    // --- 幫浦持續時間 ---
    // - 按鈕 (12~62, 136~176)
    if (tx >= 12 && tx <= 62 && ty >= 136 && ty <= 176) {
        settingPumpDuration -= 100;
        if (settingPumpDuration < 500) settingPumpDuration = 500;
        updateSettingsPage();
        return;
    }
    // + 按鈕 (258~308, 136~176)
    if (tx >= 258 && tx <= 308 && ty >= 136 && ty <= 176) {
        settingPumpDuration += 100;
        if (settingPumpDuration > 5000) settingPumpDuration = 5000;
        updateSettingsPage();
        return;
    }
}

// ============================================================
// Arduino 主程式
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n========================================");
    Serial.println("  Cold Drip Coffee Controller v1.1");
    Serial.println("  ESP32-CYD | 5V Unified Architecture");
    Serial.println("========================================\n");
    
    // 背光
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    
    // TFT 初始化
    tft.init();
    tft.setRotation(3);       // 橫向, USB 在右
    tft.invertDisplay(true);   // CYD 需要反色
    tft.fillScreen(CD_BG);
    
    // 觸控初始化
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(3);
    
    // 開機畫面
    drawSplashScreen();
    
    // 初始化引擎
    engine.begin();
    
    // 繪製主頁面
    drawMainPage();
    updateMainPage();
}

void loop() {
    unsigned long now = millis();
    
    // 更新引擎 (永遠執行)
    engine.update();
    
    // 更新顯示
    if (now - lastDisplayUpdate >= DISPLAY_REFRESH_MS) {
        lastDisplayUpdate = now;
        
        if (currentPage == PAGE_MAIN) {
            if (needFullRedraw) {
                drawMainPage();
                needFullRedraw = false;
            }
            updateMainPage();
        } else if (currentPage == PAGE_SETTINGS) {
            // 設定頁面只更新電池
            BrewStatus st = engine.getStatus();
            tft.fillRect(210, 2, 108, 22, CD_ESPRESSO);
            UIComponents::drawBatteryIcon(&tft, 210, 5, st.batteryPercent, st.batteryVoltage);
        }
    }
    
    // 動畫幀更新
    if (now - lastAnimFrame >= 200) {
        lastAnimFrame = now;
        animFrame++;
    }
    
    // 觸控處理
    if (touch.touched() && (now - lastTouchTime >= TOUCH_DEBOUNCE_MS)) {
        lastTouchTime = now;
        TS_Point p = touch.getPoint();
        int tx = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, 320);
        int ty = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, 240);
        tx = constrain(tx, 0, 319);
        ty = constrain(ty, 0, 239);
        
        Serial.printf("[Touch] X=%d, Y=%d (Raw: %d, %d)\n", tx, ty, p.x, p.y);
        
        if (currentPage == PAGE_MAIN) {
            handleMainTouch(tx, ty);
        } else if (currentPage == PAGE_SETTINGS) {
            handleSettingsTouch(tx, ty);
        }
    }
    
    delay(10);
}
