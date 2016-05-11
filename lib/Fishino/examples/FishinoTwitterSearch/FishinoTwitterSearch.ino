/*
  Twitter search sample sketch

 This sketch uses Twitter APIs to search for twitters using Fishino's https capabilities.
 It search the hashtag "#fishino" on Twitter network and displays results on serial port

 Circuit:
 * Fishino

 created 23 September 2015
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
#define IPADDR	192, 168, 1, 251

// here put your application Bearer authorization key
// see https://dev.twitter.com/oauth/application-only for details
// inserire qui la chiave "bearer" di autenticazione dell'applicazione
// vedere https://dev.twitter.com/oauth/application-only per dettagli
#define BEARER ""


//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

// define ip address if required
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

// Initialize the Fishino client library
FishinoSecureClient client;


void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSID della rete:
	Serial.print("SSID: ");
	Serial.println(Fishino.SSID());

	// print your WiFi shield's IP address:
	// stampa l'indirizzo IP della rete:
	IPAddress ip = Fishino.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	// stampa la potenza del segnale di rete:
	long rssi = Fishino.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.print(rssi);
	Serial.println(" dBm");
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
		Serial << F("Fishino RESET FAILED, RETRYING....\n");
	Serial << F("Fishino WiFi RESET OK\n");

	// go into station mode
	// imposta la modalità stazione
	Fishino.setMode(STATION_MODE);

	// Initialize the Wifi connection.
	// Inizializza la connessione WiFi
	Serial << F("Setting up the Wifi connection...\n");

	// try forever to connect to AP
	// tenta la connessione finchè non riesce
	while(Fishino.begin(MY_SSID, MY_PASS))
		Serial << F("ERROR CONNECTING TO AP, RETRYING....\n");
	Serial << F("SUCCESFULLY CONNECTED TO AP\n");
	
	// setup IP or start DHCP client
	// imposta l'IP statico oppure avvia il client DHCP
#ifdef IPADDR
	Fishino.config(ip);
#else
	Fishino.staStartDHCP();
#endif

	// print connection status on serial port
	// stampa lo stato della connessione sulla porta seriale
	printWifiStatus();
}

void loop()
{
	// if client is not connected and there are no data still to process....
	// se il client non è connesso e se non sono rimasti dati da processare....
	if (!client.connected() && !client.available())
	{
		Serial.println("\nStarting connection to server...");
		if(client.connect("api.twitter.com", 443))
		{
			Serial.println("connected to server");

			// Make a HTTP request:
			// this request search Twitter network for #fishino hashtag and displays data for LAST tweet
			// Esegue la request HTTP:
			// questo ricerca l'hashtag #fishino nella rete e mostra i dati dell'ULTIMO tweet trovato
//			client.println("GET /1.1/search/tweets.json?count=1&since_id=646038066440896517&q=%23fishino&result_type=recent HTTP/1.1");
			client.println("GET /1.1/search/tweets.json?count=1&q=%23fishino&result_type=recent HTTP/1.1");
			
			client.println("Host: api.twitter.com");
			client.println("User-Agent: Fishino Twitter Client");

			// send auth key to server
			// invia la chiave di autorizzazione al server
			client.println("Authorization: Bearer " BEARER);
			client.println();
		}
	}
	
	// wait 1 second max for data from server
	// attende al massimo 1 secondo i dati dal server
	unsigned long tim = millis() + 1000;
	while(!client.available() && millis() < tim)
		;
	
	// if no data is available, close the connection
	// se non ci sono dati, chiude la connessione
	if(!client.available())
		client.stop();
	
	// if there are incoming bytes available from the server, read them and print them:
	// se ci sono bytes dal server, li legge e li stamp sulla seriale
	while(client.available())
	{
		char c = client.read();
		Serial.write(c);
	}
	
	// wait 1 second between requests
	// attende 1 secondo tra le richieste
	delay(1000);
}
