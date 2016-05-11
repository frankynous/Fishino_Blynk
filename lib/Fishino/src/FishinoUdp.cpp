/*
  FishinoUdp.h - Library for Fishino Udp packet networking.
  Copyright (c) 205 Massimo Del Fedele.  All right reserved.

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
*/

#define FISHINO_MODULE "FishinoUdp"
#include "Fishino.h"
#include "FishinoUdp.h"
#include "FishinoClient.h"
#include "FishinoServer.h"

// Constructor
FishinoUDP::FishinoUDP() : _sock(0xff)
{
}

// Start WiFiUDP socket, listening at local port PORT
uint8_t FishinoUDP::begin(uint16_t port)
{
	_sock = Fishino.udpBegin(port);
	return _sock != 0xff;
}

// return number of bytes available in the current packet,
// will return zero if parsePacket hasn't been called yet
int FishinoUDP::available()
{
	return Fishino.udpAvail(_sock);
}

// Release any resources being used by this WiFiUDP instance
void FishinoUDP::stop()
{
	Fishino.udpEnd(_sock);
	_sock = 0xff;
}

int FishinoUDP::beginPacket(const char *host, uint16_t port)
{
	return Fishino.udpBeginPacket(_sock, host, port);
}

int FishinoUDP::beginPacket(IPAddress ip, uint16_t port)
{
	return Fishino.udpBeginPacket(_sock, ip, port);
}

int FishinoUDP::endPacket()
{
	return Fishino.udpEndPacket(_sock);
}

size_t FishinoUDP::write(uint8_t b)
{
	uint8_t buf[1];
	buf[0] = b;
	return Fishino.udpWrite(_sock, buf, 1);
}

size_t FishinoUDP::write(const uint8_t *buffer, size_t size)
{
	return Fishino.udpWrite(_sock, buffer, size);
}

int FishinoUDP::parsePacket()
{
	return Fishino.udpParsePacket(_sock);
}

int FishinoUDP::read()
{
	uint8_t buf[1];
	uint16_t bufLen = 0;
	
	bool res = Fishino.udpRead(_sock, 1, buf, bufLen);
	if(!res || bufLen != 1)
		return 0;
	else
		return buf[0];
}

int FishinoUDP::read(unsigned char* buffer, size_t len)
{
	uint16_t bufLen;

	bool res = Fishino.udpRead(_sock, len, buffer, bufLen);
	if(res)
		return bufLen;
	else
		// hmmmm... not a nice result, indeed... would be better 0
		return -1;
}

int FishinoUDP::peek()
{
	uint8_t b;

	bool res = Fishino.udpPeek(_sock, b);
	if(!res)
		return -1;
	return b;
}

void FishinoUDP::flush()
{
	Fishino.udpFlush(_sock);
}

IPAddress  FishinoUDP::remoteIP()
{
	IPAddress ip;
	bool res = Fishino.udpRemoteIP(_sock, ip);
	if(!res)
		return IPAddress(0, 0, 0, 0);
	return ip;
}

uint16_t  FishinoUDP::remotePort()
{
	uint32_t port;
	
	bool res = Fishino.udpRemotePort(_sock, port);
	if(!res)
		return 0;
	else
		return port;
}

