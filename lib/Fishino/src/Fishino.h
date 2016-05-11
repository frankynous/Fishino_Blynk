/*
  Fishino.h - Library for ESP8266 WiFi module
  Copyright (c) 2015 MicioMax.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  VERSION 1.0.0 - INITIAL VERSION
  VERSION 2.0.0 - 06/01/2016 - REWROTE SPI INTERFACE AND ERROR HANDLING
*/

/////////////////////////////////////////////////////////////////////////////////////
// NOTE :
// ALL driver functions returns a boolean to show that command has been
// accepted. If return value is TRUE, the command was OK and more data is following
// If return value is FALSE, no more data is following
// This was changed since 2.0.0 to simplify error handling
// This library is *NOT* compatible with firmware prior to 2.0.0
// Next versions will be backwards compatible with previous firmware version
// *ONLY* if major version number is the same
/////////////////////////////////////////////////////////////////////////////////////

#ifndef FISHINO_H
#define FISHINO_H

#include "FishinoDebug.h"

// macro to build 32 bit version number from components
#define VERSION(maj, min, dev) ( ((uint32_t)maj << 16) | ((uint32_t)(min & 0xFF)) << 8 | ((uint32_t)(dev & 0xFF)) )


// minimal firmware version allowed for this library
#define FISHINO_FIRMWARE_VERSION_MIN VERSION(2,0,0)

#include "IPAddress.h"

#include "Stream.h"

// SPI handshake I/O pin
#define ESP8266_HANDSHAKE 7

// SPI CS I/O pin
#define ESP8266_CS 10

// the driver error codes
typedef enum
{
	ERROR_NONE				= 0,
	
	// Parameters errors
	ERROR_WRONG_PARAM_NUM,
	ERROR_INVALID_PARAM,
	
	// Function still not implemented
	ERROR_NOT_IMPLEMENTED,
	
	// Timeout errors
	ERROR_TIMEOUT,
	
	// WIFI station errors
	ERROR_WIFI_SCAN,
	ERROR_WIFI_BAD_MODE,
	ERROR_WIFI_NO_AP_FOUND,
	ERROR_WIFI_WRONG_PASSWORD,
	ERROR_WIFI_BAD_SSID,
	ERROR_WIFI_BAD_CHANNEL,
	ERROR_WIFI_BAD_HIDDEN_FLAG,
	ERROR_WIFI_CONNECTION_FAILED,
	ERROR_WIFI_TIMEOUT,
	
	// AP errors
	ERROR_AP_CONFIG_FAILED,
	ERROR_AP_CONFIG_BAD_MODE,
	ERROR_AP_CONFIG_BAD_DATA,
	
	// STATION errors
	ERROR_STATION_CONFIG_FAILED,
	ERROR_STATION_CONFIG_BAD_MODE,
	ERROR_STATION_CONFIG_BAD_DATA,

	// Socket errors
	ERROR_SOCKET_INVALID,
	ERROR_SOCKET_CLOSED,
	ERROR_SOCKET_OPENED,
	ERROR_NO_SOCKETS_AVAILABLE,
	
	// DHCP errors
	ERROR_DHCP,
	
	// MAC errors
	ERROR_MAC,
	
	// IP errors
	ERROR_IP,
	
	// PHY mode errors
	ERROR_BAD_PHY_MODE,
	
	// Server errors
	ERROR_SERVER_ALREADY,
	ERROR_SERVER_BAD_DATA,
	ERROR_SERVER_OFF,
	ERROR_SERVER_TIMEOUT,
	
	// Connection errors
	ERROR_CONNECTION_BAD_DATA,
	ERROR_CONNECTION_BAD_SOCKET,
	ERROR_CONNECTION_DNSFAIL,
	ERROR_CONNECTION_BAD_TYPE,
	ERROR_CONNECTION_TIMEOUT,
	ERROR_CONNECTION_GENERIC,
	
	// Disconnection errors
	ERROR_DISCONNECTION_BAD_DATA,
	ERROR_DISCONNECTION_BAD_SOCKET,
	ERROR_DISCONNECTION_TIMEOUT,
	ERROR_DISCONNECTION_GENERIC,
	
	// Send error
	ERROR_SEND_BAD_DATA,
	ERROR_SEND_BAD_SOCKET,
	ERROR_SEND_TIMEOUT,
	ERROR_SEND_TOO_BIG,
	ERROR_SEND_GENERIC,
	
	// Receive error
	ERROR_RECEIVE_BAD_DATA,
	ERROR_RECEIVE_BAD_SOCKET,
	ERROR_RECEIVE_TIMEOUT,
	ERROR_RECEIVE_OUT_OF_MEMORY,
	ERROR_RECEIVE_GENERIC,
	ERROR_RECEIVE_NO_DATA,
	
	// UART errors
	ERROR_UART_BAD_SPEED,
	ERROR_UART_BAD_PARITY,
	ERROR_UART_BAD_BITS,
	ERROR_UART_BAD_STOP_BITS,
	
	// PORTS ERROR
	ERROR_PORT_INVALID,
	ERROR_PORTMODE_INVALID,
	
	// UDP ERRORS
	ERROR_UDP_CONNECTION_BAD_DATA,
	ERROR_UDP_CONNECTION_BAD_SOCKET,
	ERROR_UDP_CONNECTION_DNSFAIL,
	ERROR_UDP_CONNECTION_GENERIC,
	
	ERROR_UDP_RECEIVE_BAD_DATA,
	ERROR_UDP_RECEIVE_BAD_SOCKET,
	ERROR_UDP_RECEIVE_TIMEOUT,
	ERROR_UDP_RECEIVE_OUT_OF_MEMORY,
	ERROR_UDP_RECEIVE_GENERIC,
	ERROR_UDP_RECEIVE_NO_PACKET_PARSED,
	ERROR_UDP_RECEIVE_NO_DATA,

	ERROR_UDP_SEND_BAD_DATA,
	ERROR_UDP_SEND_BAD_SOCKET,
	ERROR_UDP_SEND_TIMEOUT,
	ERROR_UDP_SEND_TOO_BIG,
	ERROR_UDP_SEND_GENERIC,
	ERROR_UDP_SEND_NO_DATA,
	
	ERROR_UNKNOWN,
	ERROR_MAX
	
} ErrorCodes;

