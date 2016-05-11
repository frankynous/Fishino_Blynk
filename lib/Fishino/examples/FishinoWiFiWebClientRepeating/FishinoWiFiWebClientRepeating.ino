/*
  Repeating Wifi Web Client

 This sketch connects to a a web server and makes a request
 using an Arduino Wifi shield.

 Circuit:
 * WiFi shield attached to pins SPI pins and pin 7

 created 23 April 2012
 modified 31 May 2012
 by Tom Igoe
 modified 13 Jan 2014
 by Federico Vanzati

 http://arduino.cc/en/Tutorial/WifiWebClientRepeating
 This code is in the public domain.

 adapted to Fishino 16 Ago 2015
 by Massimo Del Fedele

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


//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

// define ip address if required
// NOTE : if your network is not of type 255.255.255.0 or your gateway is not xx.xx.xx.1
// you should set also both netmask and gateway
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

// Initialize the Fishino client library
FishinoClient client;

// server address:
char server[] = "www.arduino.cc";

// last time you connected to the server, in milliseconds
// l'ultima volta che vi siete connessi al server, in millisecondi
unsigned long lastConnectionTime = 0;

// delay between updates, in milliseconds
// ritardo tra gli aggiornamenti, in millisecondi
const unsigned long postingInterval = 2L * 1000L;

// this method makes a HTTP connection to the server:
void httpRequest()
{
	// close any connection before send a new request.
	// This will free the socket on the WiFi module
	// chiude ogni ventuale connessione prima di inviare una nuova richiesta
	// quest libera il socket sull modulo WiFi
	client.stop();

	// if there's a successful connection:
	// se la connessione è riuscita:
	if (client.connect(server, 80))
	{
		Serial << F("connecting...\n");
		
		// send the HTTP PUT request:
		// invia la richiesta HTTP:
		client << F("GET /latest.txt HTTP/1.1\r\n");
		client << F("Host: www.veneto.com\r\n");
		client << F("User-Agent: FishinoWiFi/1.1\r\n");
		client << F("Connection: close\r\n");
		client.println();
	}
	else
	{
		// if you couldn't make a connection:
		// se la connessione non è riuscita:
		Serial << F("connection failed\n");
	}

	// note the time that the connection was made:
	// registra il tempo in cui è stata fatta la connessione
	lastConnectionTime = millis();
}

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
}

void loop()
{
	// if there's incoming data from the net connection.
	// send it out the serial port.  This is for debugging purposes only
	// se ci sono dati provenienti dalla rete
	// li invia alla porta seriale. Questo solo a scopo di debugging
	while (client.available())
	{
		char c = client.read();
		Serial.write(c);
	}

	// if ten seconds have passed since your last connection,
	// then connect again and send data
	// se son passati 10 secondi dall'ultima connessione
	// riconnetti e invia i dati
	if (millis() - lastConnectionTime > postingInterval)
	{
		httpRequest();
	}
}
