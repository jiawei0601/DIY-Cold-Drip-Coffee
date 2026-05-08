#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <TFT_eSPI.h>
#include "Style.h"
#include "Config.h"

// ============================================================
// 冰滴咖啡機 - v2.0 圓形介面元件繪製工具 (GC9A01)
// ============================================================

class UIComponents {
public:
    // 繪製圓弧進度條 (Arc)
    static void drawArcProgress(TFT_eSPI* tft, int cx, int cy, int r, int thickness, 
                                 int startAngle, int endAngle, float percent, uint16_t color) {
        // 背景圓弧
        tft->drawArc(cx, cy, r, r - thickness, startAngle, endAngle, CD_PROGRESS_BG, CD_BG);
        
        // 前景圓弧
        if (percent > 0) {
            int currentAngle = startAngle + (int)((endAngle - startAngle) * percent / 100.0f);
            tft->drawArc(cx, cy, r, r - thickness, startAngle, currentAngle, color, CD_BG);
        }
    }
    
    // 繪製圓形電量
    static void drawCircularBattery(TFT_eSPI* tft, int cx, int cy, int percent, float voltage) {
        uint16_t color = (percent > 20) ? CD_GREEN : CD_RED;
        
        // 繪製底部小弧線
        drawArcProgress(tft, cx, cy, 115, 6, 150, 210, (float)percent, color);
        
        tft->setTextColor(CD_GRAY);
        tft->setTextDatum(MC_DATUM);
        tft->setTextSize(1);
        char buf[16];
        sprintf(buf, "%.1fV", voltage);
        tft->drawString(buf, cx, cy + 90);
    }
    
    // 繪製滴水動畫 (中心位置)
    static void drawCircularDrip(TFT_eSPI* tft, int cx, int cy, int frame) {
        int dropY = cy - 40 + (frame % 20) * 3;
        if (dropY < cy + 60) {
            tft->fillCircle(cx, dropY, 4, CD_WATER_BLUE);
            tft->fillTriangle(cx - 3, dropY - 2, cx + 3, dropY - 2, cx, dropY - 8, CD_WATER_BLUE);
        }
    }
    
    // 繪製設定焦點 (圓框)
    static void drawFocusCircle(TFT_eSPI* tft, int cx, int cy, int r, bool focused) {
        if (focused) {
            tft->drawCircle(cx, cy, r, CD_WATER_BLUE);
            tft->drawCircle(cx, cy, r - 1, CD_WATER_BLUE);
        } else {
            tft->drawCircle(cx, cy, r, CD_BORDER);
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
};

#endif
