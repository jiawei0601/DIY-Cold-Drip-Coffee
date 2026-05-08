#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置 (v2.2)
// 硬體平台: ESP32 DevKit V1 (WROOM-32)
// 顯示器: 1.28" 圓形 LCD (GC9A01) - SPI 介面
// 互動: EC11 旋轉編碼器
// 驅動: 2 路繼電器模組 (低電平觸發)
// 電源: XL4015 降壓模組 (4xAA -> 5V)
// ============================================================

// --- 顯示器配置 (Standard ESP32 VSPI) ---
#define LCD_SCL_PIN         18      // VSPI SCK
#define LCD_SDA_PIN         23      // VSPI MOSI
#define LCD_RES_PIN         4       
#define LCD_DC_PIN          2       
#define LCD_CS_PIN          5       
#define LCD_BLK_PIN         15      

// --- 繼電器控制 (Low Level Trigger) ---
#define PUMP_PIN            25      // 繼電器 IN1 (水泵)
#define AIR_PUMP_PIN        26      // 繼電器 IN2 (氣泵)
#define ACTIVATE_SIGNAL     LOW     // 低電平吸合
#define DEACTIVATE_SIGNAL    HIGH    // 高電平釋放

// --- EC11 旋轉編碼器 ---
#define ENCODER_CLK_PIN     32      
#define ENCODER_DT_PIN      33      
#define ENCODER_SW_PIN      35      // GPIO35 是 Input-only
#define ENCODER_DEBOUNCE_MS 5

// --- 電量監測 (分壓電阻: 100kΩ / 10kΩ) ---
#define BATTERY_ADC_PIN     34      // GPIO34 是 Input-only ADC
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

#endif
