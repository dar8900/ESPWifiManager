/**
 * @file udp.h
 * @author Home Microtech 
 * @brief  Modulo per la gestione dell'invio/ricezione messaggi via udp
 * @version 0.1
 * @date 2022-11-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef UDP_H
#define UDP_H

#include <Arduino.h>

#ifdef ESP32			
	#include <WiFi.h>
#else
#ifdef ESP8266
	#include <ESP8266WiFi.h>
#endif

#endif

#include <WiFiUdp.h>
#include <stdint.h> 

#define MAX_UPD_MSG_LEN     1024

class UdpTalker
{
    public:
	UdpTalker();
	int8_t init(const char *LocalIp, const char *MasterIp, uint16_t MasterPort);
	int8_t init(IPAddress LocalIp, IPAddress MasterIp, uint16_t MasterPort);
	int8_t sendMessage(String MsgSnd);
	int8_t receiveMessage(String &MsgRcv);
	void stop();

    private:
	bool _isSettingUp = false;
	WiFiUDP *_udpManager;
	IPAddress _masterIpAddress;
	uint16_t _masterPort;
	uint8_t *_msgBuffer;
	void _clearBuffer();
};


#endif