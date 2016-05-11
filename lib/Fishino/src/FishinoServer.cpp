#define FISHINO_MODULE "FishinoServer"
#include "FishinoServer.h"
#include "Fishino.h"

#include "FishinoSockBuf.h"

// frees client data
void FishinoServer::freeClients(void)
{
	if(_clientSocks)
	{
		// frees all allocated clients buffers
		for(uint8_t i = 0; i < _nClients; i++)
			fishinoSockBuffers().KillBuf(_clientSocks[i]);
		free(_clientSocks);
	}
	_nClients = 0;
	_clientSocks = NULL;
	_curClient = 0;
	_processBuffers = false;
}

FishinoServer::FishinoServer(uint16_t port)
{
	_port = port;
	
	_accepting = false;
		
	_nClients = 0;
	_clientSocks = NULL;
	_curClient = 0;
	_processBuffers = false;
}

FishinoServer::~FishinoServer()
{
	if(_accepting)
		end();
	freeClients();
}


void FishinoServer::begin()
{
	// free any previous run data
	freeClients();
	
	// start server with 100 seconds timeout
	_accepting = Fishino.startServer(_port, 100);
}

void FishinoServer::end()
{
	Fishino.stopServer();
	freeClients();
}

FishinoClient FishinoServer::available()
{
	if(!_accepting)
		begin();
	if(!_accepting)
		return FishinoClient();

	if(_nClients && !_processBuffers && _curClient >= _nClients)
	{
		_processBuffers = true;
		_curClient = 0;
	}
	
	// if we're emptying buffers, do it
	if(_processBuffers)
	{
		if(_curClient >= _nClients)
			_curClient = 0;
		
		// if we've still some buffer to process....
		while(_curClient < _nClients)
		{
			uint8_t sock = _clientSocks[_curClient++];
			FishinoSockBuf *buf = fishinoSockBuffers().GetBuf(sock);
			if(buf && buf->hasData())
				return FishinoClient(sock);
		}
		_processBuffers = false;
	}
	
	// if it gets to here, we have either some socket to process
	// or all has been processed AND buffers are empty
	// so we shall check latter case
	if(!_nClients || _curClient >= _nClients)
	{
		freeClients();
		Fishino.pollServer(false, _clientSocks, _nClients);
		
		// preallocate buffers for clients
		// so they don't get wiped by last client delete
		for(uint8_t i = 0; i < _nClients; i++)
			fishinoSockBuffers().AllocBuf(_clientSocks[i]);
	}
	
	// if still no sockets, just leave
	if(_curClient >= _nClients)
		return FishinoClient();
	
	// create a client for current socket and return it
	uint8_t sock = _clientSocks[_curClient++];
	return FishinoClient(sock);
}

size_t FishinoServer::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FishinoServer::write(const uint8_t *buffer, size_t size)
{
	// get a list of all connected client
	uint8_t *socks = NULL;
	uint16_t nSocks = 0;
	Fishino.pollServer(true, socks, nSocks);
	
	size_t n = 0;
	for(uint16_t i = 0; i < nSocks; i++)
	{
		FishinoClient client(socks[i]);
		n += client.write(buffer, size);
	}
	if(nSocks && socks)
		free(socks);

	return n;
}
