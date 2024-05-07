#pragma once
#if defined(ESP8266)
#define SYS_TIMESTAMPFUNC 
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#define SYS_TIMESTAMPFUNC esp_log_system_timestamp(),
#endif

#ifdef USE_LOG_COLORS
#define LOG_COLOR_ERR  "\033[0;31m" // red
#define LOG_COLOR_WRN  "\033[0;33m" // yellow
#define LOG_COLOR_DBG  "\033[0;36m" // cyan
#define LOG_NO_COLOR   "\033[0m"
#else
#define LOG_COLOR_ERR
#define LOG_COLOR_WRN
#define LOG_COLOR_DBG
#define LOG_NO_COLOR
#endif 

#ifndef dbgVerbose
#define dbgVerbose 0
#endif

#define INF_FORMAT(format) "[%s %s] " format "\n", esp_log_system_timestamp(), __FUNCTION__
#define LOG_INF(format, ...) Serial.printf(INF_FORMAT(format), ##__VA_ARGS__)
#define ERR_FORMAT(format) LOG_COLOR_ERR "[%s ERROR @ %s:%u] " format LOG_NO_COLOR "\n", SYS_TIMESTAMPFUNC pathToFileName(__FILE__), __LINE__
#define LOG_ERR(format, ...) Serial.printf(ERR_FORMAT(format "~"), ##__VA_ARGS__)
#define LOG_ALT(format, ...) Serial.printf(INF_FORMAT(format "~"), ##__VA_ARGS__)
#define WRN_FORMAT(format) LOG_COLOR_WRN "[%s WARN %s] " format LOG_NO_COLOR "\n", SYS_TIMESTAMPFUNC __FUNCTION__
#define LOG_WRN(format, ...) Serial.printf(WRN_FORMAT(format "~"), ##__VA_ARGS__)
#define DBG_FORMAT(format) LOG_COLOR_DBG "[%s DEBUG @ %s %s:%u] " format LOG_NO_COLOR "\n", SYS_TIMESTAMPFUNC pathToFileName(__FILE__), __FUNCTION__, __LINE__
#define LOG_DBG(format, ...) if (dbgVerbose) Serial.printf(DBG_FORMAT(format), ##__VA_ARGS__)