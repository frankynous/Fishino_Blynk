/*
	Udp NTP Client
	
	Get the time from a Network Time Protocol (NTP) time server
	Demonstrates use of UDP sendPacket and ReceivePacket
	For more on NTP time servers and the messages needed to communicate with them,
	see http://en.wikipedia.org/wiki/Network_Time_Protocol
	
	created 4 Sep 2010
	by Michael Margolis
	modified 9 Apr 2012
	by Tom Igoe
	
	This code is in the public domain.
	
	2016_02_10 Adapted to Fishino by Massimo Del Fedele

 */

#include <SPI.h>
#include <Flash.h>
#include <Fishino.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA		-- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE	-- ADATTARE ALLA PROPRIA RETE WiFi !!!

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non ￨ protetta
#define MY_PASS	""

// here put required IP address of your Fishino
// comment out this line if you want AUTO IP (dhcp)
// NOTE : if you use auto IP you must find it somehow !
// inserire qui l'IP desiderato per il fishino
// commentare la linea sotto se si vuole l'IP automatico
// nota : se si utilizza l'IP automatico, occorre un metodo per trovarlo !
#define IPADDR	192, 168,   1, 251

//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

// define ip address if required
// NOTE : if your network is not of type 255.255.255.0 or your gateway is not xx.xx.xx.1
// you should set also both netmask and gateway
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

// local port to listen for UDP packets
unsigned int localPort = 2390;

// time.nist.gov NTP server
IPAddress timeServer(129, 6, 15, 28);

// NTP time stamp is in the first 48 bytes of the message
const int NTP_PACKET_SIZE = 48;

//buffer to hold incoming and outgoing packets
byte packetBuffer[NTP_PACKET_SIZE];

// A UDP instance to let us send and receive packets over UDP
FishinoUDP Udp;

void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSID della rete:
	Serial.print("SSID: ");
	Serial.println(Fishino.SSID());

	// print your WiFi shield's IP address:
	// stampa l'indirizzo IP della rete:
	IPAddress ip = Fishino.localIP();
	Serial << F("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	// stampa la potenza del segnale di rete:
	long rssi = Fishino.RSSI();
	Serial << F("signal strength (RSSI):");
	Serial.print(rssi);
	Serial << F(" dBm\n");
}

// send an NTP request to the time server at the given address
// invia una richiesta al server NTP all'indirizzo fornito
unsigned long sendNTPpacket(IPAddress& address)
{
	// set all bytes in the buffer to 0
	// azzera il buffer di ricezione NTP
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	// Inizializza i valori da inviare al server NTP
	// (vedere URL del server per dettagli sul formato pacchetto)
	
	// LI, Version, Mode
	packetBuffer[0] = 0b11100011;
	
	// Stratum, or type of clock
	packetBuffer[1] = 0;
	
	// Polling Interval
	packetBuffer[2] = 6;
	
	// Peer Clock Precision
	packetBuffer[3] = 0xEC;
	
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	// tutti i campi del paccketto NTP sono stati impostati
	// è quindi possibile inviare il paccetto di richiesta di data/ora
	
	// NTP requests are to port 123
	// beginPacket() just opens the connection
	// invia la richiesta NTP alla porta 123
	// beginPacket() apre solo la connessione
	Udp.beginPacket(address, 123);
	
	// fill UDP buffer with packet data
	// riempie il buffer di invio UDP con i dati del pacchetto
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	
	// ends and send the UDP packet
	// termina ed invia il pacchetto UDP
	Udp.endPacket();
	
	return 0;
}

