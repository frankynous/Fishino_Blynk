// for in-program strings
#include "Flash.h"

#define FISHINO_MODULE "Fishino"
#include "Fishino.h"

#include "Arduino.h"
#include "SPI.h"

enum Command
{
	CMD_NOP,			// no-op command								0
	CMD_RESET,			// chip reset									1
	CMD_TEST,			// test - returns given strings parameters		2
	CMD_GETFWVERSION,	// get firmware version							3
	
	CMD_GETLASTERROR,	// get last error code							4
	CMD_GETLASTERRORSTR,// get last error message						5
	CMD_GETERRORSTR,	// get error message for a given code			6
	CMD_CLEARLASTERROR,	// clear last error code						7

	CMD_GETPHYMODE,		// get WiFi physical connection mode			8
	CMD_SETPHYMODE,		// set WiFi physical connection mode			9

	CMD_QUERYMODE,		// query chip mode (AP, STA or AP+STA)			10
	CMD_SETMODE,		// set chip mode								11
	CMD_LISTAP,			// get list of available AP						12
	CMD_JOINAP,			// join an access point							13
	CMD_QUITAP,			// quit access point							14
	CMD_JOINSTATUS,		// query status of wifi connection to AP		15

	CMD_SETSTACONFIG,	// sets station configuration data				16
	CMD_GETSTACONFIG,	// get station configuration data				17
	CMD_STACONNECT,		// connect station using stored parameters		18
	CMD_STADISCONNECT,	// disconnect station (duplicate of CMD_QUITAP)	19

	CMD_SETSTAIP,		// set station IP								20
	CMD_GETSTAIP,		// get station IP								21
	CMD_SETSTAGW,		// set station gateway							22
	CMD_GETSTAGW,		// get station gatway							23
	CMD_SETSTANM,		// set station Netmask							24
	CMD_GETSTANM,		// get station Netmask							25
	CMD_SETSTADNS,		// set station DNS								26
	CMD_GETSTADNS,		// get station DNS								27
	CMD_SETSTAMAC,		// set station MAC								28
	CMD_GETSTAMAC,		// get station MAC								29
	CMD_SETSTAIPINFO,	// set all station IP info(IP, GW, NM)			30
	CMD_GETSTAIPINFO,	// set all station IP info(IP, GW, NM)			31
	CMD_ENABLESTADHCP,	// enable station DHCP							32
	CMD_DISABLESTADHCP,	// disable station DHCP							33
	CMD_QUERYSTADHCP,	// queries status of station DHCP client		34

	CMD_APSETCONFIG,	// sets ap config (SSID, PASS, CHANNEL, HIDDEN)	35
	CMD_APGETCONFIG,	// gets ap config								36
	
	CMD_SETAPIP,		// set AP IP									37
	CMD_GETAPIP,		// get AP IP									38
	CMD_SETAPGW,		// set AP gateway								39
	CMD_GETAPGW,		// get AP gatway								40
	CMD_SETAPNM,		// set AP Netmask								41
	CMD_GETAPNM,		// get AP Netmask								42
	CMD_SETAPDNS,		// set AP DNS									43
	CMD_GETAPDNS,		// get AP DNS									44
	CMD_SETAPMAC,		// set AP MAC									45
	CMD_GETAPMAC,		// get AP MAC									46
	CMD_SETAPIPINFO,	// sets full AP IP info (IP, GW and NM)			47
	CMD_GETAPIPINFO,	// sets full AP IP info (IP, GW and NM)			48
	CMD_ENABLEAPDHCP,	// enable AP DHCP								49
	CMD_DISABLEAPDHCP,	// disable AP DHCP								50
	CMD_QUERYAPDHCP,	// queries status of AP DHCP server				51
	CMD_SETAPDHCPRANGE,	// sets AP DHCP range							52

	CMD_CONNSTATUS,		// get tcp connection status					53
	CMD_CONNECT,		// starts a TCP or UDP connection				54
	CMD_CLOSE,			// close connection								55
	CMD_SEND,			// send data to socket							56
	CMD_AVAIL,			// get number of available data on a socket		57
	CMD_READ,			// read data from socket						58
	CMD_PEEK,			// peeks a byte of data from socket, if any		59
	CMD_FLUSH,			// flushes socket data							60
	
	CMD_STARTSERVER,	// start server									61
	CMD_STOPSERVER,		// stop server									62
	CMD_POLLSERVER,		// poll server for sockets with data			63

	CMD_PINMODE,		// pinMode for extra I/O on ESP module			64
	CMD_DIGITALREAD,	// digital read for extra I/O on ESP module		65
	CMD_DIGITALWRITE,	// digital write for extra I/O on ESP module	66
	CMD_ANALOGREAD,		// analog read									67
	CMD_ANALOGWRITE,	// analog write									68
	
	CMD_SERIALBEGIN,	// starts built-in serial port					69
	CMD_SERIALEND,		// stops built-in serial port					70
	CMD_SERIALAVAIL,	// gets number of available bytes on port		71
	CMD_SERIALREAD,		// reads data from serial port					72
	CMD_SERIALWRITE,	// writes data to serial port					73

	CMD_UDPBEGIN,		// starts UDP listening on given port			74
	CMD_UDPEND,			// ends UDP listening on given socket			75
	CMD_UDPBEGINPACKET,	// starts an UDP packet							76
	CMD_UDPENDPACKET,	// ends and send an UDP packet					77
	CMD_UDPPARSEPACKET,	// parse a received UDP packet					78
	CMD_UDPWRITE,		// write data to current outgoing UDP packet	79
	CMD_UDPAVAIL,		// number of avail bytes on current UDP packet	80
	CMD_UDPREAD,		// read data from current incoming UDP packet	81
	CMD_UDPPEEK,		// peek a byte from current incoming UDP packet	82
	CMD_UDPFLUSH,		// flush data on current incoming UDP packet	83
	CMD_UDPFLUSHALL,	// flush all incoming UDP packets				84
	CMD_UDPREMOTEIP,	// get remote ip of current incoming UDP packet	85
	CMD_UDPREMOTEPORT,	// get remote port of current inc. UDP packet	86
};

// status commands sent by host
typedef enum
{
	STATUS_RESET			= 0xa5,
	STATUS_PRESENCE_TEST	= 1,
	STATUS_COMMAND_START	= 2,
	STATUS_COMMAND_END		= 3
	
} STATUS_COMMANDS;

// ESP SPI commands
typedef enum
{
	ESP_STATUS_READ			= 0x04,
	ESP_STATUS_WRITE		= 0X01,
	ESP_BUFFER_READ			= 0x03,
	ESP_BUFFER_WRITE		= 0x02
	
} ESP_SPI_COMMANDS;

// check word for presence test
#define PRESENCE_MAGIC	0x55aa8118

// milliseconds between presence test retries
#define PRESENCE_TEST_RETRY_TIME	300

// timeout for ESP presence test, milliseconds
#define PRESENCE_TEST_TIMEOUT		5000

// repeat command macro - modify if you want more or less retries
#define REPEAT_COMMAND for(uint8_t rep = 0; rep < 5; rep++)

// error messages
FLASH_STRING(_busy_err, "Fishino busy ");
FLASH_STRING(_send_err, "sending");
FLASH_STRING(_recv_err, "receiving");
FLASH_STRING(_byte_err, " byte");

// helper for flash strings
char *strdup_P(const __FlashStringHelper *f)
{
	char *res = (char *)malloc(strlen_P((PGM_P)f) + 1);
	if(!res)
		return 0;
	strcpy_P(res, (PGM_P)f);
	return res;
}

