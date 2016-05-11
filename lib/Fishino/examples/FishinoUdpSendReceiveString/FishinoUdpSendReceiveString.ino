
/*
	WiFi UDP Send and Receive String
	
	This sketch wait an UDP packet on localPort using a WiFi shield.
	When a packet is received an Acknowledge packet is sent to the client on port remotePort
	
	created 30 December 2012
	by dlf (Metodo2 srl)
	
	Adapted to Fishino by Massimo Del Fedele on 15/02/2016

 */

#include <Flash.h>
#include <FishinoUdp.h>
#include <FishinoSockBuf.h>
#include <Fishino.h>
#include <SPI.h>

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

// local port to listen on
// porta UDP sulla quale Fishino attende i pacchetti
unsigned int localPort = 2390;


//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

// define ip address if required
// NOTE : if your network is not of type 255.255.255.0 or your gateway is not xx.xx.xx.1
// you should set also both netmask and gateway
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

// buffer to hold incoming packet
// buffer per contenere i pacchetti ricevuti
char packetBuffer[255];

// a string to send back
// una stringa di conferma da spedire al mittente
char  ReplyBuffer[] = "acknowledged";

// the UDP client/server
// il client/server UDP
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

// setup code
// codice di inizializzazione
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

	// starts listening on local port
	// inizia l'ascolto dei pacchetti UDP alla porta specificata
	Serial << F("Starting connection to server...\n");
	Udp.begin(localPort);
}

void loop()
{

	// if there's data available, read a packet
	// se ci sono dati in arrivo, li stampa
	int packetSize = Udp.parsePacket();
	if (packetSize)
	{
		Serial << F("Received packet of size ");
		Serial << packetSize << "\n";
		Serial << F("From ");
		IPAddress remoteIp = Udp.remoteIP();
		Serial << remoteIp;
		Serial << F(", port ");
		Serial << Udp.remotePort() << "\n";

		// read the packet into packetBufffer
		// legge il pacchetto nel buffer
		int len = Udp.read(packetBuffer, 255);
		if (len > 0)
			packetBuffer[len] = 0;

		Serial << F("Contents:\n");
		Serial.println(packetBuffer);

		// send a reply, to the IP address and port that sent us the packet we received
		// invia una risposta all'indirizzo IP e alla porta da cui proviene la richiesta
		Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
		Udp.write(ReplyBuffer);
		Udp.endPacket();
	}
}
