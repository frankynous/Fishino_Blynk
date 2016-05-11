/*

 This example  prints the Wifi shield's MAC address, and
 scans for available Wifi networks using the Wifi shield.
 Every ten seconds, it scans again. It doesn't actually
 connect to any network, so no encryption scheme is specified.

 Circuit:
 * WiFi shield attached

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 21 Junn 2012
 by Tom Igoe and Jaymes Dec

 adapted to Fishino 16 Ago 2015
 by Massimo Del Fedele

 */

#include <Flash.h>
#include <FishinoUdp.h>
#include <FishinoSockBuf.h>
#include <Fishino.h>
#include <SPI.h>

void printMacAddress()
{

	// print your MAC address:
	byte const *mac = Fishino.macAddress();
	Serial.print("MAC: ");
	Serial.print(mac[5], HEX);
	Serial.print(":");
	Serial.print(mac[4], HEX);
	Serial.print(":");
	Serial.print(mac[3], HEX);
	Serial.print(":");
	Serial.print(mac[2], HEX);
	Serial.print(":");
	Serial.print(mac[1], HEX);
	Serial.print(":");
	Serial.println(mac[0], HEX);
}

void printEncryptionType(int thisType)
{
	// read the encryption type and print out the name:
	switch (thisType)
	{
		case AUTH_WEP:
			Serial.println("WEP");
			break;
		case AUTH_WPA_PSK:
			Serial.println("WPA");
			break;
		case AUTH_WPA2_PSK:
			Serial.println("WPA2");
			break;
		case AUTH_WPA_WPA2_PSK:
			Serial.println("WPA-WPA2");
			break;
		case AUTH_OPEN:
			Serial.println("None");
			break;
		default:
			Serial.println("Unknown");
			break;
	}
}

void listNetworks()
{
	// scan for nearby networks:
	Serial.println("** Scan Networks **");
	int numSsid = Fishino.scanNetworks();
	if (numSsid == -1)
	{
		Serial.println("Couldn't get a wifi connection");
		while (true)
			;
	}

	// print the list of networks seen:
	Serial.print("number of available networks:");
	Serial.println(numSsid);

	// print the network number and name for each network found:
	for (int thisNet = 0; thisNet < numSsid; thisNet++)
	{
		Serial.print(thisNet);
		Serial.print(") ");
		Serial.print(Fishino.SSID(thisNet));
		Serial.print("\tSignal: ");
		Serial.print(Fishino.RSSI(thisNet));
		Serial.print(" dBm");
		Serial.print("\tEncryption: ");
		printEncryptionType(Fishino.encryptionType(thisNet));
	}
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

	// Print WiFi MAC address:
	printMacAddress();

}

void loop()
{
	// scan for existing networks:
	Serial.println("Scanning available networks...");
	listNetworks();

	delay(10000);
}