// frees APINFO array
void FishinoClass::freeApInfo(void)
{
	if(_apInfo)
	{
		for(int i = 0; i < _numAp; i++)
			if(_apInfo[i].ssid)
				free(_apInfo[i].ssid);
		free(_apInfo);
		_apInfo = NULL;
	}
	_numAp = 0;
}

// find an APINFO given SSID
APINFO const *FishinoClass::findApInfo(const char *ssid)
{
	if(!ssid)
		return NULL;
	for(int i = 0; i < _numAp; i++)
		if(!strcmp(ssid, _apInfo[i].ssid))
			return &_apInfo[i];
	return NULL;
}

// SPI settings for ESP
SPISettings spiSettings(8000000, MSBFIRST, SPI_MODE0);

// low-level i/o routines
void FishinoClass::spiSelect(bool s)
{
	if(s)
	{
		if(!_spiSel)
		{
			SPI.beginTransaction(spiSettings);
			::digitalWrite(ESP8266_CS, LOW);
		}
	}
	else
	{
		if(_spiSel)
		{
			::digitalWrite(ESP8266_CS, HIGH);
			// wait approx 255 cycles MAX (16 MHz x 3 instructions ~ 47 uSec
			// for busy signal, showing that ESP received the data
			// we don't use timers here, too slow
			// we must ensure that busy signal stand for at least 10-20 uSec on ESP side
			// otherwise we'll have problems....
/*
			uint8_t i;
			for(i = 0; i < 255 && !::digitalRead(ESP8266_HANDSHAKE); i++)
				;
			if(i == 255)
				Serial << F("STILL NOT BUSY\n");
*/
			if(!::digitalRead(ESP8266_HANDSHAKE))
			{
				uint32_t tim = millis() + 100;
				while(!::digitalRead(ESP8266_HANDSHAKE) && millis() < tim)
					;
				if(!::digitalRead(ESP8266_HANDSHAKE))
					Serial << F("STILL NOT BUSY\n");
			}
			SPI.endTransaction();
		}
	}
	_spiSel = s;
}

// check if ESP is ready
bool FishinoClass::isSpiReady(void)
{
	return !::digitalRead(ESP8266_HANDSHAKE);
}

// wait up ESP is ready, timeout in milliseconds
bool FishinoClass::spiWaitReady(uint16_t tOut)
{
	// fast path
	if(!::digitalRead(ESP8266_HANDSHAKE))
		return true;
	
	// slow path, waiting
	uint32_t t = millis() + tOut;
	while(::digitalRead(ESP8266_HANDSHAKE))
		if(millis() >= t)
			break;
/*
	if(::digitalRead(ESP8266_HANDSHAKE))
		Serial << "OOPS, still not ready\n";
*/
/*
	else
		Serial << "TOOK " << millis() - (t - tOut) << " ms to be ready\n";
*/
	return !::digitalRead(ESP8266_HANDSHAKE);
}


// start a command
// cancels any previous pending command in WiFi module
void FishinoClass::spiStartCommand(uint8_t cmd)
{
	// wait for ESP to be ready, or execute anyways if not
	// some milliseconds should be enough
	// we don't want to block too long on errors
	spiWaitReady(100);

	// resets crc
	crc1 = crc2 = cmd;
	
	// toggle select to cancel any pending transaction
	spiSelect(true);

	// send a status command, value 02 (command start)
	SPI.transfer(ESP_STATUS_WRITE);		// write status
	SPI.transfer(STATUS_COMMAND_START);	// 2 status code means command start
	SPI.transfer(0x00);					// high byte of command start
	SPI.transfer(cmd);					// the command
	SPI.transfer(0x00);					// null high command part

	spiSelect(false);
}

// end a command, checking transmitted data CRC
// returns true if OK, false if bad crc
bool FishinoClass::spiEndCommand(void)
{
	// wait for ESP to be ready, or execute anyways if not
	// some milliseconds should be enough
	// we don't want to block too long on errors
	spiWaitReady(100);

	// selects the module, to ask for command termination
	spiSelect(true);
	
	uint32_t crc = (((uint32_t)crc2) << 12) | crc1;
	
	// send a status command asking to end current command
	SPI.transfer(ESP_STATUS_WRITE);		// write status
	SPI.transfer(STATUS_COMMAND_END);	// 3 status code means command end
	SPI.transfer((uint8_t)crc);			// low CRC byte
	SPI.transfer((uint8_t)(crc >> 8));	// mid CRC byte
	SPI.transfer((uint8_t)(crc >> 16));	// high CRC byte

	spiSelect(false);

	// wait for ESP to be ready, or execute anyways if not
	// some milliseconds should be enough
	// we don't want to block too long on errors
	spiWaitReady(100);

	spiSelect(true);

	// issue a read status command to get command status byte
	SPI.transfer(ESP_STATUS_READ);	// read status
	bool res = SPI.transfer(0x00);	// get status byte

	// 3 dummy bytes to complete status read
	// without having to free spi
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);

	spiSelect(false);

	return res;
}


// send/receive a chunk of 1..64 bytes
// beware, ESP MUST know data size in advance
bool FishinoClass::spiSend(const uint8_t *b, uint8_t size)
{
	// wait for ESP to be ready, or timeout (2 sec should be enough)
	if(!spiWaitReady(5000))
	{
		Serial << _busy_err << _send_err << _byte_err << "\n";
		return false;
	}
	
	spiSelect(true);

	// send byte through spi
	SPI.transfer(ESP_BUFFER_WRITE);	// write command
	SPI.transfer(0xff);				// dummy address (sigh)
	for(uint8_t i = 0; i < size; i++)
	{
		crc1 += *b;
		crc1 %= 4093;
		crc2 += crc1;
		crc2 %= 4093;
		SPI.transfer(*b++);
	}
	
	spiSelect(false);

	return true;
}

bool FishinoClass::spiSend(const __FlashStringHelper *bf, uint8_t size)
{
	PGM_P b = (PGM_P)bf;
	
	// wait for ESP to be ready, or timeout (2 sec should be enough)
	if(!spiWaitReady(5000))
	{
		Serial << _busy_err << _send_err << _byte_err << "\n";
		return false;
	}
	
	spiSelect(true);

	// send byte through spi
	SPI.transfer(ESP_BUFFER_WRITE);	// write command
	SPI.transfer(0xff);				// dummy address (sigh)
	for(uint8_t i = 0; i < size; i++)
	{
		crc1 += pgm_read_byte(b);
		crc1 %= 4093;
		crc2 += crc1;
		crc2 %= 4093;
		SPI.transfer(pgm_read_byte(b++));
	}
	
	spiSelect(false);

	return true;
}


bool FishinoClass::spiReceive(uint8_t *b, uint8_t size)
{
	// read byte, timeout of 200 mSec
	if(!spiWaitReady(5000))
	{
		Serial << _busy_err << _recv_err << _byte_err << "\n";
		return false;
	}

	spiSelect(true);

	// send byte through spi
	SPI.transfer(ESP_BUFFER_READ);	// read command
	SPI.transfer(0x00);				// dummy address (sigh)

	for(uint8_t i = 0; i < size; i++)
		*b++ = SPI.transfer(0x00);		// receive byte

	spiSelect(false);

	return true;
}

