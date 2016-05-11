#ifndef FISHINOSERVER_H
#define FISHINOSERVER_H

#include "FishinoClient.h"

#include "Server.h"

class FishinoServer : public Server
{
	private:
		uint16_t _port;
		bool _accepting;
		
		// list of client socks got by POLL function
		uint16_t _curClient;
		uint16_t _nClients;
		uint8_t *_clientSocks;
		bool _processBuffers;
		
		// frees client data
		void freeClients(void);

	public:
		FishinoServer(uint16_t port);
		~FishinoServer();
		
		FishinoClient available();
		
		virtual void begin();
		virtual void end();
		
		virtual size_t write(uint8_t);
		virtual size_t write(const uint8_t *buf, size_t size);
		using Print::write;
};



#endif
