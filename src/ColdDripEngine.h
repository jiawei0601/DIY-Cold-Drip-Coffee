#ifndef COLD_DRIP_ENGINE_H
#define COLD_DRIP_ENGINE_H

#include <Arduino.h>
#include "Config.h"

// ============================================================
// 冰滴咖啡機 - 核心控制引擎
// 管理幫浦控制、電量監測、萃取排程
// ============================================================

enum BrewState {
    BREW_IDLE,       // 待機中
    BREW_RUNNING,    // 萃取進行中
    BREW_PAUSED,     // 暫停中
    BREW_COMPLETE,   // 萃取完成
    BREW_LOW_BATTERY // 低電量停機
};

struct BrewStatus {
    BrewState state;
    unsigned long elapsedMs;        // 已經過時間 (毫秒)
    unsigned long totalMs;          // 總預計時間 (毫秒)
    float estimatedMl;              // 已萃取估計量 (ml)
    int cycleCount;                 // 已完成週期數
    int totalCycles;                // 總預計週期數
    float batteryVoltage;           // 電池電壓
    int batteryPercent;             // 電池百分比
    bool pumpActive;                // 幫浦是否正在運轉
    int dripIntervalSec;            // 當前滴灌間隔
    int pumpDurationMs;             // 當前幫浦持續時間
};

class ColdDripEngine {
public:
    ColdDripEngine();
    
    void begin();
    void update();  // 主循環呼叫
    
    // 控制指令
    void startBrew();
    void stopBrew();
    void pauseBrew();
    void resumeBrew();
    
    // 參數調整
    void setDripInterval(int seconds);
    void setPumpDuration(int milliseconds);
    
    // 狀態查詢
    BrewStatus getStatus();
    BrewState getState() { return _state; }
    float getBatteryVoltage();
    int getBatteryPercent();
    bool isPumpOn() { return _pumpOn; }
    
private:
    BrewState _state;
    bool _pumpOn;
    
    // 時間追蹤
    unsigned long _brewStartMs;
    unsigned long _pauseStartMs;
    unsigned long _totalPausedMs;
    unsigned long _lastDripMs;
    unsigned long _pumpStartMs;
    unsigned long _lastBatteryReadMs;
    
    // 萃取參數
    int _dripIntervalSec;
    int _pumpDurationMs;
    int _cycleCount;
    int _totalCycles;
    
    // 電池
    float _batteryVoltage;
    int _batteryPercent;
    float _batteryReadings[8];  // 滑動平均
    int _batteryReadIndex;
    
    // 內部方法
    void _readBattery();
    void _pumpOn_();
    void _pumpOff();
    void _checkSafety();
    float _rawAdcToVoltage(int raw);
};

// ============================================================
// 實作
// ============================================================

ColdDripEngine::ColdDripEngine()
    : _state(BREW_IDLE), _pumpOn(false),
      _brewStartMs(0), _pauseStartMs(0), _totalPausedMs(0),
      _lastDripMs(0), _pumpStartMs(0), _lastBatteryReadMs(0),
      _dripIntervalSec(DRIP_INTERVAL_SEC),
      _pumpDurationMs(PUMP_ON_DURATION_MS),
      _cycleCount(0), _batteryVoltage(0), _batteryPercent(0),
      _batteryReadIndex(0) {
    
    _totalCycles = (TARGET_TIME_HOURS * 3600) / _dripIntervalSec;
    
    for (int i = 0; i < 8; i++) _batteryReadings[i] = 0;
}

void ColdDripEngine::begin() {
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);  // 確保幫浦初始關閉
    
    // 設定 ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);  // 0~3.3V 範圍
    
    // 初始電壓讀取 (填滿滑動平均)
    for (int i = 0; i < 8; i++) {
        int raw = analogRead(BATTERY_ADC_PIN);
        _batteryReadings[i] = _rawAdcToVoltage(raw);
        delay(10);
    }
    _readBattery();
    
    Serial.println("[ColdDrip] 引擎初始化完成");
    Serial.printf("[ColdDrip] 電池電壓: %.2fV (%d%%)\n", _batteryVoltage, _batteryPercent);
}

