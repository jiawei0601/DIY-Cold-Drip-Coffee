#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置 (v2.0)
// 硬體平台: ESP32-S3 DevKitC
// 顯示器: 1.28" 圓形 LCD (GC9A01) - SPI 介面
// 互動: EC11 旋轉編碼器 (完整接線，不需硬體改裝)
// 全 5V 統一架構 (無升壓模組)
// ============================================================

// --- 顯示器配置 (GC9A01 SPI) ---
#define LCD_SCL_PIN         10      // SPI SCK
#define LCD_SDA_PIN         11      // SPI MOSI
#define LCD_RES_PIN         12      // Reset
#define LCD_DC_PIN          13      // Data/Command
#define LCD_CS_PIN          14      // Chip Select
#define LCD_BLK_PIN         15      // Backlight Control

// --- 水泵控制 (Kamoer NKP 6V via D4184 MOSFET) ---
#define PUMP_PIN            4       // D4184 MOSFET #1

// --- 氣泵控制 (5V Air Pump via D4184 MOSFET #2) ---
#define AIR_PUMP_PIN        5       // D4184 MOSFET #2
#define AIR_PURGE_DURATION_MS 30000 // Air Purge 持續時間
#define AIR_PURGE_ENABLED   true

// --- EC11 旋轉編碼器 ---
// S3 具備充足腳位，可使用具備內部拉高的 GPIO
#define ENCODER_CLK_PIN     1       // EC11 CLK
#define ENCODER_DT_PIN      2       // EC11 DT
#define ENCODER_SW_PIN      3       // EC11 SW (按鈕)
#define ENCODER_DEBOUNCE_MS 5

// --- 電源架構 ---
// 電池: Eneloop Pro AA x4 (串聯 4.8~5.8V)
// 降壓: DC-DC 降壓模組 → 穩定 5V 供應所有元件

// --- 電量監測 (分壓電阻: 100kΩ / 10kΩ) ---
#define BATTERY_ADC_PIN     6       // S3 ADC1_CH5
#define VOLTAGE_DIVIDER_R1  100000.0f
#define VOLTAGE_DIVIDER_R2  10000.0f
#define ADC_REF_VOLTAGE     3.3f
#define ADC_RESOLUTION      4095.0f

// --- 電池參數 ---
#define BATTERY_FULL_V      5.8f
#define BATTERY_NOMINAL_V   4.8f
#define BATTERY_LOW_V       4.4f
#define BATTERY_CUTOFF_V    4.0f

// --- 冰滴控制參數 ---
#define TARGET_VOLUME_ML    600
#define TARGET_TIME_HOURS   6
#define DRIP_INTERVAL_SEC   60
#define PUMP_ON_DURATION_MS 2500
#define FLOW_RATE_ML_PER_CYCLE  1.67f

// --- 更新頻率 ---
#define DISPLAY_REFRESH_MS  500     // S3 效能較佳，可提高更新頻率
#define BATTERY_READ_MS     5000

#endif
