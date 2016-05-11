// Progetto : Fishino
// Creato con l'applicazione TheArduIDE
#ifndef __FISHINOSOCKBUF_H
#define __FISHINOSOCKBUF_H

#include "FishinoDebug.h"

#include <inttypes.h>

// buffer size (MAX 255 bytes, suggested 64 bytes or less for UNO, 255 for MEGA
#define FISHINO_SOCKET_BUF_SIZE		16

// number of starting buffers
// suggested number is 4 -- will grow if more buffers are requested
#define FISHINO_SOCKET_BUFFERS_START_COUNT	4

// a socket local buffer
// used to speed up transfers avoiding slow single byte
// transfers to/from ESP module
class FishinoSockBuf
{
	friend class FishinoSockBuffers;
	friend class FishinoClient;
	friend class FishinoSecureClient;
	friend class FishinoServer;
	
	// socket for allocated buffer
	uint8_t sock;
	
	// using client count
	uint8_t clientCount;
		
	// connected buffer
	uint8_t *buf;

	// character count in buffer
	uint8_t count;
	
	// current position in buffer
	uint8_t pos;
	
	// check if buffer has data
	bool hasData(void) const;
};

// buffer handler
class FishinoSockBuffers
{
	// the buffers
	FishinoSockBuf *buffers;
	
	// number of allocated buffers
	uint8_t bufAlloc;
	
	// locate a buffer from socket
	// return 0xff if none
	uint8_t FindBuf(uint8_t sock);
	
	public:
	
		// get buffer address for socket
		// return NULL if none
		FishinoSockBuf *GetBuf(uint8_t sock);
		
		// create buffer for socket
		// (or increase usage count if already there)
		FishinoSockBuf *AllocBuf(uint8_t sock);
		
		// remove buffer for socket
		void KillBuf(uint8_t sock);

		// constructor
		FishinoSockBuffers();
		
		// destructor
		~FishinoSockBuffers();
};

FishinoSockBuffers &fishinoSockBuffers();

#endif

