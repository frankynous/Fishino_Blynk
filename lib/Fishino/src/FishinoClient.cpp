#include "Arduino.h"

#define FISHINO_MODULE "FishinoClient"
#include "Fishino.h"
#include "FishinoClient.h"

#include "FishinoSockBuf.h"

//uint16_t EthernetClient::_srcport = 49152;      //Use IANA recommended ephemeral port range 49152-65535

FishinoClient::FishinoClient() : _sock(0xff)
{
}

FishinoClient::FishinoClient(uint8_t sock) : _sock(sock)
{
	fishinoSockBuffers().AllocBuf(sock);
}

// copy constructor
FishinoClient::FishinoClient(FishinoClient const &c)
{
	_sock = c._sock;
	fishinoSockBuffers().AllocBuf(_sock);
}


FishinoClient::~FishinoClient()
{
	fishinoSockBuffers().KillBuf(_sock);
}

// copy operator
FishinoClient const &FishinoClient::operator=(FishinoClient const &c)
{
	uint8_t oldSock = _sock;
	_sock = c._sock;
	fishinoSockBuffers().AllocBuf(_sock);
	fishinoSockBuffers().KillBuf(oldSock);
	return *this;
}

int FishinoClient::connect(const char* host, uint16_t port)
{
	// disconnect if already connected
	if (_sock != 0xff)
		stop();

	// connect
	_sock = Fishino.connect(host, port);
	if (_sock == 0xff)
		return false;

	// allocate buffer for me
	FishinoSockBuf *buf = fishinoSockBuffers().AllocBuf(_sock);
	if (!buf)
	{
		Serial << F("Error allocating buffer\n");
		return 0;
	}

	buf->count = 0;
	buf->pos = 0;

	return _sock != 0xff;
}

