/**
 * @file wifi.h
 * @author Home Microtech ()
 * @brief  Modulo per la gestione dell'interfaccia wifi
 * @version 0.1
 * @date 2022-11-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef WIFI_H
#define WIFI_H
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <stdint.h>
#include <ctime>

class Esp32Wifi
{
    private:

        typedef enum
        {
            WIFI_GET_NETWORK_TIME_STATE = 0,
            WIFI_GET_LOCAL_TIME_STATE,
            WIFI_CHECK_CONNECTION_STATE,
            WIFI_WAIT_FOR_CONNECTION_STATE,
            WIFI_RECONNECT_STATE,
            MAX_WIFI_STATES
        }wifi_run_states;

        const uint32_t _TIMESTAMP_DFLT = 1633039200;
        WiFiUDP *_ntpUDP;
        NTPClient *_timeClient; //(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
        const String NTP_SERVER = "ntp1.inrim.it";
        bool _wifiConnected = false;
        uint16_t _localHourShift = 3600; // shift di 1h rispetto all'ora di greenwitch
        uint16_t _timeRefreshFrq = 30000; // frequenza di rischiesta di orario (in ms)
        bool _legalHourIsSetted = false;
        IPAddress *_myIp;
        time_t _ntpTimeStamp = (time_t)_TIMESTAMP_DFLT; 
        String _hostname = "ESP32WifiManager";
        String _SSID = "";
        String _Passwd = "";
        uint32_t _ntpBackUpTimer;
        uint32_t _reconnectTimer = 0;
        wifi_run_states _wifiRunState = WIFI_CHECK_CONNECTION_STATE;
        
        bool _searchWifiSsid();
        void _connectToWifi();
        uint32_t _getTimestamp();
        String _getTimeFormatted();
        String _getDateFormatted();
        String _getWeekday();
        void _legalHourShift();

    public:

        typedef struct 
        {
            uint32_t timestamp;
            String timeFormatted;
            String dateFormatted;
            uint8_t dayHour;
            String weekDay;
        }TIME_VARS;

        Esp32Wifi();
        TIME_VARS currentTimeInfo;
        String getMyIp();
        void setHostname(String Hostname);
        void setWifiSSID(String SSID);
        void setWifiPasswd(String Passwd);
        bool wifiConnected();
        bool initWifi();
        void runWifi();
};

#endif