// drops data from server (like spiReceive() but don't save it
bool FishinoClass::spiDrop(uint8_t size)
{
	// read byte, timeout of 200 mSec
	if(!spiWaitReady(5000))
	{
		Serial << _busy_err << _recv_err << _byte_err << "\n";
		return false;
	}

	spiSelect(true);

	// send byte through spi
	SPI.transfer(ESP_BUFFER_READ);	// read command
	SPI.transfer(0x00);				// dummy address (sigh)
	for(int i = 0; i < size; i++)
		SPI.transfer(0x00);		// receive byte and drop it

	spiSelect(false);

	return true;
}

// receive following data size
bool FishinoClass::spiReceiveSize(uint16_t &w)
{
	return spiReceive((uint8_t *)&w, 2);
}

// send the minimum 6 bytes data buffer unit
bool FishinoClass::spiSendMinBuf(void)
{
	// wait for ESP to be ready, or timeout (2 sec should be enough)
	if(!spiWaitReady(5000))
	{
		Serial << _busy_err << _send_err << _byte_err << "\n";
		return false;
	}
	
	spiSelect(true);

	uint16_t siz = min(spiMinBuf.siz + 2, 6);
	uint8_t *b = (uint8_t *)&spiMinBuf;

	// send byte through spi
	SPI.transfer(ESP_BUFFER_WRITE);	// write command
	SPI.transfer(0xff);				// dummy address (sigh)
	for(uint8_t i = 0; i < siz; i++)
	{
		crc1 += *b;
		crc1 %= 4093;
		crc2 += crc1;
		crc2 %= 4093;
		SPI.transfer(*b++);
	}
	for(uint8_t i = siz; i < 6; i++)
		SPI.transfer(0);
	
	spiSelect(false);

	return true;
}

// send/receive data bytes
bool FishinoClass::spiSend8(uint8_t b)
{
	spiMinBuf.siz = 1;
	spiMinBuf.dw = b;
	return spiSendMinBuf();
}

bool FishinoClass::spiReceive8(uint8_t &b)
{
	uint16_t w;
	if(!spiReceiveSize(w) || w != 1)
		return false;
	return spiReceive(&b, 1);
}

bool FishinoClass::spiSend16(uint16_t w)
{
	spiMinBuf.siz = 2;
	spiMinBuf.dw = w;
	return spiSendMinBuf();
}

bool FishinoClass::spiReceive16(uint16_t &w)
{
	uint16_t ws;
	if(!spiReceiveSize(ws) || ws != 2)
		return false;
	return spiReceive((uint8_t *)&w, 2);
}

bool FishinoClass::spiSend32(uint32_t dw)
{
	spiMinBuf.siz = 4;
	spiMinBuf.dw = dw;
	return spiSendMinBuf();
}

bool FishinoClass::spiReceive32(uint32_t &dw)
{
	uint16_t w;
	if(!spiReceiveSize(w) || w != 4)
		return false;
	return spiReceive((uint8_t *)&dw, 4);
}

// send/receive a string
// received string is malloc-ed, MUST be freed with free()
bool FishinoClass::spiSendString(const char *str)
{
	uint16_t w = strlen(str);
	
	// setup min buffer for size and first 4 bytes
	spiMinBuf.siz = w;
	spiMinBuf.dw = 0;
	for(uint8_t i = 0; i < 4 && w; i++)
	{
		spiMinBuf.d[i] = *str++;
		w--;
	}
	if(!spiSendMinBuf())
		return false;

	// then send data. in chunks of 64 bytes max
	while(w)
	{
		uint16_t siz = min(64, w);
		if(!spiSend((uint8_t *)str, siz))
			return false;
		str += siz;
		w -= siz;
	}

	return true;
}

bool FishinoClass::spiSendString(const __FlashStringHelper *str)
{
	PGM_P s = (PGM_P)str;
	uint16_t w = strlen_P(s);
	
	// setup min buffer for size and first 4 bytes
	spiMinBuf.siz = w;
	spiMinBuf.dw = 0;
	for(uint8_t i = 0; i < 4 && w; i++)
	{
		spiMinBuf.d[i] = pgm_read_byte(s++);
		w--;
	}
	if(!spiSendMinBuf())
		return false;

	// then send data. in chunks of 64 bytes max
	while(w)
	{
		uint16_t siz = min(64, w);
		if(!spiSend((__FlashStringHelper const *)s, siz))
			return false;
		s += siz;
		w -= siz;
	}

	return true;
}

bool FishinoClass::spiReceiveString(char *&str)
{
	uint16_t w = 0;
	if(!spiReceiveSize(w))
		return false;
	
	// special case for null string
	if(w == 0x0000)
	{
		str = NULL;
		return true;
	}

	// allocate string
	str = (char *)FISHINO_MALLOC((w + 1) * sizeof(char));
	if(!str)
		return false;
	uint8_t *strP = (uint8_t *)str;
	
	// receive it in chunks of max 64 bytes
	while(w)
	{
		uint8_t siz = min(64, w);
		if(!spiReceive(strP, siz))
		{
			free(str);
			str = NULL;
			return false;
		}
		strP += siz;
		w -= siz;
	}
	*strP = 0;
	return true;
}

// send data by pointer and length
bool FishinoClass::spiSendData(byte const *buf, word bufSize)
{
	// setup min buffer for size and first 4 bytes
	spiMinBuf.siz = bufSize;
	spiMinBuf.dw = 0;
	for(uint8_t i = 0; i < 4 && bufSize; i++)
	{
		spiMinBuf.d[i] = *buf++;
		bufSize--;
	}
	if(!spiSendMinBuf())
		return false;
	
	while(bufSize)
	{
		uint8_t siz = min(64, bufSize);
		if(!spiSend(buf, siz))
			return false;
		buf += siz;
		bufSize -= siz;
	}
	return true;
}

// receive data, buffer allocated, must be freed by free()
bool FishinoClass::spiReceiveDataAlloc(byte *&buf, word &bufSize)
{
	if(!spiReceiveSize(bufSize))
		return false;

	// special case for null string
	if(bufSize == 0x0000)
	{
		buf = NULL;
		return true;
	}
	
	// allocate string
	buf = (byte *)FISHINO_MALLOC((bufSize + 1) * sizeof(byte));
	if(!buf)
		return false;
	
	// receive it in chunks of max 64 bytes
	byte *bufP = buf;
	uint16_t w = bufSize;
	while(w)
	{
		uint8_t siz = min(64, w);
		if(!spiReceive(bufP, w))
		{
			free(buf);
			buf = NULL;
			return false;
		}
		bufP += siz;
		w -= siz;
	}
	return true;
}

// receive data, buffer with enough space MUST be given
// bufSize MUST be initialized with buffer size
// bufSize is replaced with actual data size on success
// on error or buffer overflow, bufSize is set to 0
bool FishinoClass::spiReceiveDataFixed(uint8_t *buf, uint16_t &bufSize)
{
	uint16_t maxSize = bufSize;
	bufSize = 0;

	if(!spiReceiveSize(bufSize))
	{
		bufSize = 0;
		return false;
	}

	// if available data is greater than given buffer size
	// simply discard ALL data and return false
	if(bufSize > maxSize)
	{
		uint16_t w = bufSize;
		while(w)
		{
			uint8_t siz = min(64, w);
			if(!spiDrop(siz))
				break;
			w -= siz;
		}

		bufSize = 0;
		return false;
	}
		
	if(bufSize)
	{
		uint8_t *p = buf;
		uint16_t w = bufSize;
		while(w)
		{
			uint8_t siz = min(64, w);
			if(!spiReceive(p, siz))
			{
				bufSize = 0;
				return false;
			}
			p += siz;
			w -= siz;
		}
	}
	return true;
}

