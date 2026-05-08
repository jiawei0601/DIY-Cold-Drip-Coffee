#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// 冰滴咖啡機控制器 - 硬體與參數配置
// ESP32-CYD (ESP32-2432S028)
// 全 5V 統一架構 (無升壓模組)
// ============================================================

// --- 水泵控制 (Kamoer NKP 6V via D4184 MOSFET) ---
// 6V 額定蠕動幫浦，5V 供電下流量約降 15-20%
// CYD 可用 GPIO: 27, 22 (未被螢幕/觸控佔用)
#define PUMP_PIN            27      // D4184 MOSFET #1 訊號腳位 (水泵)

// --- 氣泵控制 (5V Air Pump via D4184 MOSFET #2) ---
#define AIR_PUMP_PIN        22      // D4184 MOSFET #2 訊號腳位 (氣泵)
#define AIR_PURGE_DURATION_MS 30000 // Air Purge 持續時間 (毫秒, 預設 30 秒)
#define AIR_PURGE_ENABLED   true    // 萃取完成後自動執行 Air Purge

// --- EC11 旋轉編碼器 ---
// 需拆除 CYD 板上 RGB LED 以釋放 GPIO16/GPIO17
// GPIO35 為 input-only，適合做按鈕輸入
#define ENCODER_CLK_PIN     16      // EC11 CLK (Phase A) - 原 RGB LED Green
#define ENCODER_DT_PIN      17      // EC11 DT  (Phase B) - 原 RGB LED Blue
#define ENCODER_SW_PIN      35      // EC11 SW  (按鈕) - 共用 ADC 腳位 (input-only)
#define ENCODER_DEBOUNCE_MS 5       // 編碼器消抖時間 (毫秒)

// --- 電源架構 ---
// 電池: Eneloop Pro AA x4 (串聯 4.8~5.8V)
// 降壓: DC-DC 降壓模組 → 穩定 5V
// 供電: 5V → ESP32 CYD + 水泵 (D4184 #1) + 氣泵 (D4184 #2)
// 注意: 不需要升壓模組，所有元件統一 5V 供電

// --- 電量監測 (分壓電阻: 100kΩ / 10kΩ) ---
// ADC 腳位: GPIO35 (ADC1_CH7, 僅輸入, 與 EC11 SW 共用)
// 透過軟體切換讀取模式: 平時讀電壓，按下時讀按鈕
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
#define PUMP_ON_DURATION_MS 2500    // 每次幫浦啟動時間 (毫秒, 5V 流量補償)

// 計算流量: 6V 泵在 5V 下流量約降 20%, 延長 ON 時間補償
// 每次 ~1.67ml, 每分鐘一次
// 6hr = 360次 x 1.67ml ≈ 600ml
#define FLOW_RATE_ML_PER_CYCLE  1.67f  // 每週期流量 (ml, 校準後調整)

// --- 顯示更新頻率 ---
#define DISPLAY_REFRESH_MS  1000    // 畫面更新間隔 (毫秒)
#define BATTERY_READ_MS     5000    // 電壓讀取間隔 (毫秒)

// --- 觸控校準 (CYD 預設值) ---
#define TOUCH_X_MIN     3550
#define TOUCH_X_MAX     350
#define TOUCH_Y_MIN     3750
#define TOUCH_Y_MAX     350

#endif
