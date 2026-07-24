#pragma once
// ============================================================================
// DeskBuddy — Time Service (NTP)
// ============================================================================
#include <Arduino.h>
#include "storage.h"

class TimeService {
public:
    void begin(Storage* storage);
    void update(); // Handle sync if needed
    
    // Check if time is currently synced
    bool isSynced() const { return _isSynced; }
    
    // Get time components
    int getHour() const;
    int getMinute() const;
    int getSecond() const;
    int getDay() const;
    int getMonth() const;
    int getYear() const;
    
    // Formatted strings
    String getFormattedTime(bool format12h = false, bool showSeconds = false) const;
    String getFormattedDate() const;
    
    // Trigger manual sync (blocking if wifi connected)
    bool syncTime();

private:
    Storage* _storage = nullptr;
    bool _isSynced = false;
    unsigned long _lastSyncTime = 0;
};