int FishinoClient::connect(IPAddress ip, uint16_t port)
{
	char host[16];
	snprintf(host, 15, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	host[15] = 0;

	return connect(host, port);
}

size_t FishinoClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FishinoClient::write(const uint8_t *buf, size_t size)
{
	if (_sock == 0xff)
	{
		setWriteError();
		return 0;
	}

	if (Fishino.write(_sock, buf, size))
		return size;

	setWriteError();
	return 0;
}

/*
static int FreeRam(void)
{
	extern int  __bss_end;
	extern int* __brkval;
	int free_memory;
	if (reinterpret_cast<int>(__brkval) == 0)
	{
		// if no heap use from end of bss section
		free_memory = reinterpret_cast<int>(&free_memory)
					  - reinterpret_cast<int>(&__bss_end);
	}
	else
	{
		// use from top of stack to heap
		free_memory = reinterpret_cast<int>(&free_memory)
					  - reinterpret_cast<int>(__brkval);
	}
	return free_memory;
}
*/

void FishinoClient::fetchBuf(void)
{
// Serial << "FETCHBUF\n";
	// get socket's buffer
	FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(_sock);
	if (!buf)
		return;

	buf->pos = 0;
	buf->count = 0;

// Serial << "FETCHBUF READ\n";
	uint16_t siz;
	if (!Fishino.read(_sock, FISHINO_SOCKET_BUF_SIZE, buf->buf, siz))
		buf->count = 0;
	else
		buf->count = siz;

// Serial << "FETCHBUF COUNT = " << buf->count << "\n";
}

int FishinoClient::available()
{
// Serial << "AVAILABLE\n";
	if (_sock == 0xff)
		return 0;

	// get socket's buffer
	FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(_sock);
	if (!buf)
		return 0;

	if (buf->pos >= buf->count)
		fetchBuf();
	if (buf->pos >= buf->count)
		return 0;
	return buf->count - buf->pos;
}

int FishinoClient::read()
{
	if (_sock == 0xff)
		return -1;

	// get socket's buffer
	FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(_sock);
	if (!buf)
		return -1;

	if (buf->pos >= buf->count)
		fetchBuf();
	if (buf->pos >= buf->count)
		return -1;
	return buf->buf[buf->pos++];
}

int FishinoClient::read(uint8_t *buf, size_t size)
{
	if (_sock == 0xff)
		return 0;

	// get socket's buffer
	FishinoSockBuf *sbuf = fishinoSockBuffers().GetBuf(_sock);
	if (!sbuf)
		return 0;

	size_t siz = 0;
	uint8_t *bufP = buf;
	while (siz < size)
	{
		if (sbuf->pos >= sbuf->count)
			fetchBuf();
		if (sbuf->pos >= sbuf->count)
			return siz;
		size_t cnt = sbuf->count - sbuf->pos;
		if (siz + cnt > size)
			cnt = size - siz;
		memcpy(bufP, sbuf->buf + sbuf->pos, cnt);
		siz += cnt;
		bufP += cnt;
		sbuf->pos += cnt;
	}
	return siz;
}

int FishinoClient::peek()
{
	if (_sock == 0xff)
		return -1;

	// get socket's buffer
	FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(_sock);
	if (!buf)
		return -1;

	if (buf->pos >= buf->count)
		fetchBuf();
	if (buf->pos >= buf->count)
		return -1;
	return buf->buf[buf->pos];
}

void FishinoClient::flush()
{
	if (_sock == 0xff)
		return;

	Fishino.flush(_sock);
}

void FishinoClient::stop()
{
	if (_sock == 0xff)
		return;

	Fishino.disconnect(_sock);
	fishinoSockBuffers().KillBuf(_sock);
	_sock = 0xff;
}

uint8_t FishinoClient::connected()
{
	if (_sock == 0xff)
		return 0;

	// first look at buffer for socket
	FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(_sock);
	if (buf && buf->hasData())
		return 1;

	// still no buffer, look on ESP
	return status();
}

bool FishinoClient::status()
{
	if (_sock == 0xff)
		return false;

	return Fishino.status(_sock);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

FishinoClient::operator bool()
{
	return _sock != 0xff;
}

bool FishinoClient::operator==(const FishinoClient& rhs)
{
	return _sock == rhs._sock && _sock != 0xff && rhs._sock != 0xff;
}

#define _FISHINO_PRINT_BUF_SIZE 16
size_t FishinoClient::print(const __FlashStringHelper *s)
{
	PGM_P p = reinterpret_cast<PGM_P>(s);
	char buf[_FISHINO_PRINT_BUF_SIZE];
	uint8_t iBuf = 0;
	size_t n = 0;
	while (1)
	{
		unsigned char c = pgm_read_byte(p++);
		if(c == 0)
		{
			if(iBuf > 0)
				n += write(buf, iBuf);
			return n;
		}
		buf[iBuf++] = c;
		if(iBuf >= _FISHINO_PRINT_BUF_SIZE)
		{
			n +=write(buf, _FISHINO_PRINT_BUF_SIZE);
			iBuf = 0;
		}
	}
}

size_t FishinoClient::println(const __FlashStringHelper *s)
{
	PGM_P p = reinterpret_cast<PGM_P>(s);
	char buf[_FISHINO_PRINT_BUF_SIZE];
	uint8_t iBuf = 0;
	size_t n = 0;
	while (1)
	{
		unsigned char c = pgm_read_byte(p++);
		if(c == 0)
		{
			if(iBuf > 0)
				n += write(buf, iBuf);
			break;
		}
		buf[iBuf++] = c;
		if(iBuf >= _FISHINO_PRINT_BUF_SIZE)
		{
			n +=write(buf, _FISHINO_PRINT_BUF_SIZE);
			iBuf = 0;
		}
	}
	
	if(iBuf >= _FISHINO_PRINT_BUF_SIZE - 1)
	{
		n +=write(buf, iBuf);
		iBuf = 0;
	}
	buf[iBuf++] = '\r';
	buf[iBuf++] = '\n';
	n +=write(buf, iBuf);
	return n;
}

/////////////////////////////////////////////////////////////////////////////

// SSL support
int FishinoSecureClient::connect(const char *host, uint16_t port)
{
	// disconnect if already connected
	if (_sock != 0xff)
		stop();

	// connect
	_sock = Fishino.secureConnect(host, port);
	if (_sock == 0xff)
		return false;

	// allocate buffer for me
	FishinoSockBuf *buf = fishinoSockBuffers().AllocBuf(_sock);
	if (!buf)
	{
		Serial << F("Error allocating buffer\n");
		return 0;
	}

	buf->count = 0;
	buf->pos = 0;

	return _sock != 0xff;
}

