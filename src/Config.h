#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置
// ESP32-CYD (ESP32-2432S028)
// ============================================================

// --- 幫浦控制 (Kamoer NKP via D4184 MOSFET) ---
// CYD 可用 GPIO: 27, 22 (未被螢幕/觸控佔用)
#define PUMP_PIN            27      // D4184 MOSFET 訊號腳位

// --- 電量監測 (分壓電阻: 100kΩ / 10kΩ) ---
// ADC 腳位: GPIO35 (ADC1_CH7, 僅輸入, 不與觸控衝突)
#define BATTERY_ADC_PIN     35
#define VOLTAGE_DIVIDER_R1  100000.0f  // 上臂電阻 100kΩ
#define VOLTAGE_DIVIDER_R2  10000.0f   // 下臂電阻 10kΩ
#define ADC_REF_VOLTAGE     3.3f       // ESP32 ADC 參考電壓
#define ADC_RESOLUTION      4095.0f    // 12-bit ADC

// --- 電池參數 (4x Eneloop Pro AA) ---
// 滿電: 4x1.45V = 5.8V, 標稱: 4x1.2V = 4.8V
#define BATTERY_FULL_V      5.8f    // 滿電電壓
#define BATTERY_NOMINAL_V   4.8f    // 標稱電壓
#define BATTERY_LOW_V       4.4f    // 低電量警告
#define BATTERY_CUTOFF_V    4.0f    // 自動停機保護電壓

// --- 冰滴控制參數 ---
#define TARGET_VOLUME_ML    600     // 目標萃取量 (ml)
#define TARGET_TIME_HOURS   6       // 目標萃取時間 (小時)
#define DRIP_INTERVAL_SEC   60      // 滴灌間隔 (秒)
#define PUMP_ON_DURATION_MS 2000    // 每次幫浦啟動時間 (毫秒)

// 計算流量: 每次 ~1.67ml, 每分鐘一次
// 6hr = 360次 x 1.67ml ≈ 600ml
#define FLOW_RATE_ML_PER_CYCLE  1.67f  // 每週期流量 (ml)

// --- 顯示更新頻率 ---
#define DISPLAY_REFRESH_MS  1000    // 畫面更新間隔 (毫秒)
#define BATTERY_READ_MS     5000    // 電壓讀取間隔 (毫秒)

// --- 觸控校準 (CYD 預設值) ---
#define TOUCH_X_MIN     3550
#define TOUCH_X_MAX     350
#define TOUCH_Y_MIN     3750
#define TOUCH_Y_MAX     350

#endif