void setup()
{
	// Initialize serial and wait for port to open
	// Inizializza la porta seriale e ne attende l'apertura
	Serial.begin(115200);
	
	// only for Leonardo needed
	// necessario solo per la Leonardo
	while (!Serial)
		;

	// initialize SPI
	// inizializza il modulo SPI
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	
	// reset and test WiFi module
	// resetta e testa il modulo WiFi
	while(!Fishino.reset())
		Serial << F("Fishino RESET FAILED, RETRYING...\n");
	Serial << F("Fishino WiFi RESET OK\n");

	// go into station mode
	// imposta la modalità stazione
	Fishino.setMode(STATION_MODE);

	// try forever to connect to AP
	// tenta la connessione finchè non riesce
	Serial << F("Connecting to AP...");
	while(!Fishino.begin(MY_SSID, MY_PASS))
	{
		Serial << ".";
		delay(2000);
	}
	Serial << "OK\n";
	
	
	// setup IP or start DHCP client
	// imposta l'IP statico oppure avvia il client DHCP
#ifdef IPADDR
	Fishino.config(ip);
#else
	Fishino.staStartDHCP();
#endif

	// wait till connection is established
	Serial << F("Waiting for IP...");
	while(Fishino.status() != STATION_GOT_IP)
	{
		Serial << ".";
		delay(500);
	}
	Serial << "OK\n";
	
	// print connection status on serial port
	// stampa lo stato della connessione sulla porta seriale
	printWifiStatus();

	Serial << F("Starting connection to server...\n");
	Udp.begin(localPort);
}

void loop()
{
	// send an NTP packet to a time server
	Serial << F("Sending UDP request...");
	sendNTPpacket(timeServer);
	Serial << "OK\n";
	
	
	// wait to see if a reply is available
	delay(1000);
	
	while(Udp.parsePacket())
	{
		Serial << F("Packet received\n");
		
		// print remote port and IP of incoming packet, just to show them
		// stampa IP e porta remoti per mostrare la provenienza del pacchetto
		IPAddress remoteIp = Udp.remoteIP();
		uint32_t remotePort = Udp.remotePort();
		Serial << F("Remote IP   : ") << remoteIp << "\n";
		Serial << F("Remote port : ") << remotePort << "\n";
		
		// We've received a packet, read the data from it and put into a buffer
		// abbiamo ricevuto un pacchetto, leggiamo i dati ed inseriamoli in un buffer
		Udp.read(packetBuffer, NTP_PACKET_SIZE);

		// the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, extract the two words:
		// il timestamp inizia dal byte 40 del pacchetto ricevuto, e consiste in 4 bytes
		// o due words, long. Innanzitutto estraiamo le due words
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		// combiniamo i 4 bytes (o 2 words) in un long integer
		// che è il tempo NTP (secondi dal primo Gennaio 1900)
		unsigned long secsSince1900 = highWord << 16 | lowWord;
		Serial << F("Seconds since Jan 1 1900 = ") << secsSince1900 << "\n";

		// now convert NTP time into everyday time
		// ora convertiamo il tempo NTP in formato leggibile
		Serial << F("Unix time = ");
		
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800
		// il tempo Unix inizia dal primo Gennaio 1970. In secondi, sono 2208988800
		const unsigned long seventyYears = 2208988800UL;
		
		// subtract seventy years
		// sottrae dal tempo NTP la base Unix
		unsigned long epoch = secsSince1900 - seventyYears;
		
		// print Unix time:
		// stampa il tempo Unix
		Serial << epoch << "\n";

		// print the hour, minute and second
		// stampa ora, minuti e secondi
		
		// UTC is the time at Greenwich Meridian (GMT)
		// Tempo UTC (ora al meridiano di Greenwich, GMT)
		Serial << F("The UTC time is ");
		
		// print the hour (86400 equals secs per day)
		// stampa l'ora (contando 86400 secondi al giorno
		Serial << ((epoch  % 86400L) / 3600);
		Serial.print(':');
		if (((epoch % 3600) / 60) < 10)
		{
			// In the first 10 minutes of each hour, we'll want a leading '0'
			// nei primi 10 minuti di ogni ora vogiamo uno zero iniziale
			Serial << '0';
		}
		
		// print the minute (3600 equals secs per minute)
		// stampa i minuti (contando 3600 secondi per minuto)
		Serial << ((epoch  % 3600) / 60);
		Serial << ':';
		if ((epoch % 60) < 10)
		{
			// In the first 10 seconds of each minute, we'll want a leading '0'
			// nei primi 10 secondi di ogni minuto vogliamo lo zero iniziale
			Serial << '0';
		}
		// print the second
		// stampa i secondi
		Serial << epoch % 60 << "\n";
	}
	
	// wait ten seconds before asking for the time again
	// attende 10 secondi prima di effettuare una nuova richiesta
	delay(10000);
}
