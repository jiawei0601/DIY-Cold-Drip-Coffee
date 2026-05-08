#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置 (v3.3)
// 硬體平台: ESP32 DevKit V1 (WROOM-32)
// 顯示器: 1.3" OLED (SH1106) - 128x64 一體化模組
// 互動: 整合式旋轉編碼器 (EC11)
// 驅動: 2 組 LR7843 MOSFET 模組 (高電平觸發)
// 電源: 2S 18650 (7.4V) + XL4015 降壓 (5V)
// ============================================================

// --- OLED 顯示器 (I2C) ---
#define OLED_SDA_PIN        21      
#define OLED_SCL_PIN        22      
#define OLED_ADDRESS        0x3C    

// --- MOSFET 控制 (High Level Trigger) ---
#define PUMP_PIN            25      // 水泵
#define AIR_PUMP_PIN        26      // 氣泵
#define ACTIVATE_SIGNAL     HIGH    
#define DEACTIVATE_SIGNAL    LOW     

// --- 整合式編碼器 (模組引腳) ---
#define ENCODER_CLK_PIN     32      
#define ENCODER_DT_PIN      33      
#define ENCODER_SW_PIN      35      

// --- 電量監測 (使用 30K/10K 分壓電阻) ---
#define BATTERY_ADC_PIN     34      
#define VOLTAGE_DIVIDER_R1  30000.0f
#define VOLTAGE_DIVIDER_R2  10000.0f
#define ADC_REF_VOLTAGE     3.3f
#define ADC_RESOLUTION      4095.0f

// --- 電池參數 (2S Li-ion) ---
#define BATTERY_FULL_V      8.4f
#define BATTERY_NOMINAL_V   7.4f
#define BATTERY_LOW_V       6.4f

// --- 冰滴控制參數 ---
#define DRIP_INTERVAL_SEC   60
#define PUMP_ON_DURATION_MS 2500

#endif
