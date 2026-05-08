#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置 (v3.0)
// 硬體平台: ESP32 DevKit V1 (WROOM-32)
// 顯示器: 1.3" OLED 模組 (SH1106 / SSD1306) - I2C 介面
// 互動: KY-040 旋轉編碼器模組
// 驅動: 2 路繼電器模組 (低電平觸發)
// 電源: XL4015 降壓模組 (4xAA -> 5V)
// ============================================================

// --- OLED 顯示器 (I2C) ---
#define OLED_SDA_PIN        21      // I2C SDA
#define OLED_SCL_PIN        22      // I2C SCL
#define OLED_ADDRESS        0x3C    // 常見 OLED 位址

// --- 繼電器控制 (Low Level Trigger) ---
#define PUMP_PIN            25      // 繼電器 IN1 (水泵)
#define AIR_PUMP_PIN        26      // 繼電器 IN2 (氣泵)
#define ACTIVATE_SIGNAL     LOW     // 低電平吸合
#define DEACTIVATE_SIGNAL    HIGH    // 高電平釋放

// --- KY-040 編碼器模組 ---
#define ENCODER_CLK_PIN     32      
#define ENCODER_DT_PIN      33      
#define ENCODER_SW_PIN      35      // GPIO35 是 Input-only
#define ENCODER_VCC_PIN     -1      // KY-040 VCC 接 3.3V

// --- 電量監測 (分壓電阻: 100kΩ / 10kΩ) ---
#define BATTERY_ADC_PIN     34      // GPIO34 是 Input-only ADC
#define VOLTAGE_DIVIDER_R1  100000.0f
#define VOLTAGE_DIVIDER_R2  10000.0f
#define ADC_REF_VOLTAGE     3.3f
#define ADC_RESOLUTION      4095.0f

// --- 冰滴控制參數 ---
#define DRIP_INTERVAL_SEC   60
#define PUMP_ON_DURATION_MS 2500

#endif
