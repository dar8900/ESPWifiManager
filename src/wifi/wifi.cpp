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



const String WeekDays[7] = {"Domenica", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato"};

#if WIFI_MANAGER_LOG_ENABLE
static void WriteWifiDebugLog(String Msg)
{
    Serial.println(Msg);
}
#else
static void WriteWifiDebugLog(String Msg)
{}
#endif

Esp32Wifi::Esp32Wifi()
{
    _ntpUDP = new WiFiUDP();
    _timeClient = new NTPClient(*_ntpUDP, NTP_SERVER.c_str(), _localHourShift, _timeRefreshFrq);
    _ntpBackUpTimer = 0;
    _myIp = IPAddress(0, 0, 0, 0);
}

/**
 * @brief Scansiona le reti wifi per cercare l'ssid impostato
 * 
 * @return SsidFoud 
 */
bool Esp32Wifi::_searchWifiSsid()
{
    const uint32_t SCAN_PERIOD = 10000;
    uint32_t ScanTime = 0;
    bool SsidFoud = false;
    WriteWifiDebugLog("Ora scansiono le reti wifi");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks();

    // print out Wi-Fi network scan result uppon completion
    int N_SSID = WiFi.scanComplete();
    if(N_SSID >= 0 && _SSID != "" && _Passwd != "")
    {
        bool FoundWifi = false;
        WriteWifiDebugLog("Trovate " + String(N_SSID) + " reti");
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
        WriteWifiDebugLog("Impossibile eseguire la ricerca delle reti");
    }
    return SsidFoud;
}

/**
 * @brief Tenta la connessione al wifi dopo aver scansionato e cercato l'ssid impostato
 * 
 */
void Esp32Wifi::_connectToWifi()
{
    uint16_t Timeout = 150; // 15 sec
    if(_searchWifiSsid())
    {
        WiFi.setHostname(_hostname.c_str());
        WiFi.begin(_SSID.c_str(), _Passwd.c_str());
        WriteWifiDebugLog("Connessione in corso...");
        while(!_wifiConnected && Timeout > 0)
        {
            if(WiFi.status() != WL_CONNECTED)
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
            WriteWifiDebugLog("Connesso all rete " + _SSID);
        }
        else
        {
            WriteWifiDebugLog("Connessione non riuscita");
        }
    }
    else
    {
        WriteWifiDebugLog("Nessuna rete trovata con SSID \"" + _SSID + "\"");
    }
}

/**
 * @brief Restituisce il timestamp caricato dall'ntp
 * 
 * @return uint32_t ts
 */
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

/**
 * @brief Restituisce ora e minuti formattati in una stringa
 * 
 * @return String TimeStr
 */
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

/**
 * @brief Restituisce giorno mese e anno formattati in una stringa
 * 
 * @return String DateStr
 */
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

/**
 * @brief Restituisce il giorno della settimana in stringa
 * 
 * @return [String] Weekday 
 */
String Esp32Wifi::_getWeekday()
{
    std::tm *locTime = std::localtime(&_ntpTimeStamp);
    if(locTime->tm_wday >= 0 && locTime->tm_wday <= 6)
        return WeekDays[locTime->tm_wday];
    else
        return "UNKNOWN";
}

/**
 * @brief Inserisce uno shift dell'ora in caso di ora legale/solare
 * 
 */
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

/**
 * @brief Restituisce l'ip assegnato
 * 
 * @return [String] Ip 
 */
String Esp32Wifi::getMyIp()
{
    return _myIp.toString();
}

/**
 * @brief Imposta l'hostname del modulo
 * 
 * @param String Hostname 
 */
void Esp32Wifi::setHostname(String Hostname)
{
    if(Hostname.length() > 0)
    {
        _hostname = Hostname;
    }
}

/**
 * @brief Imposta a quale SSID ci si deve connettere
 * 
 * @param String SSID 
 */
void Esp32Wifi::setWifiSSID(String SSID)
{
    if(SSID != "" && SSID != _SSID)
    {
        _SSID = SSID;
    }
}

/**
 * @brief Imposta la password dell'SSID
 * 
 * @param String Passwd 
 */
void Esp32Wifi::setWifiPasswd(String Passwd)
{
    if(Passwd != "" && _Passwd != Passwd)
    {
        _Passwd = Passwd;
    }
}

/**
 * @brief Controllo della connessione al wifi
 * 
 * @return [bool] connected 
 */
bool Esp32Wifi::wifiConnected()
{
    return _wifiConnected;
}

/**
 * @brief Funzione di inizializzazione del manager, effettua la prima connessione all'SSID
 * 
 * @return [bool] Connected 
 */
bool Esp32Wifi::initWifi()
{
    _connectToWifi();
    delay(1000);
    if(_wifiConnected)
    {
        _timeClient->begin();
        _myIp = WiFi.localIP();
        WriteWifiDebugLog("Assegnato IP: " + _myIp.toString());
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

/**
 * @brief Macchina a stati che controlla e mantiene la connessione al wifi e 
 *          effettua l'update dell'NTP
 * 
 */
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
            WriteWifiDebugLog("Wifi disconnesso, riprovo la connessione");
        }
        else
        {
            if(getMyIp().compareTo("0.0.0.0") == 0)
            {
                _wifiRunState = WIFI_GET_LOCAL_TIME_STATE;
                _wifiConnected = false;
                WiFi.disconnect();
                WriteWifiDebugLog("Ottenuto IP invalido, mi disconnetto e riprovo la connessione");
            }
            else
            {
                _wifiConnected = true;
                _wifiRunState = WIFI_GET_NETWORK_TIME_STATE;
            }
        }
        break;
    case WIFI_GET_NETWORK_TIME_STATE:
        _ntpBackUpTimer = millis();
        _timeClient->update();
        currentTimeInfo.timestamp = _getTimestamp();
        currentTimeInfo.dateFormatted = _getDateFormatted();
        currentTimeInfo.timeFormatted = _getTimeFormatted();
        currentTimeInfo.weekDay = _getWeekday();
        _legalHourShift();
        _wifiRunState = WIFI_CHECK_CONNECTION_STATE;
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
                WriteWifiDebugLog("Tentata la riconnessione per 5s, riprovo...");
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
        if(_searchWifiSsid())
        {
            WiFi.begin(_SSID.c_str(), _Passwd.c_str());
        }
        _reconnectTimer = millis();
        _wifiRunState = WIFI_WAIT_FOR_CONNECTION_STATE;
        break;
    default:
        break;
    }
}