typedef enum
{
	NULL_MODE,
	STATION_MODE,
	SOFTAP_MODE,
	STATIONAP_MODE

} WIFI_MODE;

typedef enum
{
    AUTH_OPEN           = 0,
    AUTH_WEP,
    AUTH_WPA_PSK,
    AUTH_WPA2_PSK,
    AUTH_WPA_WPA2_PSK,
    AUTH_MAX
    
} AUTH_MODE;

typedef enum
{
    STATION_IDLE = 0,
    STATION_CONNECTING,
    STATION_WRONG_PASSWORD,
    STATION_NO_AP_FOUND,
    STATION_CONNECT_FAIL,
    STATION_GOT_IP,
    STATION_TIMEOUT,
    STATION_BAD_SSID,

} JOIN_STATUS;

struct APINFO
{
	uint8_t authmode;
	char *ssid;
	int8_t rssi;
	uint8_t bssid[6];
	uint8_t channel;
};

typedef enum _SOCK_STATUS
{
	SOCK_INVALID,
	SOCK_DISCONNECTED,
	SOCK_CONNECTED,

} SOCK_STATUS;

typedef enum _PHY_MODE
{
	PHY_MODE_UNKNOWN	= 0,
	PHY_MODE_11B		= 1,
	PHY_MODE_11G		= 2,
	PHY_MODE_11N		= 3
} PHY_MODE;

class FishinoClass
{
	private:
		
		// firmware version
		uint32_t _fwVersion;
		
		// SSID of joined AP, if any
		char *_ssid;

		// scanned access point info
		APINFO *_apInfo;
		uint8_t _numAp;
		
		// crc of sent data bytes
		// (Fletcher CRC of 24 bit size, modulo 4093)
		uint16_t crc1, crc2;
		
		// frees APINFO array
		void freeApInfo(void);
		
		// find an APINFO given SSID
		APINFO const *findApInfo(const char *ssid);
		
		// low-level i/o routines
		bool _spiSel;
		void spiSelect(bool s);
		
		// wait for busy handshake, or 1 msecond
		// which comes first
		void spiWaitBusy(void);

		// check if ESP is ready
		bool isSpiReady(void);
		
		// wait up ESP is ready, timeout in milliseconds
		bool spiWaitReady(uint16_t tOut);
		
		// start a command
		// cancels any previous pending command in WiFi module
		void spiStartCommand(uint8_t cmd);
		
		// end a command, checking transmitted data CRC
		// returns true if OK, false if bad crc
		bool spiEndCommand(void);
		
		// send/receive from 1 to 64 bytes
		// beware, ESP MUST know data length in advance
		bool spiSend(uint8_t const *b, uint8_t size);
		bool spiSend(const __FlashStringHelper *b, uint8_t size);
		bool spiReceive(uint8_t *b, uint8_t size);
		
