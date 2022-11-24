/**
 * @file wifi.cpp
 * @author Home Microtech ()
 * @brief  Modulo per la gestione dell'interfaccia wifi
 * @version 0.1
 * @date 2022-11-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "wifi.h"

#include "../debug/debug.h"


const String WeekDays[7] = {"Domenica", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato"};

Esp32Wifi::Esp32Wifi()
{
    _ntpUDP = new WiFiUDP();
    _timeClient = new NTPClient(*_ntpUDP, NTP_SERVER.c_str(), _localHourShift, _timeRefreshFrq);
    _ntpBackUpTimer = new Chrono(Chrono::MILLIS);
    _myIp = new IPAddress(0, 0, 0, 0);
}

bool Esp32Wifi::_searchWifiSsid()
{
    const uint32_t SCAN_PERIOD = 10000;
    uint32_t ScanTime = 0;
    bool SsidFoud = false;
    WriteDebugString("Ora scansiono le reti wifi", "_searchWifiSsId", DEBUG_LEVEL_VERBOSE);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks();

    // print out Wi-Fi network scan result uppon completion
    int N_SSID = WiFi.scanComplete();
    if(N_SSID >= 0 && _SSID != "" && _Passwd != "")
    {
        bool FoundWifi = false;
        WriteDebugString("Trovate " + DebugString(N_SSID) + " reti", "_searchWifiSsId", DEBUG_LEVEL_DEBUG);
        for(int j = 0; j < N_SSID; j++)
        {
            if(WiFi.SSID(j) == _SSID)
            {
                SsidFoud = true;
                break;
            }
        }
        WiFi.scanDelete();
    }
    else
    {
        WriteDebugString("Impossibile eseguire la ricerca delle reti", "_searchWifiSsId", DEBUG_LEVEL_ERROR);
    }
    return SsidFoud;
}

void Esp32Wifi::_connectToWifi()
{
    uint16_t Timeout = 150; // 15 sec
    if(_searchWifiSsid())
    {
        WiFi.setHostname(_hostname.c_str());
        WiFi.begin(_SSID.c_str(), _Passwd.c_str());
        WriteDebugString("Connessione in corso...", "_connectToWifi", DEBUG_LEVEL_DEBUG);
        while(!_wifiConnected)
        {
            if(WiFi.status() != WL_CONNECTED && Timeout > 0)
            {
                Timeout--;
                _wifiConnected = false;
            }
            else
            {
                _wifiConnected = true;
            }
            delay(100);
        }
        if(_wifiConnected)
        {
            WriteDebugString("Connesso all rete " + _SSID, "_connectToWifi", DEBUG_LEVEL_INFO);
        }
        else
        {
            WriteDebugString("Connessione non riuscita", "_connectToWifi", DEBUG_LEVEL_ERROR);
        }
    }
    else
    {
        WriteDebugString("Nessuna rete trovata con SSID \"" + _SSID + "\"", "_connectToWifi", DEBUG_LEVEL_ERROR);
    }
}

uint32_t Esp32Wifi::_getTimestamp()
{
    uint32_t ts = 0;
    if(_wifiConnected)
    {
        ts = _timeClient->getEpochTime();
        _ntpTimeStamp = (time_t)ts;
    }
    else
    {
        ts = (uint32_t)_ntpTimeStamp;
    }
    return ts;
}

String Esp32Wifi::_getTimeFormatted()
{
    uint8_t Hour = 0, Minute = 0;
    String TimeStr = "";
    std::tm *locTime = std::localtime(&_ntpTimeStamp);
    Hour = locTime->tm_hour;
    currentTimeInfo.dayHour = Hour;
    Minute = locTime->tm_min;
    TimeStr = (Hour < 10? "0" + String(Hour) : String(Hour)) + ":" + (Minute < 10? "0" + String(Minute) : String(Minute));
    return TimeStr;
}

String Esp32Wifi::_getDateFormatted()
{
    uint8_t Day = 0, Month = 0, Year = 0;
    String DateStr = "";
    std::tm *locTime = std::localtime(&_ntpTimeStamp);
    Year = (1900 + locTime->tm_year) % 100;
    Month = 1 + locTime->tm_mon;
    Day = locTime->tm_mday;
    DateStr = (Day < 10? "0" + String(Day) : String(Day)) + "/" + (Month < 10? "0" + String(Month) : String(Month)) + "/" + String(Year);
    return DateStr;
}


String Esp32Wifi::_getWeekday()
{
    std::tm *locTime = std::localtime(&_ntpTimeStamp);
    return WeekDays[locTime->tm_wday];
}

void Esp32Wifi::_legalHourShift()
{
    uint8_t WeekDay = 0, Month = 0, MonthDay = 0;
    bool LegalHourShift = false;
    std::tm *locTime = std::localtime(&_ntpTimeStamp);
    Month = 1 + locTime->tm_mon;
    WeekDay = locTime->tm_wday;
    MonthDay = locTime->tm_mday;
    if(Month >= 3 && Month <= 10)
    {
        if((Month == 3 && MonthDay >= 25 && WeekDay == 0)  || (Month > 3))
        {
            LegalHourShift = true;
        }
        if((Month == 10 && MonthDay >= 25 && WeekDay == 0)  || (Month > 10))
        {
            LegalHourShift = false;
        }
    }
    if(!_legalHourIsSetted && LegalHourShift)
    {
        _timeClient->setTimeOffset(7200);
        _legalHourIsSetted = true;
    }
    if(_legalHourIsSetted && !LegalHourShift)
    {
        _timeClient->setTimeOffset(3600);
        _legalHourIsSetted = false;
    }
}

String Esp32Wifi::getMyIp()
{
    return _myIp->toString();
}

void Esp32Wifi::setHostname(String Hostname)
{
    if(Hostname.length() > 0)
    {
        _hostname = Hostname;
    }
}

void Esp32Wifi::setWifiSSID(String SSID)
{
    if(SSID != "" && SSID != _SSID)
    {
        _SSID = SSID;
    }
}

void Esp32Wifi::setWifiPasswd(String Passwd)
{
    if(Passwd != "" && _Passwd != Passwd)
    {
        _Passwd = Passwd;
    }
}

bool Esp32Wifi::wifiConnected()
{
    return _wifiConnected;
}

bool Esp32Wifi::initWifi()
{
    _connectToWifi();
    delay(1000);
    if(_wifiConnected)
    {
        _timeClient->begin();
        *_myIp = WiFi.localIP();
        WriteDebugString("Assegnato IP: " + _myIp->toString(), "initWifi()", DEBUG_LEVEL_INFO);
    }
    else
    {
        WiFi.disconnect();
        currentTimeInfo.timestamp = 0;
        currentTimeInfo.dateFormatted = "--/--/--";
        currentTimeInfo.timeFormatted = "--:--";
        currentTimeInfo.weekDay = "--------";
    }
    return _wifiConnected;
}

void Esp32Wifi::runWifi()
{
    switch (_wifiRunState)
    {
    case WIFI_CHECK_CONNECTION_STATE:
        if(WiFi.status() != WL_CONNECTED)
        {
            _wifiRunState = WIFI_GET_LOCAL_TIME_STATE;
            _wifiConnected = false;
            WiFi.disconnect();
            WriteDebugString("Wifi disconnesso, riprovo la connessione", DEBUG_LEVEL_ERROR);
        }
        else
        {
            if(getMyIp().compareTo("0.0.0.0") == 0)
            {
                _wifiRunState = WIFI_GET_LOCAL_TIME_STATE;
                _wifiConnected = false;
                WiFi.disconnect();
                WriteDebugString("Ottenuto IP invalido, mi disconnetto e riprovo la connessione", DEBUG_LEVEL_ERROR);
            }
            else
            {
                _wifiConnected = true;
                _wifiRunState = WIFI_GET_NETWORK_TIME_STATE;
            }
        }
        break;
    case WIFI_GET_NETWORK_TIME_STATE:
        _wifiRunState = WIFI_CHECK_CONNECTION_STATE;
        _ntpBackUpTimer = millis();
        _timeClient->update();
        currentTimeInfo.timestamp = _getTimestamp();
        currentTimeInfo.dateFormatted = _getDateFormatted();
        currentTimeInfo.timeFormatted = _getTimeFormatted();
        currentTimeInfo.weekDay = _getWeekday();
        _legalHourShift();
        break;
    case WIFI_GET_LOCAL_TIME_STATE:
        _wifiRunState = WIFI_RECONNECT_STATE;
        if(millis() - _ntpBackUpTimer >= 1000)
        {
            _ntpBackUpTimer = millis();
            _ntpTimeStamp += (time_t)1;
        }        
        currentTimeInfo.timestamp = _getTimestamp();
        currentTimeInfo.dateFormatted = _getDateFormatted();
        currentTimeInfo.timeFormatted = _getTimeFormatted();
        currentTimeInfo.weekDay = _getWeekday();
        break;
    case WIFI_WAIT_FOR_CONNECTION_STATE:
        if(WiFi.status() != WL_CONNECTED)
        {
            if(millis() - _reconnectTimer > 5000 && WiFi.status() != WL_CONNECTED)
            {
                WriteDebugString("Tentata la riconnessione per 5s, riprovo...", "runWifi", DEBUG_LEVEL_ERROR);
                _wifiRunState = WIFI_CHECK_CONNECTION_STATE;
                _reconnectTimer = 0;
            }
        }
        else
        {
            _wifiRunState = WIFI_CHECK_CONNECTION_STATE;
        }
        break;
    case WIFI_RECONNECT_STATE:
        WiFi.reconnect();
        _reconnectTimer = millis();
        _wifiRunState = WIFI_WAIT_FOR_CONNECTION_STATE;
        break;
    default:
        break;
    }
}