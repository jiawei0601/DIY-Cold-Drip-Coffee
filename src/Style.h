#ifndef STYLE_H
#define STYLE_H

#include <TFT_eSPI.h>

// ============================================================
// 冰滴咖啡機 - 咖啡色系 UI 調色盤
// ============================================================

// 基底色
#define CD_BG           0x1082      // 深炭黑 (背景)
#define CD_BG_CARD      0x2104      // 深灰卡片背景
#define CD_BORDER       0x4228      // 灰色邊框

// 咖啡主題色
#define CD_COFFEE       0xA301      // 咖啡棕 (主色)
#define CD_ESPRESSO     0x6180      // 深濃縮咖啡色
#define CD_CREAM        0xFED6      // 奶油白
#define CD_LATTE        0xD5B0      // 拿鐵色

// 功能色
#define CD_WATER_BLUE   0x3D7F      // 水藍色 (水量相關)
#define CD_GREEN        0x07E0      // 亮綠 (正常運作)
#define CD_AMBER        0xFDA0      // 琥珀色 (警告)
#define CD_RED          0xF800      // 紅色 (錯誤/停機)
#define CD_WHITE        0xFFFF      // 白色
#define CD_GRAY         0x8410      // 中灰
#define CD_DARK_GRAY    0x3186      // 暗灰

// 進度條色
#define CD_PROGRESS_BG  0x2945      // 進度條背景
#define CD_PROGRESS_FG  0x3D7F      // 進度條前景 (水藍)

// 按鈕色
#define CD_BTN_START    0x0640      // 開始按鈕 (深綠)
#define CD_BTN_STOP     0xC000      // 停止按鈕 (深紅)
#define CD_BTN_ACTIVE   0x07E0      // 啟用中按鈕
#define CD_BTN_BORDER   0x5ACB      // 按鈕邊框

#endif