void ColdDripEngine::update() {
    unsigned long now = millis();
    
    // 定期讀取電池電壓
    if (now - _lastBatteryReadMs >= BATTERY_READ_MS) {
        _readBattery();
        _lastBatteryReadMs = now;
    }
    
    // 安全檢查
    _checkSafety();
    
    if (_state != BREW_RUNNING) {
        // 確保非運作狀態時幫浦關閉
        if (_pumpOn) _pumpOff();
        return;
    }
    
    // --- 萃取控制邏輯 ---
    
    // 檢查幫浦是否需要關閉
    if (_pumpOn && (now - _pumpStartMs >= (unsigned long)_pumpDurationMs)) {
        _pumpOff();
        _cycleCount++;
        Serial.printf("[ColdDrip] 週期 %d/%d 完成\n", _cycleCount, _totalCycles);
    }
    
    // 檢查是否完成所有萃取
    unsigned long activeMs = now - _brewStartMs - _totalPausedMs;
    unsigned long totalTargetMs = (unsigned long)TARGET_TIME_HOURS * 3600UL * 1000UL;
    
    if (activeMs >= totalTargetMs || _cycleCount >= _totalCycles) {
        _pumpOff();
        _state = BREW_COMPLETE;
        Serial.println("[ColdDrip] 萃取完成！");
        return;
    }
    
    // 檢查是否啟動幫浦
    if (!_pumpOn && (now - _lastDripMs >= (unsigned long)_dripIntervalSec * 1000UL)) {
        _pumpOn_();
        _lastDripMs = now;
        _pumpStartMs = now;
    }
}

void ColdDripEngine::startBrew() {
    if (_state == BREW_RUNNING) return;
    
    // 安全檢查
    if (_batteryVoltage > 0 && _batteryVoltage < BATTERY_CUTOFF_V) {
        _state = BREW_LOW_BATTERY;
        Serial.println("[ColdDrip] 電量不足，無法啟動！");
        return;
    }
    
    _state = BREW_RUNNING;
    _brewStartMs = millis();
    _totalPausedMs = 0;
    _cycleCount = 0;
    _lastDripMs = millis() - ((unsigned long)_dripIntervalSec * 1000UL);  // 立即首次滴灌
    _totalCycles = (TARGET_TIME_HOURS * 3600) / _dripIntervalSec;
    
    Serial.println("[ColdDrip] 開始萃取！");
    Serial.printf("[ColdDrip] 間隔: %ds, 每次: %dms, 總週期: %d\n",
        _dripIntervalSec, _pumpDurationMs, _totalCycles);
}

void ColdDripEngine::stopBrew() {
    _pumpOff();
    _state = BREW_IDLE;
    _cycleCount = 0;
    _brewStartMs = 0;
    _totalPausedMs = 0;
    Serial.println("[ColdDrip] 萃取已停止");
}

void ColdDripEngine::pauseBrew() {
    if (_state != BREW_RUNNING) return;
    _pumpOff();
    _state = BREW_PAUSED;
    _pauseStartMs = millis();
    Serial.println("[ColdDrip] 萃取已暫停");
}

void ColdDripEngine::resumeBrew() {
    if (_state != BREW_PAUSED) return;
    _totalPausedMs += millis() - _pauseStartMs;
    _state = BREW_RUNNING;
    Serial.println("[ColdDrip] 萃取已繼續");
}

void ColdDripEngine::setDripInterval(int seconds) {
    if (seconds < 10) seconds = 10;
    if (seconds > 300) seconds = 300;
    _dripIntervalSec = seconds;
    _totalCycles = (TARGET_TIME_HOURS * 3600) / _dripIntervalSec;
    Serial.printf("[ColdDrip] 滴灌間隔更新: %ds\n", _dripIntervalSec);
}

