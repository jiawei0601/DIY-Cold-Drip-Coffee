#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <U8g2lib.h>
#include "Style.h"
#include "Config.h"

// ============================================================
// 冰滴咖啡機 - v3.0 OLED (128x64) UI 元件
// ============================================================

class UIComponents {
public:
    // 繪製頂部狀態列
    static void drawHeader(U8X8* u8x8, const char* status, int battery) {
        char buf[16];
        sprintf(buf, "%-8s %3d%%", status, battery);
        u8x8->drawStr(0, 0, buf);
    }
    
    // 繪製大數值 (使用 U8G2)
    static void drawValue(U8G2& u8g2, int y, const char* label, float value, const char* unit) {
        char buf[32];
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, y - 15, label);
        
        u8g2.setFont(u8g2_font_logisoso24_tn);
        sprintf(buf, "%.0f", value);
        u8g2.drawStr(0, y + 10, buf);
        
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(60, y + 10, unit);
    }

    // 繪製進度條 (單色)
    static void drawProgressBar(U8G2& u8g2, int x, int y, int w, int h, float percent) {
        u8g2.drawFrame(x, y, w, h);
        int fillW = (int)((w - 4) * percent / 100.0f);
        if (fillW > 0) {
            u8g2.drawBox(x + 2, y + 2, fillW, h - 4);
        }
    }
};

#endif
