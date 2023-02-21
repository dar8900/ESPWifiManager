#include <wifi/wifi.h>

EspWifi Wifi(EspWifi::wifi_mode::WIFI_MODE_ACCESS_POINT, "", "", "EspWifiManager", "EspAP", "EspAP");

void setup()
{
	Wifi.initWifi();
}


void loop()
{
	Wifi.runWifi();
}