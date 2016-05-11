#ifndef FISHINOCLIENT_H
#define FISHINOCLIENT_H

#include "Arduino.h"
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include <Flash.h>

class FishinoClient : public Client
{
	protected:

		uint8_t _sock;

		void fetchBuf(void);

	public:
		FishinoClient();
		FishinoClient(uint8_t sock);

		// copy constructor
		FishinoClient(FishinoClient const &c);

		// destructor, frees buffer
		~FishinoClient();

		// copy operator
		FishinoClient const &operator=(FishinoClient const &c);

		bool status();

		virtual int connect(IPAddress ip, uint16_t port);
		virtual int connect(const char *host, uint16_t port);

		virtual size_t write(uint8_t);
		virtual size_t write(const uint8_t *buf, size_t size);

		virtual int available();

		virtual int read();
		virtual int read(uint8_t *buf, size_t size);

		virtual int peek();

		virtual void flush();

		virtual void stop();

		virtual uint8_t connected();
		virtual operator bool();
		virtual bool operator==(const FishinoClient&);
		virtual bool operator!=(const FishinoClient& rhs)
		{
			return !operator==(rhs);
		}

		using Print::write;
		using Print::print;
		using Print::println;
		
/*
		// these are needed to speedup flash string sending
		template<class T> size_t print(T t)							{ return Print::print(t); }

*/
		size_t print(const __FlashStringHelper *s);
		size_t println(const __FlashStringHelper *s);
		
/*
		template<class T> size_t println(T t)						{ return Print::print(t); }
		size_t println(void)										{ return Print::println(); }
		
		template<class T> FishinoClient &operator<<(T &t)			{ return (FishinoClient &)::operator<<(*this, t); }
		FishinoClient &operator<<(const __FlashStringHelper *s)		{ print(s); return *this; }
*/

/*
		size_t print(const __FlashStringHelper *s);
		size_t print(const String &s)					{ return Print::print(s); }
		size_t print(const char s[])					{ return Print::print(s); }
		size_t print(char c)							{ return Print::print(c); }
		size_t print(unsigned char c, int f = DEC)		{ return Print::print(c, f); }
		size_t print(int i, int f = DEC)				{ return Print::print(i, f); }
		size_t print(unsigned int u, int f = DEC)		{ return Print::print(u, f); }
		size_t print(long l, int f = DEC)				{ return Print::print(l, f); }
		size_t print(unsigned long ul, int f = DEC)		{ return Print::print(ul, f); }
		size_t print(double d, int f = 2)				{ return Print::print(d, f); }
		size_t print(const Printable &p)				{ return Print::print(p); }

		size_t println(const __FlashStringHelper *s);
		size_t println(const String &s)					{ return Print::println(s); }
		size_t println(const char s[])					{ return Print::println(s); }
		size_t println(char c)							{ return Print::println(c); }
		size_t println(unsigned char c, int f = DEC)	{ return Print::println(c, f); }
		size_t println(int i, int f = DEC)				{ return Print::println(i, f); }
		size_t println(unsigned int u, int f = DEC)		{ return Print::println(u, f); }
		size_t println(long l, int f = DEC)				{ return Print::println(l, f); }
		size_t println(unsigned long ul, int f = DEC)	{ return Print::println(ul, f); }
		size_t println(double d, int f = 2)				{ return Print::println(d, f); }
		size_t println(const Printable &p)				{ return Print::println(p); }
		size_t println(void)							{ return Print::println(); }
*/
//		template<class T> FishinoClient &operator<<(T &t)			{ return (FishinoClient &)::operator<<(*this, t); }
		FishinoClient &operator<<(const __FlashStringHelper *s)	{ print(s); return *this; }
};

class FishinoSecureClient : public FishinoClient
{
	private:

	protected:

	public:

		FishinoSecureClient() : FishinoClient() {};
		FishinoSecureClient(uint8_t sock) : FishinoClient(sock) {};

		// copy constructor
		FishinoSecureClient(FishinoClient const &c) : FishinoClient(c) {};

		// copy operator
		FishinoSecureClient const &operator=(FishinoSecureClient const &c)
		{
			return (FishinoSecureClient const &)FishinoClient::operator=(c);
		}

		virtual int connect(const char *host, uint16_t port);

		template<class T> FishinoSecureClient &operator<<(T &t)			{ return (FishinoSecureClient &)::operator<<(*this, t); }
		FishinoSecureClient &operator<<(const __FlashStringHelper *s)	{ print(s); return *this; }
};

#endif