		// 6 bytes is the minimum transfer unit used
		struct _SpiMinBuf
		{
			uint16_t siz;
			union
			{
				uint32_t dw;
				uint8_t d[4];
			};
		} spiMinBuf;

		// send the minimum 6 bytes data buffer unit
		bool spiSendMinBuf(void);

		// drops data from server (like spiReceive() but don't save it
		bool spiDrop(uint8_t size);
		
		// receive following data size
		bool spiReceiveSize(uint16_t &w);
		
		// send/receive data bytes
		bool spiSend8(uint8_t b);
		bool spiReceive8(uint8_t &b);
		bool spiSend16(uint16_t w);
		bool spiReceive16(uint16_t &w);
		bool spiSend32(uint32_t dw);
		bool spiReceive32(uint32_t &dw);
		
		// send/receive a string
		// received string is malloc-ed, MUST be freed with free()
		bool spiSendString(const char *str);
		bool spiSendString(const __FlashStringHelper *str);
		bool spiReceiveString(char *&str);
		
		// send data by pointer and length
		bool spiSendData(uint8_t const *buf, uint16_t bufSize);
		
		// receive data, buffer allocated, must be freed by free()
		bool spiReceiveDataAlloc(uint8_t *&buf, uint16_t &bufSize);
		
		// receive data, buffer with enough space MUST be given
		// bufSize MUST be initialized with buffer size
		// bufSize is replaced with actual data size on success
		// on error or buffer overflow, bufSize is set to 0
		bool spiReceiveDataFixed(uint8_t *buf, uint16_t &bufSize);

		// low level TCP stuffs -- used by client and server
		
		// connect to host -- return socket number, or 0xff if error
		uint8_t connect(const char *host, uint16_t port);
		
		// connect to a secure host (HTTPS - SSL)
		uint8_t secureConnect(const char *host, uint16_t port);
		
		// disconnect from host -- return true on success, false otherwise
		bool disconnect(uint8_t sock);
		
		// write to socket -- return true on success, false otherwise
		bool write(uint8_t sock, uint8_t const *buf, uint16_t bufLen);
		
		// read from socket -- return true on success, false otherwise
		// buffer MUST be big enough to contain reqBytes
		// on exit, bufLen contains number of bytes actually read
		bool read(uint8_t sock, uint16_t reqBytes, uint8_t *buf, uint16_t &bufLen);

		// return number of available bytes on socket
		uint16_t available(uint8_t sock);

		// peek a data byte from socket, if any
		// returns true on success, false otherwise
		bool peek(uint8_t sock, uint8_t &b);
		
		// flush socket data
		bool flush(uint8_t sock);
		
		// connection status - true if connected, false if disconnected or invalid
		bool status(uint8_t sock);

		// start a server on given port
		// ONLY ONE SERVER AT A TIME
		bool startServer(uint32_t port, uint16_t timeout);
		
		// stops server
		bool stopServer(void);
		
		// poll server for sockets with data
		// returns a dynamic allocated buffer which MUST be freed by free
		bool pollServer(bool all, uint8_t *&buf, uint16_t &count);
		
		// binds UDP socket to IP/Port
		// returns udp socket number
		uint8_t udpBegin(uint32_t localPort);

		// ends UDP socked binding
		bool udpEnd(uint8_t sock);
		
		// starts building a packet for sending to a receiver
		bool udpBeginPacket(uint8_t sock, const char *host, uint32_t localPort);
		bool udpBeginPacket(uint8_t sock, const __FlashStringHelper *host, uint32_t localPort);
		bool udpBeginPacket(uint8_t sock, const IPAddress &ip, uint32_t localPort);
		
		// ends and send current packet
		bool udpEndPacket(uint8_t sock);
		
		// parse currently received UDP packet and returns number of available bytes
		uint16_t udpParsePacket(uint8_t sock);
		
		// write data to current packet
		bool udpWrite(uint8_t sock, const uint8_t *buf, uint16_t len);
		
		// check if data is available in current packet
		uint16_t udpAvail(uint8_t sock);
		
		// read data from current packet
		// buffer MUST be big enough to contain reqBytes
		// on exit, bufLen contains number of bytes actually read
		bool udpRead(uint8_t sock, uint16_t reqBytes, uint8_t *buf, uint16_t &bufLen);
		
		// peek next byte in current packet
		bool udpPeek(uint8_t sock, uint8_t &b);
		
		// Finish reading the current packet
		bool udpFlush(uint8_t sock);
		
		// flush all stored UDP packets
		bool udpFlushAll(uint8_t sock);
		
