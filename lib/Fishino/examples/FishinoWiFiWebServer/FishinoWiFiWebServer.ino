/*
  WiFi Web Server

 A simple web server that shows the value of the analog input pins.
 using a WiFi shield.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * WiFi shield attached
 * Analog inputs attached to pins A0 through A5 (optional)

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 adapted to Fishino 16 Ago 2015
 by Massimo Del Fedele

 */

#include <Flash.h>
#include <FishinoUdp.h>
#include <FishinoSockBuf.h>
#include <Fishino.h>
#include <SPI.h>

#include <SD.h>

///////////////////////////////////////////////////////////////////////
//           CONFIGURATION DATA -- ADAPT TO YOUR NETWORK             //
//     CONFIGURAZIONE SKETCH -- ADATTARE ALLA PROPRIA RETE WiFi      //

// OPERATION MODE :
// NORMAL (STATION)	-- NEEDS AN ACCESS POINT/ROUTER
// STANDALONE (AP)	-- BUILDS THE WIFI INFRASTRUCTURE ON FISHINO
// COMMENT OR UNCOMMENT FOLLOWING #define DEPENDING ON MODE YOU WANT
// MODO DI OPERAZIONE :
// NORMAL (STATION)	-- HA BISOGNO DI UNA RETE WIFI ESISTENTE A CUI CONNETTERSI
// STANDALONE (AP)	-- REALIZZA UNA RETE WIFI SUL FISHINO
// COMMENTARE O DE-COMMENTARE LA #define SEGUENTE A SECONDA DELLA MODALITÀ RICHIESTA
//#define STANDALONE_MODE

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non ￨ protetta
#define MY_PASS	""

// comment this line if you want a dynamic IP through DHCP
// obtained IP will be printed on serial port monitor
// commentare la linea seguente per avere un IP dinamico tramite DHCP
// l'IP ottenuto verrà visualizzato sul monitor seriale
#define IPADDR 192,168,1,251

//                    END OF CONFIGURATION DATA                      //
//                       FINE CONFIGURAZIONE                         //
///////////////////////////////////////////////////////////////////////

// define ip address if required
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

FishinoServer server(80);

void printWifiStatus()
{
	// print the SSID of the network you're attached to:
	// stampa lo SSDI della rete a cui si è connessi
	Serial.print("SSID: ");
	Serial.println(Fishino.SSID());

	// get phy mode and show it
	uint8_t mode = Fishino.getPhyMode();
	Serial.print("PHY MODE: (");
	Serial.print(mode);
	Serial.print(") ");
	switch(mode)
	{
		case PHY_MODE_11B:
			Serial.println("11B");
			break;

		case PHY_MODE_11G:
			Serial.println("11G");
			break;

		case PHY_MODE_11N:
			Serial.println("11N");
			break;
			
		default:
			Serial.println("UNKNOWN");
	}
	
#ifdef STANDALONE_MODE
	// get AP IP info
	IPAddress ip, gw, nm;
	if(Fishino.getApIPInfo(ip, gw, nm))
	{
		Serial << F("Fishino IP      :") << ip << "\r\n";
		Serial << F("Fishino GATEWAY :") << gw << "\r\n";
		Serial << F("Fishino NETMASK :") << nm << "\r\n";
	}
	else
		Serial << F("Couldn't get Fishino IP info\r\n");
#else
	// print your Fishino's IP address:
	// stampa l'indirizzo IP del Fishino
	IPAddress ip = Fishino.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	// stampa la qualità del segnale WiFi
	Serial.print("signal strength (RSSI):");
	Serial.print(Fishino.RSSI());
	Serial.println(" dBm");
#endif

}

