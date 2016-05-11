#define FISHINO_MODULE "FishinoSockBuf"
#include "FishinoSockBuf.h"

#include "Arduino.h"

// check if buffer has data
bool FishinoSockBuf::hasData(void) const
{
	if(sock == 0xff || !buf || pos >= count)
		return false;
	return true;
}

// locate a buffer from socket
// return 0xff if none
uint8_t FishinoSockBuffers::FindBuf(uint8_t sock)
{
	// no buffer for invalid socket
	if(sock == 0xFF)
		return 0xff;

	for(uint8_t i = 0; i < bufAlloc; i++)
		if(buffers[i].sock == sock)
			return i;
		
	// not found
	return 0xff;
}
	

// get buffer address for socket
FishinoSockBuf *FishinoSockBuffers::GetBuf(uint8_t sock)
{
	// search for socket
	uint8_t iBuf = FindBuf(sock);

	// return NULL if not found
	if(iBuf == 0xff)
		return NULL;
	
	// return found buffer
	return buffers + iBuf;
}

// create buffer for socket
// (or increase usage count if already there)
FishinoSockBuf *FishinoSockBuffers::AllocBuf(uint8_t sock)
{
	// do NOT allocate for invalid socket
	if(sock == 0xff)
		return 0;

	// search for socket
	uint8_t iBuf = FindBuf(sock);

	// if not found, allocate a new buffer extending buffer count if needed
	if(iBuf == 0xff)
	{
		// first try to find a free slot
		for(uint8_t i = 0; i < bufAlloc; i++)
			if(buffers[i].sock == 0xff)
			{
				iBuf = i;
				break;
			}
			
		// if no free slot is found (unlikely....) just increase number of buffers
		if(iBuf == 0xff)
		{
			FishinoSockBuf *newBufs = (FishinoSockBuf *)FISHINO_MALLOC((bufAlloc + 1) * sizeof(FishinoSockBuf));
			memcpy(newBufs, buffers, bufAlloc * sizeof(FishinoSockBuf));
			free(buffers);
			buffers = newBufs;
			iBuf = bufAlloc++;
			buffers[iBuf].sock = 0xff;
			buffers[iBuf].buf = NULL;
			buffers[iBuf].count = 0;
			buffers[iBuf].pos = 0;
			buffers[iBuf].clientCount = 0;
		}
	}
	
	FishinoSockBuf *buf = buffers + iBuf;
	if(buf->sock == 0xff)
	{
		buf->sock = sock;
		buf->buf = (uint8_t *)FISHINO_MALLOC(FISHINO_SOCKET_BUF_SIZE);
		buf->count = 0;
		buf->pos = 0;
		buf->clientCount = 0;
	}
	buf->clientCount++;
	return buf;
}

// remove buffer for socket (or decrease its usage counter if more than one)
void FishinoSockBuffers::KillBuf(uint8_t sock)
{
	// search for socket
	uint8_t iBuf = FindBuf(sock);
	if(iBuf == 0xff)
		return;

	FishinoSockBuf *buf = buffers + iBuf;
	if(!buf->clientCount || !--buf->clientCount)
	{
		if(buf->buf)
			free(buf->buf);
		buf->buf = NULL;
		buf->count = 0;
		buf->pos = 0;
		buf->sock = 0xff;
		buf->clientCount = 0;
	}
}

// constructor
FishinoSockBuffers::FishinoSockBuffers()
{
	bufAlloc = FISHINO_SOCKET_BUFFERS_START_COUNT;
	buffers = (FishinoSockBuf *)FISHINO_MALLOC(bufAlloc * sizeof(FishinoSockBuf));
	FishinoSockBuf *bufP = buffers;
	for(uint8_t iBuf = 0; iBuf < bufAlloc; iBuf++)
	{
		bufP->sock = 0xff;
		bufP->buf = NULL;
		bufP->count = 0;
		bufP->pos = 0;
		bufP->clientCount = 0;
		bufP++;
	}
}

// destructor
FishinoSockBuffers::~FishinoSockBuffers()
{
	// frees all allocated buffers
	FishinoSockBuf *buf = buffers;
	for(uint8_t i = 0; i < bufAlloc; i++)
	{
		if(buf->sock != 0xff && buf->buf)
			free(buf->buf);
		buf++;
	}
	free(buffers);
	buffers = NULL;
	bufAlloc = 0;
}

FishinoSockBuffers &fishinoSockBuffers()
{
	static FishinoSockBuffers bufs;
	return bufs;
}
