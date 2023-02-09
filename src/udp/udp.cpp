/**
 * @file udp.cpp
 * @author Home Microtech
 * @brief 
 * @version 0.1
 * @date 2022-11-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "udp.h"
#include "cstring"

void UdpTalker::_clearBuffer()
{
	memset(_msgBuffer, 0x00, MAX_UPD_MSG_LEN);
}


UdpTalker::UdpTalker()
{
	_udpManager = new WiFiUDP();
	_msgBuffer = new uint8_t[MAX_UPD_MSG_LEN];
	_masterPort = 0;
	_clearBuffer();
}

/**
 * @brief  Funzione che inizializza l'udp manager e inserisce i valori dell'ip address
 * 		 e della porta del master
 * 
 * @param char* MasterIpAddress 
 * @param uint16_t MasterPort 
 * @return int8_t Ret
 */
int8_t UdpTalker::init(const char *LocalIp, const char *MasterIp, uint16_t MasterPort)
{
	int8_t Ret = 0;
	IPAddress LocalIpAddr;
	if(LocalIp != NULL && MasterPort != 0 && MasterIp != NULL)
	{
		_masterPort = MasterPort;
		_masterIpAddress.fromString(MasterIp);
		if(LocalIpAddr.fromString(LocalIp))
		{
#ifdef ESP32			
			_udpManager->begin(LocalIpAddr, _masterPort);
#else
#ifdef ESP8266
			_udpManager->begin(_masterPort);
#endif

#endif
			_isSettingUp = true;
			Ret = 1;
		}
	}
	return Ret;
}

/**
 * @brief  Funzione che inizializza l'udp manager e inserisce i valori dell'ip address
 * 		 e della porta del master
 * 
 * @param IPAddress MasterIpAddress 
 * @param uint16_t MasterPort 
 * @return int8_t Ret
 */
int8_t UdpTalker::init(IPAddress LocalIp, IPAddress MasterIp, uint16_t MasterPort)
{
	int8_t Ret = 0;
	if(MasterPort != 0)
	{
		_masterPort = MasterPort;
		_masterIpAddress = MasterIp;
#ifdef ESP32			
		if(_udpManager->begin(LocalIp, _masterPort) > 0)
		{
			_isSettingUp = true;
			Ret = 1;
		}
#else
#ifdef ESP8266
		if(_udpManager->begin(_masterPort) > 0)
		{
			_isSettingUp = true;
			Ret = 1;
		}
#endif

#endif		
	}
	return Ret;
}

/**
 * @brief  Funzione per l'invio dei messaggi via udp
 * 		 Ritorna 1 se tutto ok
 * 		 Ritorna da -3 a -1 se ci sono problemi con l'invio
 * 		 Ritorna 0 se il messaggio supera MAX_UPD_MSG_LEN
 * 
 * @param String MsgSnd 
 * @return int8_t Ret
 */
int8_t UdpTalker::sendMessage(String MsgSnd)
{
	int8_t Ret = 0;
	uint32_t StrLen = MsgSnd.length();
	int32_t ByteWritten = 0;
	if(_isSettingUp)
	{
		if(StrLen < MAX_UPD_MSG_LEN)
		{
			MsgSnd.toCharArray((char *)_msgBuffer, StrLen);
			if(_udpManager->beginPacket(_masterIpAddress, _masterPort) > 0)
			{
				ByteWritten = _udpManager->write(_msgBuffer, StrLen);
				if(ByteWritten > 0 && ByteWritten == StrLen)
				{
					if(_udpManager->endPacket() > 0)
					{
						_clearBuffer();
						Ret = 1;
					}
					else
					{
						Ret = -3;
					}
				}
				else
				{
					Ret = -2;
				}
			}
			else
			{
				Ret = -1;
			}
		}
	}
	return Ret;
}

/**
 * @brief  Funzione per la ricezione di messaggi via udp
 * 		 Ritorna 1 se tutto è andato a buon 
 * 		 Ritorna da -2 a -1 se ci sono stati problemi nei vari step
 * 		 Ritorna 0 se non ci sono messaggi
 * 
 * @param String &MsgRcv 
 * @return int8_t Ret
 */
int8_t UdpTalker::receiveMessage(String &MsgRcv)
{
	int8_t Ret = 0;
	if(_isSettingUp)
	{
		if(_udpManager->parsePacket() > 0)
		{
			if(_udpManager->read(_msgBuffer, MAX_UPD_MSG_LEN) > 0)
			{
				MsgRcv = String((char*)_msgBuffer);
				if(MsgRcv.length() > 0)
				{
					Ret = 1;
					_clearBuffer();
				}
				else
				{
					Ret = -2;
				}
			}
			else
			{
				Ret = -1;
			}
		}
		else
		{
			Ret = 0;
		}
	}
	return Ret;
}

/**
 * @brief Ferma il client udp 
 * 
 */
void UdpTalker::stop()
{
	_udpManager->stop();
}