		// Return the IP address of the host who sent the current incoming packet
		// PARAMETERS:
		//		socket		byte
		// RESULT:
		//		remote IP	IP
		bool udpRemoteIP(uint8_t sock, IPAddress &ip);
		
		// Return the port of the host who sent the current incoming packet
		// PARAMETERS:
		//		socket		byte
		// RESULT:
		//		remote port	uint32
		bool udpRemotePort(uint8_t sock, uint32_t &port);
		
	public:

		// constructor
		FishinoClass();
		
		// destructor
		~FishinoClass();
		
		// reset ESP and wait for it to be ready
		// return true if ok, false if not ready
		bool reset(void);
		
		// Get firmware version
		uint32_t firmwareVersion();
		char *firmwareVersionStr();
		
		// error handling; useful mostly on debugging code
		// forward the request to ESP8266 module
		
		// get last driver error code
		uint16_t getLastError(void);
		
		// get last driver error string
		// MUST be freed by free() upon usage
		const char *getLastErrorString(void);
		
		// get string for error code
		// MUST be freed by free() upon usage
		const char *getErrorString(uint16_t errCode);
		
		// clear last error
		void clearLastError(void);
		
		// set operation mode (one of WIFI_MODE)
		bool setMode(uint8_t mode);
		
		// get operation mode
		uint8_t getMode(void);

		// Start Wifi connection for OPEN networks
		// param ssid: Pointer to the SSID string.
		// return true on success
		uint8_t begin(const char *ssid);
		uint8_t begin(const __FlashStringHelper *ssid);
		
		// Start Wifi connection with encryption.
		// return true on success
		uint8_t begin(const char* ssid, const char *passphrase);
		uint8_t begin(const __FlashStringHelper *ssid, const __FlashStringHelper *passphrase);

		// setup station AP parameters without joining
		bool setStaConfig(const char *ssid);
		bool setStaConfig(const __FlashStringHelper *ssid);
		bool setStaConfig(const char *ssid, const char *passphrase);
		bool setStaConfig(const __FlashStringHelper *ssid, const __FlashStringHelper *passphrase);
		
		// get current station AP parameters
		// WARNING : returns dynamic buffers wich MUST be freed
		bool getStaConfig(char *&ssid, char *&pass);
		
		// join AP setup by setStaConfig call (or previous AP set by begin)
		bool joinAp(void);
		
		// quits AP
		bool quitAp(void);

		// Change Ip configuration settings disabling the dhcp client
		//	param local_ip
		// WARNING : you should also set gateway and netmask (see next function)
		// if you set only the IP, the system will default with :
		// IP      : a, b, c, d
		// gateway : a, b, c, 1
		// netmask : 255, 255, 255, 0
		// (which is OK for most but not all situations)
		bool config(IPAddress local_ip);

		// Change Ip configuration settings disabling the dhcp client
		// param local_ip:			Static ip configuration
		// param gateway :			Static gateway configuration
		// param subnet:			Static Subnet mask
		// WARNING : you should also set netmask (see next function)
		// if you set only the IP and gateway, the system will default with :
		// IP      : a, b, c, d
		// gateway : (your gateway)
		// netmask : 255, 255, 255, 0
		// (which is OK for most but not all situations)
		bool config(IPAddress local_ip, IPAddress gateway);