// constructor
FishinoClass::FishinoClass()
{
	// initialize firmware version
	_fwVersion = -1;
	
	// initialize current AP SSID
	_ssid = NULL;
	
	// initialize scanned AP info
	_apInfo = NULL;
	_numAp = 0;

	// set the slaveSelectPin as an output:
	::digitalWrite(ESP8266_CS, HIGH);
	::pinMode(ESP8266_CS, OUTPUT);
	_spiSel = false;
	
	// set handshake pin as input
	::pinMode(ESP8266_HANDSHAKE, INPUT);
}

// destructor
FishinoClass::~FishinoClass()
{
	// frees current ssid, if any
	if(_ssid)
		free(_ssid);
	
	// frees apInfo, if any
	freeApInfo();
}

// reset ESP and wait for it to be ready
// return true if ok, false if not ready
bool FishinoClass::reset(void)
{
	// send a status command asking for reset
	spiSelect(true);
	delay(10);

	// send a status command, value 00 (reset chip)
	SPI.transfer(ESP_STATUS_WRITE);	// write status
	SPI.transfer(STATUS_RESET);		// 0 status code means reset
	
	delay(10);
	spiSelect(false);

	// give some time to settle
	delay(500);
	
	// now it's time to check if ESP spi interface is ready
	// we do so issuing a STATUS_PRESENCE_TEST command
	// followed by a status read wich should return a known word
	// we check for PRESENCE_TEST_TIMEOUT milliseconds max, then give up
	// between retries we wait for PRESENCE_TEST_RETRY_TIME
	uint32_t reset_fin = 0;
	uint32_t reset_magic;
	uint8_t *reset_magic_ptr;
	while(reset_fin < PRESENCE_TEST_TIMEOUT)
	{
		// send a presence test command
		spiSelect(true);
		SPI.transfer(ESP_STATUS_WRITE);		// write status
		SPI.transfer(STATUS_PRESENCE_TEST);	// presence test command
		spiSelect(false);
		
		delay(10);

		// read status back
		reset_magic_ptr = (uint8_t *)&reset_magic;
		spiWaitReady(100);
		spiSelect(true);
		SPI.transfer(ESP_STATUS_READ);		// read status
		*reset_magic_ptr++ = SPI.transfer(0x00);
		*reset_magic_ptr++ = SPI.transfer(0x00);
		*reset_magic_ptr++ = SPI.transfer(0x00);
		*reset_magic_ptr++ = SPI.transfer(0x00);
		spiSelect(false);

		// check if we got the right magic dword
		if(reset_magic == PRESENCE_MAGIC)
			break;
		
		// nope, retry
		delay(PRESENCE_TEST_RETRY_TIME);
		reset_fin += PRESENCE_TEST_RETRY_TIME;
	}
	
	// now, if presence test went bad, we have reset_magic >= PRESENCE_TEST_TIMEOUT
	// and we give up
	if(reset_fin >= PRESENCE_TEST_TIMEOUT)
	{
		Serial << F("ESP NOT FOUND - HALTING.");
		while(true)
			;
	}

	// get and check version number
	uint32_t fwVersion = firmwareVersion();

	// allow for beta versions (version 0.5.xx)
	if(fwVersion >> 16 != 0 && ((fwVersion >> 8) & 0xff) != 5)
	{
		if(fwVersion < FISHINO_FIRMWARE_VERSION_MIN)
		{
			Serial << F("Firmware version ") << firmwareVersionStr() << F(" is too old\r\n");
			Serial << F("Please upgrade to at least ") <<
				(FISHINO_FIRMWARE_VERSION_MIN >> 16) << F(".") <<
				((FISHINO_FIRMWARE_VERSION_MIN >> 8) & 0xff) << F(".") <<
				((FISHINO_FIRMWARE_VERSION_MIN >> 16) & 0xff)
			;
			while(true)
				;
		}
		else if(fwVersion >> 16 != FISHINO_FIRMWARE_VERSION_MIN >> 16)
		{
			Serial << F("Firmware version ") << firmwareVersionStr() << F(" is too new\r\n");
			Serial << F("Please update this library");
			while(true)
				;
		}
	}
	
	return true;
}

// Get firmware version
uint32_t FishinoClass::firmwareVersion()
{
	// no need to ask twice
	if(_fwVersion != (uint32_t)-1)
		return _fwVersion;
	
	uint8_t devel, minor;
	uint16_t major;
	
	// send command
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_GETFWVERSION);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			break;
		if(
			!spiReceive8(devel)		||
			!spiReceive8(minor)		||
			!spiReceive16(major)
		)
			continue;
		break;
	}
	if(res)
		_fwVersion = (uint32_t)major << 16 | (uint32_t)minor << 8 | devel;
	else
		_fwVersion = (uint32_t)-1;
	return _fwVersion;
}

char *FishinoClass::firmwareVersionStr()
{
	static char ver[] = "00000.000.000";
	
	uint32_t fv = firmwareVersion();
	if(fv != (uint32_t)-1)
		sprintf(ver, "%u.%u.%u", (uint16_t)((fv >> 16) & 0xffff), (uint16_t)((fv >> 8) & 0xff), (uint16_t)(fv & 0xff));
	else
		strcpy(ver, "UNKNOWN");
	return ver;
}


// set operation mode (one of WIFI_MODE)
bool FishinoClass::setMode(uint8_t mode)
{
	uint8_t res = false;

	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETMODE);
		if(!spiSend8(mode))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
		continue;
		break;
	}
	return res;
}

// get operation mode
uint8_t FishinoClass::getMode(void)
{
	uint8_t res = false;
	uint8_t mode = NULL_MODE;

	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_QUERYMODE);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			break;
		if(!spiReceive8(mode))
			continue;
		break;
	}
	return res ? mode : NULL_MODE;
}

// Start Wifi connection for OPEN networks
// param ssid: Pointer to the SSID string.
// return one of JOIN_STATUS result
uint8_t FishinoClass::begin(const char* ssid)
{
	return begin(ssid, "");
}

uint8_t FishinoClass::begin(const __FlashStringHelper *ssid)
{
	return begin(ssid, F(""));
}
		
// Start Wifi connection with encryption.
// return one of JOIN_STATUS result
uint8_t FishinoClass::begin(const char* ssid, const char *passphrase)
{
	// freea any previous SSID
	if(_ssid)
	{
		free(_ssid);
		_ssid = NULL;
	}
	
	// send command
	uint8_t res;
	REPEAT_COMMAND
	{
		res = STATION_IDLE;
		spiStartCommand(CMD_JOINAP);
		// send request to ESP
		if(
			!spiSendString(ssid)		||
			!spiSendString(passphrase)
		)
			continue;
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}
	if(!res)
		return STATION_IDLE;

	// try to join for 20 seconds maximum
	res = STATION_IDLE;
	if(spiWaitReady(20000))
	{
		// get status byte back
		if(spiReceive8(res))
		{
			if(res)
				_ssid = strdup(ssid);
		}
	}
	
	return res ? STATION_GOT_IP : 0;
}

uint8_t FishinoClass::begin(const __FlashStringHelper *ssid, const __FlashStringHelper *passphrase)
{
	// freea any previous SSID
	if(_ssid)
	{
		free(_ssid);
		_ssid = NULL;
	}
	
	// send command
	uint8_t res;
	REPEAT_COMMAND
	{
		res = STATION_IDLE;
		spiStartCommand(CMD_JOINAP);
		// send request to ESP
		if(
			!spiSendString(ssid)		||
			!spiSendString(passphrase)
		)
			continue;
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}
	if(!res)
		return STATION_IDLE;

	// try to join for 20 seconds maximum
	res = STATION_IDLE;
	if(spiWaitReady(20000))
	{
		// get status byte back
		if(spiReceive8(res))
		{
			if(res)
				_ssid = strdup_P(ssid);
		}
	}
	
	return res ? STATION_GOT_IP : 0;
}