void setup()
{
	//Initialize serial and wait for port to open:
	// apre la porta seriale e ne attende l'apertura
	Serial.begin(115200);

	// wait for serial port to connect. Needed for Leonardo only
	// attende l'apertura della porta seriale. Necessario solo per le boards Leonardo
	while (!Serial)
		;

	// initialize SPI
	// inizializza il modulo SPI
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	
	// reset and test wifi module
	// resetta e testa il modulo WiFi
	Serial << F("Resetting Fishino...");
	while(!Fishino.reset())
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\r\n");

	Serial << F("Fishino WiFi web server\r\n");

	// set PHY mode to 11G
	Fishino.setPhyMode(PHY_MODE_11G);
	
	// for AP MODE, setup the AP parameters
#ifdef STANDALONE_MODE
	// setup SOFT AP mode
	// imposta la modalitè SOFTAP
	Serial << F("Setting mode SOFTAP_MODE\r\n");
	Fishino.setMode(SOFTAP_MODE);

	// stop AP DHCP server
	Serial << F("Stopping DHCP server\r\n");
	Fishino.softApStopDHCPServer();
	
	// setup access point parameters
	// imposta i parametri dell'access point
	Serial << F("Setting AP IP info\r\n");
	Fishino.setApIPInfo(ip, ip, IPAddress(255, 255, 255, 0));

	Serial << F("Setting AP WiFi parameters\r\n");
	Fishino.softApConfig(MY_SSID, MY_PASS, 1, false);
	
	// restart DHCP server
	Serial << F("Starting DHCP server\r\n");
	Fishino.softApStartDHCPServer();
	
#else
	// setup STATION mode
	// imposta la modalitè STATION
	Serial << F("Setting mode STATION_MODE\r\n");
	Fishino.setMode(STATION_MODE);

	// NOTE : INSERT HERE YOUR WIFI CONNECTION PARAMETERS !!!!!!
	Serial << F("Connecting to AP...");
	while(!Fishino.begin(MY_SSID, MY_PASS))
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\r\n");

	// setup IP or start DHCP server
#ifdef IPADDR
	Fishino.config(ip);
#else
	Fishino.staStartDHCP();
#endif

	// wait for connection completion
	Serial << F("Waiting for IP...");
	while(Fishino.status() != STATION_GOT_IP)
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\r\n");

#endif

	// show connection status
	// visualizza lo stato della connessione
	printWifiStatus();

	// start listening for clients
	// inizia l'attesa delle connessioni
	server.begin();
}


void loop()
{
	// wait for a new client:
	// attende nuovi clienti
	FishinoClient client = server.available();

	if (client)
	{
		Serial.println("new client");
		
		// an http request ends with a blank line
		// ogni richiesta http termina con una linea vuota
		boolean currentLineIsBlank = true;
		while (client.connected())
		{
			if (client.available())
			{
				char c = client.read();
				Serial.write(c);
				
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				// se si è arrivati a fine linea (carattere 'newline' ricevuto
				// e la linea è vuota, la richiesa http è terminata
				// quindi è possibile inviera una risposta
				if (c == '\n' && currentLineIsBlank)
				{
					// send a standard http response header
					// invia uno header standard http
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");  // the connection will be closed after completion of the response
					client.println("Refresh: 5");  // refresh the page automatically every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					
					// output the value of each analog input pin
					// invia il valore di tutti i pins analogici
					for (int analogChannel = 0; analogChannel < 6; analogChannel++)
					{
						int sensorReading = analogRead(analogChannel);
						client.print("analog input ");
						client.print(analogChannel);
						client.print(" is ");
						client.print(sensorReading);
						client.println("<br />");
					}
					client.println("</html>");
					break;
				}
				if (c == '\n')
				{
					// you're starting a new line
					// inizio di una nuova linea
					currentLineIsBlank = true;
				}
				else if (c != '\r')
				{
					// you've gotten a character on the current line
					// sono stati ricevuti dei caratteri nella linea corrente
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		// lascia tempo al browser per ricevere i dati
		delay(1);

		// close the connection:
		// chiudi la connessione
		client.stop();
		Serial.println("client disonnected");
		Serial << F("Free RAM: ") << FreeRam() << "\n";
	}
}
