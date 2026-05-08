#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <TFT_eSPI.h>
#include "Style.h"
#include "Config.h"

// ============================================================
// 冰滴咖啡機 - 共用 UI 元件繪製工具
// ============================================================

class UIComponents {
public:
    // 繪製圓角矩形卡片
    static void drawCard(TFT_eSPI* tft, int x, int y, int w, int h, uint16_t bgColor = CD_BG_CARD) {
        tft->fillRoundRect(x, y, w, h, 6, bgColor);
        tft->drawRoundRect(x, y, w, h, 6, CD_BORDER);
    }
    
    // 繪製進度條
    static void drawProgressBar(TFT_eSPI* tft, int x, int y, int w, int h, 
                                 float percent, uint16_t fgColor = CD_PROGRESS_FG) {
        // 背景
        tft->fillRoundRect(x, y, w, h, h/2, CD_PROGRESS_BG);
        // 前景
        int fillW = (int)(w * percent / 100.0f);
        if (fillW > 0) {
            if (fillW < h) fillW = h;  // 最小寬度 = 高度 (圓角)
            tft->fillRoundRect(x, y, fillW, h, h/2, fgColor);
        }
        // 邊框
        tft->drawRoundRect(x, y, w, h, h/2, CD_BORDER);
    }
    
    // 繪製電池圖示
    static void drawBatteryIcon(TFT_eSPI* tft, int x, int y, int percent, float voltage) {
        // 電池外殼 (30x16)
        tft->drawRoundRect(x, y, 30, 16, 2, CD_WHITE);
        tft->fillRect(x + 30, y + 4, 3, 8, CD_WHITE);  // 電池突起
        
        // 電量填充
        uint16_t color;
        if (percent > 50) color = CD_GREEN;
        else if (percent > 20) color = CD_AMBER;
        else color = CD_RED;
        
        int fillW = (int)(26.0f * percent / 100.0f);
        if (fillW > 0) {
            tft->fillRoundRect(x + 2, y + 2, fillW, 12, 1, color);
        }
        
        // 百分比文字
        tft->setTextColor(CD_WHITE, CD_BG);
        tft->setTextSize(1);
        char buf[16];
        sprintf(buf, "%d%% %.1fV", percent, voltage);
        tft->drawString(buf, x + 36, y + 4);
    }
    
    // 繪製大按鈕
    static void drawButton(TFT_eSPI* tft, int x, int y, int w, int h,
                           const char* label, uint16_t bgColor, uint16_t textColor = CD_WHITE) {
        tft->fillRoundRect(x, y, w, h, 8, bgColor);
        tft->drawRoundRect(x, y, w, h, 8, CD_BTN_BORDER);
        // 高光線
        tft->drawFastHLine(x + 4, y + 1, w - 8, tft->alphaBlend(80, CD_WHITE, bgColor));
        
        tft->setTextColor(textColor);
        tft->setTextDatum(MC_DATUM);
        tft->setTextSize(2);
        tft->drawString(label, x + w/2, y + h/2);
        tft->setTextDatum(TL_DATUM);  // 重置
    }
    
    // 繪製數值區塊 (帶標題)
    static void drawValueBlock(TFT_eSPI* tft, int x, int y, int w, int h,
                               const char* title, const char* value,
                               uint16_t valueColor = CD_WHITE) {
        drawCard(tft, x, y, w, h);
        tft->setTextColor(CD_GRAY);
        tft->setTextSize(1);
        tft->drawString(title, x + 6, y + 4);
        tft->setTextColor(valueColor);
        tft->setTextSize(2);
        tft->drawString(value, x + 6, y + 18);
    }
    
    // 繪製滴水動畫圖示
    static void drawDripIcon(TFT_eSPI* tft, int x, int y, int frame) {
        // 水龍頭口
        tft->fillRect(x - 4, y, 8, 3, CD_GRAY);
        
        // 水滴 (動畫)
        int dropY = y + 5 + (frame % 12) * 2;
        if (dropY < y + 30) {
            // 水滴形狀
            tft->fillCircle(x, dropY + 3, 3, CD_WATER_BLUE);
            tft->fillTriangle(x - 2, dropY + 2, x + 2, dropY + 2, x, dropY - 2, CD_WATER_BLUE);
        }
    }
    
    // 格式化時間 HH:MM:SS
    static void formatTime(unsigned long ms, char* buf) {
        unsigned long totalSec = ms / 1000;
        int hours = totalSec / 3600;
        int mins = (totalSec % 3600) / 60;
        int secs = totalSec % 60;
        sprintf(buf, "%02d:%02d:%02d", hours, mins, secs);
    }
    
    // 格式化剩餘時間
    static void formatRemaining(unsigned long elapsedMs, unsigned long totalMs, char* buf) {
        if (elapsedMs >= totalMs) {
            sprintf(buf, "00:00:00");
            return;
        }
        unsigned long remaining = totalMs - elapsedMs;
        formatTime(remaining, buf);
    }
};

#endif