// Change Ip configuration settings disabling the dhcp client
//	param local_ip
// WARNING : you should also set gateway and netmask (see next function)
// if you set only the IP, the system will default with :
// IP      : a, b, c, d
// gateway : a, b, c, 1
// netmask : 255, 255, 255, 0
// (which is OK for most but not all situations)
bool FishinoClass::config(IPAddress local_ip)
{
	uint8_t res = false;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAIPINFO);
		if(!spiSendData(&local_ip[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

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
bool FishinoClass::config(IPAddress local_ip, IPAddress gateway)
{
	uint8_t res = false;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAIPINFO);
		if(!spiSendData(&local_ip[0], 4))
			continue;
		if(!spiSendData(&gateway[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// Change Ip configuration settings disabling the dhcp client
// param local_ip:			Static ip configuration
// param gateway :			Static gateway configuration
// param subnet:			Static Subnet mask
bool FishinoClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet)
{
	uint8_t res = false;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAIPINFO);
		if(!spiSendData(&local_ip[0], 4))
			continue;
		if(!spiSendData(&gateway[0], 4))
			continue;
		if(!spiSendData(&subnet[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// Change Ip configuration settings disabling the dhcp client
// param local_ip:			Static ip configuration
// param gateway:			Static gateway configuration
// param subnet:			Static Subnet mask
// param dns_server:		IP configuration for DNS server 1
// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
bool FishinoClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns_server)
{
	uint8_t res = false;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAIPINFO);
		if(!spiSendData(&local_ip[0], 4))
			continue;
		if(!spiSendData(&gateway[0], 4))
			continue;
		if(!spiSendData(&subnet[0], 4))
			continue;
		if(!spiSendData(&dns_server[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// Change DNS Ip configuration
// param dns_server1:		ip configuration for DNS server 1
// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
bool FishinoClass::setDNS(IPAddress dns_server1)
{
	return false;
}

// Change DNS Ip configuration
// param dns_server1:		ip configuration for DNS server 1
// param dns_server2:		ip configuration for DNS server 2
// DNS SERVER SETTING NOT SUPPORTED BY NOW -- DUMMY
bool FishinoClass::setDNS(IPAddress dns_server1, IPAddress dns_server2)
{
	return false;
}

// Disconnect from the network
// return: one value of wl_status_t enum
bool FishinoClass::disconnect(void)
{
	uint8_t res = false;

	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_QUITAP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}

	return res;
}

// Get the interface MAC address.
// return: pointer to A STATIC uint8_t array with length WL_MAC_ADDR_LENGTH
const uint8_t* FishinoClass::macAddress(void)
{
	uint8_t res = false;
	static uint8_t MAC[6];
	uint16_t macBufLen = sizeof(MAC);

	// send command
	REPEAT_COMMAND
	{
		res = false;
		memset(MAC, macBufLen, 0);
		spiStartCommand(CMD_GETSTAMAC);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return MAC;
		if(!spiReceiveDataFixed(MAC, macBufLen))
			continue;
		break;
	}
	
	return MAC;
}

// Get the interface IP address.
// return: Ip address value
IPAddress FishinoClass::localIP()
{
	uint8_t res = false;
	uint8_t buf[4];
	uint16_t bufLen = sizeof(buf);

	// send command
	REPEAT_COMMAND
	{
		res = false;
		memset(buf, 0, bufLen);
		spiStartCommand(CMD_GETSTAIP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			break;
		if(!spiReceiveDataFixed(buf, bufLen))
			continue;
		break;
	}

	return IPAddress(buf);
}

// Get the interface subnet mask address.
// return: subnet mask address value
IPAddress FishinoClass::subnetMask()
{
	uint8_t res = false;
	IPAddress ip;
	uint16_t ipBufLen = sizeof(IPAddress);

	// send command
	REPEAT_COMMAND
	{
		res = false;
		memset((uint8_t *)&ip, 0, ipBufLen);
		spiStartCommand(CMD_GETSTANM);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			break;
		if(!spiReceiveDataFixed((uint8_t *)&ip, ipBufLen))
			continue;
		break;
	}
	
	return ip;
}

// Get the gateway ip address.
// return: gateway ip address value
IPAddress FishinoClass::gatewayIP()
{
	uint8_t res = false;
	IPAddress ip;
	uint16_t ipBufLen = sizeof(IPAddress);

	// send command
	REPEAT_COMMAND
	{
		res = false;
		memset((uint8_t *)&ip, 0, ipBufLen);
		spiStartCommand(CMD_GETSTAGW);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			break;
		if(!spiReceiveDataFixed((uint8_t *)&ip, ipBufLen))
			continue;
		break;
	}

	return ip;
}

// Return the current SSID associated with the network
// return: ssid string
const char* FishinoClass::SSID()
{
	return _ssid;
}

// Return the current BSSID associated with the network.
// It is the MAC address of the Access Point
// return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
const uint8_t* FishinoClass::BSSID()
{
	static const uint8_t dummy[6] = {0, 0, 0, 0, 0, 0};
	
	if(!_ssid)
		return dummy;
	APINFO const *info = findApInfo(_ssid);
	if(!info)
	{
		scanNetworks();
		info = findApInfo(_ssid);
	}
	if(!info)
		return dummy;
	return info->bssid;
}

// Return the current RSSI /Received Signal Strength in dBm)
// associated with the network
// return: signed value
int32_t FishinoClass::RSSI()
{
	if(!_ssid)
		return 0;
	APINFO const *info = findApInfo(_ssid);
	if(!info)
	{
		scanNetworks();
		info = findApInfo(_ssid);
	}
	if(!info)
		return 0;
	return info->rssi;
}

// Return the Encryption Type associated with the network
// return: one value of wl_enc_type enum
uint8_t	FishinoClass::encryptionType()
{
	if(!_ssid)
		return 0;
	APINFO const *info = findApInfo(_ssid);
	if(!info)
	{
		scanNetworks();
		info = findApInfo(_ssid);
	}
	if(!info)
		return 0;
	return info->authmode;
}

// Start scan WiFi networks available
// return: Number of discovered networks
uint8_t FishinoClass::scanNetworks()
{
	// frees any previous ap info
	freeApInfo();

	// send command
	uint8_t res = false;
	REPEAT_COMMAND
	{
		spiStartCommand(CMD_LISTAP);
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}
	if(!res)
		return 0;
		
	// try 10 seconds to get ap list
	if(
		!spiWaitReady(10000) ||
		!spiReceive8(res)
	)
		return 0;

	if(!res)
		return 0;

	// get number of aps
	if(!spiReceive8(_numAp))
	{
		_numAp = 0;
		return 0;
	}

	// allocate APINFO array
	_apInfo = (APINFO *)FISHINO_MALLOC(_numAp * sizeof(APINFO));
	memset(_apInfo, 0, _numAp * sizeof(APINFO));
	
	for(int i = 0; i < _numAp; i++)
	{
		// read authmode
		uint8_t authMode;
		char *cSSID = NULL;
		char RSSI;
		uint8_t mac[6];
		uint16_t macLen = 6;
		uint8_t channel;

		if(
			!spiReceive8(authMode)				||
			!spiReceiveString(cSSID)			||
			!spiReceive8((byte &)RSSI)			||
			!spiReceiveDataFixed(mac, macLen)	||
			!spiReceive8(channel)
		)
		{
			freeApInfo();
			return 0;
		}
		APINFO *curInfo = &_apInfo[i];
		curInfo->authmode = authMode;
		curInfo->ssid = cSSID;
		curInfo->rssi = RSSI;
		curInfo->channel = channel;
		memcpy(curInfo->bssid, mac, 6);
	}

	return _numAp;
}

// Return the SSID discovered during the network scan.
// param networkItem: specify from which network item want to get the information
// return: ssid string of the specified item on the networks scanned list
const char* FishinoClass::SSID(uint8_t networkItem)
{
	if(networkItem >= _numAp)
		return NULL;
	return _apInfo[networkItem].ssid;
}

// Return the encryption type of the networks discovered during the scanNetworks
// param networkItem: specify from which network item want to get the information
// return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
uint8_t FishinoClass::encryptionType(uint8_t networkItem)
{
	if(networkItem >= _numAp)
		return 0;
	return _apInfo[networkItem].authmode;
}

// Return the RSSI of the networks discovered during the scanNetworks
// param networkItem: specify from which network item want to get the information
// return: signed value of RSSI of the specified item on the networks scanned list
int32_t FishinoClass::RSSI(uint8_t networkItem)
{
	if(networkItem >= _numAp)
		return 0;
	return _apInfo[networkItem].rssi;
}

// Return Connection status.
// return: one of the value defined in JOIN_STATUS
uint8_t FishinoClass::status()
{
	uint8_t res = STATION_IDLE;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_JOINSTATUS);
		if(!spiEndCommand())
			continue;
		
		// try 1 second max to get response
		if(
			!spiWaitReady(1000) ||
			!spiReceive8(res)
		)
			continue;
		return res;
	}
	return STATION_IDLE;
}

// Resolve the given hostname to an IP address.
// param aHostname: Name to be resolved
// param aResult: IPAddress structure to store the returned IP address
// result: 1 if aIPAddrString was successfully converted to an IP address,
// else error code
bool FishinoClass::hostByName(const char* aHostname, IPAddress& aResult)
{
	// STILL NOT IMPLEMENTED
	return false;
}

// low level TCP stuffs -- used by client and server

// connect to host -- return socket number, or 0xff if error
uint8_t FishinoClass::connect(const char *host, uint16_t port)
{
	uint8_t res;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_CONNECT);
		if(
			// type 0 == TCP
			!spiSend8(0)			||
			!spiSendString(host)	||
			!spiSend32(port)
		)
			continue;
		if(!spiEndCommand())
			continue;
		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		if(!res)
			return 0xff;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// connect to a secure host (HTTPS - SSL)
uint8_t FishinoClass::secureConnect(const char *host, uint16_t port)
{
	uint8_t res;
	
	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_CONNECT);
		if(
			// type 2 == TCP+SSL
			!spiSend8(2)			||
			!spiSendString(host)	||
			!spiSend32(port)
		)
			continue;
		if(!spiEndCommand())
			continue;
		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		if(!res)
			return 0xff;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}


// disconnect from host -- return true on success, false otherwise
bool FishinoClass::disconnect(uint8_t sock)
{
	uint8_t res;

	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_CLOSE);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		// try 5 seconds to disconnect
		if(
			!spiWaitReady(5000)		||
			!spiReceive8(res)
		)
			continue;
		break;
	}
	return res;
}

// write to socket -- return true on success, false otherwise
bool FishinoClass::write(uint8_t sock, uint8_t const *buf, uint16_t bufLen)
{
	// send command
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SEND);
		if(
			!spiSend8(sock)				||
			!spiSendData(buf, bufLen)
		)
			continue;
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}
	if(!res || !spiWaitReady(20000))
		return false;
	if(!spiReceive8(res))
		return false;
	return res;
}

// read from socket -- return true on success, false otherwise
// buffer MUST be big enough to contain reqBytes
// on exit, bufLen contains number of bytes actually read
bool FishinoClass::read(uint8_t sock, uint16_t reqBytes, uint8_t *buf, uint16_t &bufLen)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_READ);
		if(
			!spiSend8(sock)			||
			!spiSend16(reqBytes)
		)
			continue;
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}

	// try 5 seconds to get data
	if(!res || !spiWaitReady(20000))
		return false;
	if(!spiReceive8(res))
		return false;
	if(!res)
		return false;
	
	bufLen = reqBytes;
	if(!spiReceiveDataFixed(buf, bufLen))
		return false;
	return true;
}