void ColdDripEngine::setPumpDuration(int milliseconds) {
    if (milliseconds < 500) milliseconds = 500;
    if (milliseconds > 5000) milliseconds = 5000;
    _pumpDurationMs = milliseconds;
    Serial.printf("[ColdDrip] 幫浦持續時間更新: %dms\n", _pumpDurationMs);
}

BrewStatus ColdDripEngine::getStatus() {
    BrewStatus s;
    s.state = _state;
    s.batteryVoltage = _batteryVoltage;
    s.batteryPercent = _batteryPercent;
    s.pumpActive = _pumpOn;
    s.cycleCount = _cycleCount;
    s.totalCycles = _totalCycles;
    s.dripIntervalSec = _dripIntervalSec;
    s.pumpDurationMs = _pumpDurationMs;
    
    s.totalMs = (unsigned long)TARGET_TIME_HOURS * 3600UL * 1000UL;
    
    if (_state == BREW_RUNNING || _state == BREW_PAUSED || _state == BREW_COMPLETE) {
        unsigned long now = millis();
        unsigned long pauseOffset = _totalPausedMs;
        if (_state == BREW_PAUSED) {
            pauseOffset += now - _pauseStartMs;
        }
        s.elapsedMs = now - _brewStartMs - pauseOffset;
    } else {
        s.elapsedMs = 0;
    }
    
    s.estimatedMl = _cycleCount * FLOW_RATE_ML_PER_CYCLE;
    
    return s;
}

float ColdDripEngine::getBatteryVoltage() {
    return _batteryVoltage;
}

int ColdDripEngine::getBatteryPercent() {
    return _batteryPercent;
}

void ColdDripEngine::_readBattery() {
    int raw = analogRead(BATTERY_ADC_PIN);
    float voltage = _rawAdcToVoltage(raw);
    
    // 滑動平均
    _batteryReadings[_batteryReadIndex] = voltage;
    _batteryReadIndex = (_batteryReadIndex + 1) % 8;
    
    float sum = 0;
    for (int i = 0; i < 8; i++) sum += _batteryReadings[i];
    _batteryVoltage = sum / 8.0f;
    
    // 計算百分比 (線性映射)
    if (_batteryVoltage >= BATTERY_FULL_V) {
        _batteryPercent = 100;
    } else if (_batteryVoltage <= BATTERY_CUTOFF_V) {
        _batteryPercent = 0;
    } else {
        _batteryPercent = (int)((_batteryVoltage - BATTERY_CUTOFF_V) / (BATTERY_FULL_V - BATTERY_CUTOFF_V) * 100.0f);
    }
}

void ColdDripEngine::_pumpOn_() {
    digitalWrite(PUMP_PIN, HIGH);
    _pumpOn = true;
    Serial.println("[ColdDrip] 幫浦 ON");
}

void ColdDripEngine::_pumpOff() {
    digitalWrite(PUMP_PIN, LOW);
    _pumpOn = false;
    Serial.println("[ColdDrip] 幫浦 OFF");
}

void ColdDripEngine::_checkSafety() {
    if (_batteryVoltage > 0 && _batteryVoltage < BATTERY_CUTOFF_V) {
        if (_state == BREW_RUNNING || _state == BREW_PAUSED) {
            _pumpOff();
            _state = BREW_LOW_BATTERY;
            Serial.printf("[ColdDrip] ⚠️ 低電量停機保護! 電壓: %.2fV\n", _batteryVoltage);
        }
    }
}

float ColdDripEngine::_rawAdcToVoltage(int raw) {
    // ADC 讀取值 → 分壓前實際電壓
    float adcVoltage = (raw / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    float actualVoltage = adcVoltage * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2;
    return actualVoltage;
}

#endif