		// Change Ip configuration settings disabling the dhcp client
		// param local_ip:			Static ip configuration
		// param gateway :			Static gateway configuration
		// param subnet:			Static Subnet mask
		bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet);

		// Change Ip configuration settings disabling the dhcp client
		// param local_ip:			Static ip configuration
		// param gateway:			Static gateway configuration
		// param subnet:			Static Subnet mask
		// param dns_server:		IP configuration for DNS server 1
		// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
		bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns_server);

		// Change DNS Ip configuration
		// param dns_server1:		ip configuration for DNS server 1
		// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
		bool setDNS(IPAddress dns_server1);

		// Change DNS Ip configuration
		// param dns_server1:		ip configuration for DNS server 1
		// param dns_server2:		ip configuration for DNS server 2
		// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
		bool setDNS(IPAddress dns_server1, IPAddress dns_server2);

		// Disconnect from the network
		// return: one value of wl_status_t enum
		bool disconnect(void);

		// Get the interface MAC address.
		// return: pointer to A STATIC uint8_t array with length WL_MAC_ADDR_LENGTH
		const uint8_t* macAddress(void);

		// Get the interface IP address.
		// return: Ip address value
		IPAddress localIP();

		// Get the interface subnet mask address.
		// return: subnet mask address value
		IPAddress subnetMask();

		// Get the gateway ip address.
		// return: gateway ip address value
		IPAddress gatewayIP();

		// Return the current SSID associated with the network
		// return: ssid string
		const char* SSID();

		// Return the current BSSID associated with the network.
		// It is the MAC address of the Access Point
		// return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
		const uint8_t* BSSID();

		// Return the current RSSI /Received Signal Strength in dBm)
		// associated with the network
		// return: signed value
		int32_t RSSI();

		// Return the Encryption Type associated with the network
		// return: one value of wl_enc_type enum
		uint8_t	encryptionType();

		// Start scan WiFi networks available
		// return: Number of discovered networks
		uint8_t scanNetworks();

		// Return the SSID discovered during the network scan.
		// param networkItem: specify from which network item want to get the information
		// return: ssid string of the specified item on the networks scanned list
		const char* SSID(uint8_t networkItem);

		// Return the encryption type of the networks discovered during the scanNetworks
		// param networkItem: specify from which network item want to get the information
		// return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
		uint8_t encryptionType(uint8_t networkItem);

		// Return the RSSI of the networks discovered during the scanNetworks
		// param networkItem: specify from which network item want to get the information
		// return: signed value of RSSI of the specified item on the networks scanned list
		int32_t RSSI(uint8_t networkItem);

		// Return Connection status.
		// return: true if connected to AP, false otherwise
		// if connected, refresh cached AP SSID
		uint8_t status();
		
		// "uniform" functions to set station and AP parameters
		// station ones replicates the above ones, jus to have meaningful names
		
		bool setStaIP(IPAddress ip);
		bool setStaMAC(uint8_t const *mac);
		bool setStaGateway(IPAddress gw);
		bool setStaNetMask(IPAddress nm);
		
		// station DHCP client
		bool staStartDHCP(void);
		bool staStopDHCP(void);
		bool getStaDHCPStatus(void);

		bool setApIP(IPAddress ip);
		bool setApMAC(uint8_t const *mac);
		bool setApGateway(IPAddress gw);
		bool setApNetMask(IPAddress nm);

		bool setApIPInfo(IPAddress ip, IPAddress gateway, IPAddress netmask);
		bool getApIPInfo(IPAddress &ip, IPAddress &gateway, IPAddress &netmask);

		// Resolve the given hostname to an IP address.
		// param aHostname: Name to be resolved
		// param aResult: IPAddress structure to store the returned IP address
		// result: 1 if aIPAddrString was successfully converted to an IP address,
		// else error code
		bool hostByName(const char* aHostname, IPAddress& aResult);

		// softAp DHCP
		bool softApStartDHCPServer(void);
		bool softApStartDHCPServer(IPAddress startIP, IPAddress endIP);
		bool softApStopDHCPServer(void);
		bool getSoftApDHCPServerStatus(void);

		// softAp configuration
		
		// get ap SSID - return a dynamic buffer that MUST be freed by free()
		// return NULL on error
		char *softApGetSSID(void);
		
		// get ap PASSWORD - return a dynamic buffer that MUST be freed by free()
		// return NULL on error
		char *softApGetPassword(void);
		
		// return ap channel
		// return 0 on error
		uint8_t softApGetChannel(void);
		
		// return ap hidden state
		bool softApGetHidden(void);
		
		// get all softAp config data
		// warning - all returned strings MUST be freed by free()
		bool softApGetConfig(char *&SSID, char *&pass, uint8_t &channel, bool &hidden);
		
		// set softAp parameters
		bool softApConfig(const char *SSID, const char *pass, uint8_t channel, bool hidden = false);
		bool softApConfig(const __FlashStringHelper *SSID, const __FlashStringHelper *passphrase, uint8_t channel, bool hidden = false);

		friend class FishinoClient;
		friend class FishinoSecureClient;
		friend class FishinoServer;
		friend class FishinoUDP;

		// extra I/O pins on ESP module

		// pinMode for ESP I/O
		bool pinMode(uint8_t pin, uint8_t mode);

		// digital I/O for ESP I/O
		uint8_t digitalRead(uint8_t pin);
		bool digitalWrite(uint8_t pin, uint8_t val);
		
		// analog read
		uint16_t analogRead(void);
		
		// physical mode handling
		uint8_t getPhyMode(void);
		bool setPhyMode(uint8_t mode);
		
		// TEST
		void test(void);
		
};

extern FishinoClass Fishino;

#include "FishinoClient.h"
#include "FishinoServer.h"
#include "FishinoUdp.h"

#endif