// return number of available bytes on socket
uint16_t FishinoClass::available(uint8_t sock)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_AVAIL);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;

		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;

		if(!res)
			return false;
		
		// status ok, retrieve number of bytes
		uint16_t size = 0;
		if(!spiReceive16(size))
			continue;
		return size;
	}
	return 0;
}

// peek a data byte from socket, if any
// returns true on success, false otherwise
bool FishinoClass::peek(uint8_t sock, uint8_t &b)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_PEEK);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
	
		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		
		// we espect receive ok status
		if(!res)
			return false;
	
		// get peeked byte
		if(!spiReceive8(res))
			continue;
		b = res;
		return true;
	}
	return false;
}

// flush socket data
bool FishinoClass::flush(uint8_t sock)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_FLUSH);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;

		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		return res;
	}
	return false;
}

// connection status - true if connected, false if disconnected or invalid
bool FishinoClass::status(uint8_t sock)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_CONNSTATUS);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
	
		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		break;
	}
	return res;
}

// start a server on given port
// ONLY ONE SERVER AT A TIME
bool FishinoClass::startServer(uint32_t port, uint16_t timeout)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_STARTSERVER);
		if(
			!spiSend32(port)	||
			!spiSend16(timeout)
		)
			continue;
		if(!spiEndCommand())
			continue;

		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		break;
	}
	return res;
}

// stops server
bool FishinoClass::stopServer(void)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_STOPSERVER);
		if(!spiEndCommand())
			continue;

		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		break;
	}
	return res;
}

// poll server for sockets with data
// returns a dynamic allocated buffer which MUST be freed by free
bool FishinoClass::pollServer(bool all, uint8_t *&buf, uint16_t &count)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		buf = NULL;
		count = 0;
		res = false;
		spiStartCommand(CMD_POLLSERVER);
		if(!spiSend8(all))
			continue;
		if(!spiEndCommand())
			continue;
	
		// try 5 seconds to get data
		if(!spiWaitReady(5000))
			continue;
		
		// get status
		if(	!spiReceive8(res))
			continue;
		if(!res)
			return false;

		if(!spiReceiveDataAlloc(buf, count))
		{
			if(buf)
				free(buf);
			buf = NULL;
			count = 0;
			continue;
		}
		break;
	}
	return res;
}

// "uniform" functions to set station and AP parameters
// station ones replicates the above ones, jus to have meaningful names
bool FishinoClass::setStaIP(IPAddress ip)
{
	return config(ip);
}

bool FishinoClass::setStaMAC(uint8_t const *mac)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAMAC);
		if(!spiSendData(mac, 6))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setStaGateway(IPAddress gw)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTAGW);
		if(!spiSendData(&gw[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setStaNetMask(IPAddress nm)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTANM);
		if(!spiSendData(&nm[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::staStartDHCP(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_ENABLESTADHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::staStopDHCP(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_DISABLESTADHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::getStaDHCPStatus(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_QUERYSTADHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setApIP(IPAddress ip)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETAPIP);
		if(!spiSendData(&ip[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setApMAC(uint8_t const *mac)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETAPMAC);
		if(!spiSendData(mac, 6))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setApGateway(IPAddress gw)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETAPGW);
		if(!spiSendData(&gw[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setApNetMask(IPAddress nm)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETAPNM);
		if(!spiSendData(&nm[0], 4))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// full AP IP info
bool FishinoClass::setApIPInfo(IPAddress ip, IPAddress gateway, IPAddress netmask)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETAPIPINFO);
		if(
			!spiSendData(&ip[0], 4)			||
			!spiSendData(&gateway[0], 4)	||
			!spiSendData(&netmask[0], 4)	||
			!spiEndCommand()				||
			!spiReceive8(res)
		)
			continue;
		break;
	}
	return res;
}

bool FishinoClass::getApIPInfo(IPAddress &ip, IPAddress &gateway, IPAddress &netmask)
{
	uint8_t res = false;
	uint16_t siz = sizeof(IPAddress);
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_GETAPIPINFO);
		if(
			!spiEndCommand()				||
			!spiReceive8(res)
		)
			continue;
		if(!res)
			return false;
		if(!spiReceiveDataFixed(&ip[0], siz))
			continue;
		if(!spiReceiveDataFixed(&gateway[0], siz))
			continue;
		if(!spiReceiveDataFixed(&netmask[0], siz))
			continue;
		break;
	}
	return res;
}

// softAp DHCP
bool FishinoClass::softApStartDHCPServer(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_ENABLEAPDHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::softApStartDHCPServer(IPAddress startIP, IPAddress endIP)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_ENABLEAPDHCP);
		if(
			!spiSendData(&startIP[0], 4)	||
			!spiSendData(&endIP[0], 4)
		)
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::softApStopDHCPServer(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_DISABLEAPDHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::getSoftApDHCPServerStatus(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_QUERYAPDHCP);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// softAp configuration

// get ap SSID - return a dynamic buffer that MUST be freed by free()
// return NULL on error
char *FishinoClass::softApGetSSID(void)
{
	char *SSID, *pass;
	uint8_t channel;
	bool hidden;
	if(!softApGetConfig(SSID, pass, channel, hidden))
		return NULL;
	if(pass)
		free(pass);
	return SSID;
}

// get ap PASSWORD - return a dynamic buffer that MUST be freed by free()
// return NULL on error
char *FishinoClass::softApGetPassword(void)
{
	char *SSID, *pass;
	uint8_t channel;
	bool hidden;
	if(!softApGetConfig(SSID, pass, channel, hidden))
		return NULL;
	if(SSID)
		free(SSID);
	return pass;
}

// return ap channel
// return 0 on error
uint8_t FishinoClass::softApGetChannel(void)
{
	char *SSID, *pass;
	uint8_t channel;
	bool hidden;
	if(!softApGetConfig(SSID, pass, channel, hidden))
		return 0;
	if(SSID)
		free(SSID);
	if(pass)
		free(pass);
	return channel;
}

// return ap hidden state
bool FishinoClass::softApGetHidden(void)
{
	char *SSID, *pass;
	uint8_t channel;
	bool hidden;
	if(!softApGetConfig(SSID, pass, channel, hidden))
		return false;
	if(SSID)
		free(SSID);
	if(pass)
		free(pass);
	return hidden;
}

// get all softAp config data
// warning - all returned strings MUST be freed by free()
bool FishinoClass::softApGetConfig(char *&SSID, char *&pass, uint8_t &channel, bool &hidden)
{
	uint8_t res = false;

	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_APGETCONFIG);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		
		if(!res)
			return false;

		SSID = NULL;
		pass = NULL;
		uint8_t chan, hid;
		if(
			!spiReceiveString(SSID)		||
			!spiReceiveString(pass)		||
			!spiReceive8(chan)			||
			!spiReceive8(hid)
		)
		{
			// cleanup on errors
			if(SSID)
				free(SSID);
			if(pass)
				free(pass);
			SSID = pass = NULL;
			continue;
		}
		else
		{
			channel = chan;
			hidden = hid;
			return true;
		}
	}
	return false;
}

// set softAp parameters
bool FishinoClass::softApConfig(const char *SSID, const char *pass, uint8_t channel, bool hidden)
{
	uint8_t res = false;

	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_APSETCONFIG);
		if(
			!spiSendString(SSID)	||
			!spiSendString(pass)	||
			!spiSend8(channel)		||
			!spiSend8(hidden)
		)
			continue;
		if(!spiEndCommand())
			continue;

		// get response back
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::softApConfig(const __FlashStringHelper *SSID, const __FlashStringHelper *pass, uint8_t channel, bool hidden)
{
	uint8_t res = false;

	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_APSETCONFIG);
		if(
			!spiSendString(SSID)	||
			!spiSendString(pass)	||
			!spiSend8(channel)		||
			!spiSend8(hidden)
		)
			continue;
		if(!spiEndCommand())
			continue;

		// get response back
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// setup station AP parameters without joining
bool FishinoClass::setStaConfig(const char *ssid)
{
	return setStaConfig(ssid, "");
}

// setup station AP parameters without joining
bool FishinoClass::setStaConfig(const __FlashStringHelper *ssid)
{
	return setStaConfig(ssid, F(""));
}

bool FishinoClass::setStaConfig(const char *ssid, const char *passphrase)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTACONFIG);
		if(
			!spiSendString(ssid)		||
			!spiSendString(passphrase)
		)
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::setStaConfig(const __FlashStringHelper *ssid, const __FlashStringHelper *passphrase)
{
	// send command
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETSTACONFIG);
		// send request to ESP
		if(
			!spiSendString(ssid)		||
			!spiSendString(passphrase)
		)
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// get current station AP parameters
// WARNING : returns dynamic buffers wich MUST be freed
bool FishinoClass::getStaConfig(char *&ssid, char *&pass)
{
	uint8_t res = false;
	ssid = NULL;
	pass = NULL;

	REPEAT_COMMAND
	{
		ssid = NULL;
		pass = NULL;
		uint8_t *mac = NULL;
		uint16_t macSize = 0;
		res = false;
		spiStartCommand(CMD_GETSTACONFIG);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return false;
		res = false;

		if(
			!spiReceiveString(ssid)				||
			!spiReceiveString(pass)				||
			!spiReceiveDataAlloc(mac, macSize)
		)
		{
			// cleanup
			if(ssid)
				free(ssid);
			if(pass)
				free(pass);
			if(mac)
				free(mac);
			continue;
		}
		if(mac)
			free(mac);
		break;
	}
	
	return res;
}

// join AP setup by setStaConfig call (or previous AP set by begin)
bool FishinoClass::joinAp(void)
{
	uint8_t res = false;
	
	// free any previous SSID
	if(_ssid)
	{
		free(_ssid);
		_ssid = NULL;
	}
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_STACONNECT);
		if(!spiEndCommand())
			continue;

		// try to join for 20 seconds maximum
		if(
			!spiWaitReady(20000)	||
			!spiReceive8(res)
		)
			continue;
		if(!res)
			return false;
		res = false;

		// get ssid from esp
		char *pass = NULL;
		res = getStaConfig(_ssid, pass);
		if(pass)
			free(pass);
		if(!res)
			continue;
		break;
	}
	return res;
}

// quits AP
bool FishinoClass::quitAp(void)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_STADISCONNECT);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

////////////////////////////////////////////////////////////////////////
// extra I/O pins on ESP module

// pinMode for ESP I/O
bool FishinoClass::pinMode(uint8_t pin, uint8_t mode)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_PINMODE);
		if(
			!spiSend8(pin)	||
			!spiSend8(mode)
		)
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// digital I/O for ESP I/O
uint8_t FishinoClass::digitalRead(uint8_t pin)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_DIGITALREAD);
		if(!spiSend8(pin))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::digitalWrite(uint8_t pin, uint8_t val)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_DIGITALWRITE);
		if(
			!spiSend8(pin)	||
			!spiSend8(val)
		)
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// analog read
uint16_t FishinoClass::analogRead(void)
{
	uint8_t res = false;
	uint16_t val = 0;
	
	REPEAT_COMMAND
	{
		res = false;
		val = 0;
		spiStartCommand(CMD_ANALOGREAD);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return 0;
		if(!spiReceive16(val))
			continue;
		break;
	}
	return val;
}

// physical mode handling
uint8_t FishinoClass::getPhyMode(void)
{
	uint8_t res = false;
	uint8_t mode = PHY_MODE_UNKNOWN;
	
	REPEAT_COMMAND
	{
		res = false;
		mode = PHY_MODE_UNKNOWN;
		spiStartCommand(CMD_GETPHYMODE);
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return PHY_MODE_UNKNOWN;
		if(!spiReceive8(mode))
			continue;
		return mode;
	}
	return PHY_MODE_UNKNOWN;
}

bool FishinoClass::setPhyMode(uint8_t mode)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_SETPHYMODE);
		if(!spiSend8(mode))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// UDP STUFFS

// binds UDP socket to IP/Port
// returns udp socket number
uint8_t FishinoClass::udpBegin(uint32_t localPort)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPBEGIN);
		if(!spiSend32(localPort))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return 0xff;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// ends UDP socked binding
bool FishinoClass::udpEnd(uint8_t sock)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPEND);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// starts building a packet for sending to a receiver
bool FishinoClass::udpBeginPacket(uint8_t sock, const char *host, uint32_t localPort)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPBEGINPACKET);
		if(!spiSend8(sock))
			continue;
		if(!spiSendString(host))
			continue;
		if(!spiSend32(localPort))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::udpBeginPacket(uint8_t sock, const __FlashStringHelper *host, uint32_t localPort)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPBEGINPACKET);
		if(!spiSend8(sock))
			continue;
		if(!spiSendString(host))
			continue;
		if(!spiSend32(localPort))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

bool FishinoClass::udpBeginPacket(uint8_t sock, const IPAddress &ip, uint32_t localPort)
{
	char host[16];
	snprintf(host, 15, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	host[15] = 0;

	return udpBeginPacket(sock, host, localPort);
}

// ends and send current packet
bool FishinoClass::FishinoClass::udpEndPacket(uint8_t sock)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPENDPACKET);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// parse currently received UDP packet and returns number of available bytes
uint16_t FishinoClass::FishinoClass::udpParsePacket(uint8_t sock)
{
	uint8_t res = false;
	uint16_t len;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPPARSEPACKET);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return 0;
		if(!spiReceive16(len))
			continue;
		break;
	}
	return len;
}

// write data to current packet
bool FishinoClass::udpWrite(uint8_t sock, const uint8_t *buf, uint16_t len)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPWRITE);
		if(!spiSend8(sock))
			continue;
		if(!spiSendData(buf, len))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// check if data is available in current packet
uint16_t FishinoClass::udpAvail(uint8_t sock)
{
	uint8_t res = false;
	uint16_t len;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPAVAIL);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return 0;
		if(!spiReceive16(len))
			continue;
		break;
	}
	return len;
}

// read data from current packet
// buffer MUST be big enough to contain reqBytes
// on exit, bufLen contains number of bytes actually read
bool FishinoClass::udpRead(uint8_t sock, uint16_t reqBytes, uint8_t *buf, uint16_t &bufLen)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPREAD);
		if(
			!spiSend8(sock)			||
			!spiSend16(reqBytes)
		)
			continue;
		if(!spiEndCommand())
			continue;
		res = true;
		break;
	}

	// try 5 seconds to get data
	if(!res || !spiWaitReady(20000))
		return false;
	if(!spiReceive8(res))
		return false;
	if(!res)
		return false;
	
	bufLen = reqBytes;
	if(!spiReceiveDataFixed(buf, bufLen))
		return false;
	return true;
}

// peek next byte in current packet
bool FishinoClass::udpPeek(uint8_t sock, uint8_t &b)
{
	uint8_t res = false;
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPPEEK);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
	
		// try 5 seconds to get data
		if(
			!spiWaitReady(5000)	||
			!spiReceive8(res)
		)
			continue;
		
		// we espect receive ok status
		if(!res)
			return false;
	
		// get peeked byte
		if(!spiReceive8(res))
			continue;
		b = res;
		return true;
	}
	return false;
}

// Finish reading the current packet
bool FishinoClass::udpFlush(uint8_t sock)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPFLUSH);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// flush all stored UDP packets
bool FishinoClass::udpFlushAll(uint8_t sock)
{
	uint8_t res = false;
	
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPFLUSHALL);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		break;
	}
	return res;
}

// Return the IP address of the host who sent the current incoming packet
bool FishinoClass::udpRemoteIP(uint8_t sock, IPAddress &ip)
{
	uint8_t res = false;
	uint8_t buf[4];
	uint16_t bufLen = sizeof(buf);

	// send command
	REPEAT_COMMAND
	{
		res = false;
		memset(buf, 0, bufLen);
		spiStartCommand(CMD_UDPREMOTEIP);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return false;
		if(!spiReceiveDataFixed(buf, bufLen))
			continue;
		break;
	}

	ip = IPAddress(buf);
	return true;
}

// Return the port of the host who sent the current incoming packet
bool FishinoClass::udpRemotePort(uint8_t sock, uint32_t &port)
{
	uint8_t res = false;

	// send command
	REPEAT_COMMAND
	{
		res = false;
		spiStartCommand(CMD_UDPREMOTEPORT);
		if(!spiSend8(sock))
			continue;
		if(!spiEndCommand())
			continue;
		if(!spiReceive8(res))
			continue;
		if(!res)
			return false;
		if(!spiReceive32(port))
			continue;
		break;
	}

	return res;
}

// TEST
void FishinoClass::test(void)
{
/*
	uint8_t res;
	spiStartCommand(CMD_TEST);
	res = spiSendString("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura");
	Serial << "SpiSendString returned " << (res ? "TRUE" : "FALSE") << "\n";
	if(!res)
		return;
	res = spiSendString("Ed ecco un'altra stringa di prova");
	Serial << "SpiSendString returned " << (res ? "TRUE" : "FALSE") << "\n";
	if(!res)
		return;
	res = spiEndCommand();
	Serial << "SpiEndCommand returned " << (res ? "TRUE" : "FALSE") << "\n";
	if(!res)
		return;
	if(!spiReceive8(res) || res == false)
		return;
	uint16_t nStrings;
	if(!spiReceive16(nStrings))
		Serial << F("Failed to get number of strings\n");
	else
		Serial << F("Expecting ") << nStrings << F(" strings\n");
	for(uint16_t i = 0; i < nStrings; i++)
	{
		char *s;
		if(!spiReceiveString(s))
		{
			Serial << F("Failed to receive string ") << i << "\n";
			return;
		}
		Serial << F("Got string '") << s << "'\n";
		free(s);
	}
*/
}

FishinoClass Fishino;
