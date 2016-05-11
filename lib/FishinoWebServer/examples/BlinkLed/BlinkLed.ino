// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: December 2010
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//
// 06-FEB-2016 Adapted to Fishino by Massimo Del Fedele

#include <pins_arduino.h>
#include <SPI.h>
#include <Fishino.h>
#include <Flash.h>
#include <SD.h>
#include <FishinoWebServer.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION DATA		-- ADAPT TO YOUR NETWORK !!!
// DATI DI CONFIGURAZIONE	-- ADATTARE ALLA PROPRIA RETE WiFi !!!

// OPERATION MODE :
// NORMAL (STATION)	-- NEEDS AN ACCESS POINT/ROUTER
// STANDALONE (AP)	-- BUILDS THE WIFI INFRASTRUCTURE ON FISHINO
// COMMENT OR UNCOMMENT FOLLOWING #define DEPENDING ON MODE YOU WANT
// MODO DI OPERAZIONE :
// NORMAL (STATION)	-- HA BISOGNO DI UNA RETE WIFI ESISTENTE A CUI CONNETTERSI
// STANDALONE (AP)	-- REALIZZA UNA RETE WIFI SUL FISHINO
// COMMENTARE O DE-COMMENTARE LA #define SEGUENTE A SECONDA DELLA MODALITÀ RICHIESTA
// #define STANDALONE_MODE

// here pur SSID of your network
// inserire qui lo SSID della rete WiFi
#define MY_SSID	""

// here put PASSWORD of your network. Use "" if none
// inserire qui la PASSWORD della rete WiFi -- Usare "" se la rete non ￨ protetta
#define MY_PASS	""

// here put required IP address of your Fishino
// comment out this line if you want AUTO IP (dhcp)
// NOTES :
//		for STATION_MODE if you use auto IP you must find it somehow !
//		for AP_MODE you MUST choose a free address range
// 
// inserire qui l'IP desiderato per il fishino
// commentare la linea sotto se si vuole l'IP automatico
// NOTE :
//		nella modalità STATION_MOD se si utilizza l'IP automatico, occorre un metodo per trovarlo !
//		nella modalità AP_MODE occorre scegliere un range di IP NON utilizzato da altre reti presenti
#define IPADDR	192, 168, 1, 251

// NOTE : for prototype green version owners, set SD_CS to 3 !!!
// NOTA : per i possessori del prototipo verde di Fishino, impostare SD_CS a 3 !!!
const int SD_CS = 4;

// LED I/O on Fishino Board
// L'I/O del led connesso alla scheda Fishino
const int LEDPIN = 7;

// END OF CONFIGURATION DATA
// FINE DATI DI CONFIGURAZIONE
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// define ip address if required
#ifdef IPADDR
IPAddress ip(IPADDR);
#endif

// The initial state of the LED
int ledState = LOW;

void setLedEnabled(boolean state)
{
	ledState = state;
	digitalWrite(LEDPIN, ledState);
}

inline boolean getLedState()
{
	return ledState;
}

boolean file_handler(FishinoWebServer& web_server);
boolean blink_led_handler(FishinoWebServer& web_server);
boolean led_status_handler(FishinoWebServer& web_server);
boolean index_handler(FishinoWebServer& web_server);

FishinoWebServer::PathHandler handlers[] =
{
	// Work around Arduino's IDE preprocessor bug in handling /* inside
	// strings.
	//
	// `put_handler' is defined in TinyWebServer
	{"/", FishinoWebServer::GET, &index_handler },
	{"/upload/" "*", FishinoWebServer::PUT, &FishinoWebPutHandler::put_handler },
	{"/blinkled", FishinoWebServer::POST, &blink_led_handler },
	{"/ledstatus" "*", FishinoWebServer::GET, &led_status_handler },
	{"/" "*", FishinoWebServer::GET, &file_handler },
	{NULL},
};

const char* headers[] =
{
	"Content-Length",
	NULL
};

FishinoWebServer web = FishinoWebServer(handlers, headers);

boolean has_filesystem = true;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

void send_file_name(FishinoWebServer& web_server, const char* filename)
{
	if (!filename)
	{
		web_server.send_error_code(404);
		web_server << F("Could not parse URL");
	}
	else
	{
		FishinoWebServer::MimeType mime_type
		= FishinoWebServer::get_mime_type_from_filename(filename);
		web_server.send_error_code(200);
		web_server.send_content_type(mime_type);
		web_server.end_headers();
		if (file.open(&root, filename, O_READ))
		{
			Serial << F("Read file ");
			Serial.println(filename);
			web_server.send_file(file);
			file.close();
		}
		else
		{
			web_server << F("Could not find file: ") << filename << "\n";
		}
	}
}

boolean file_handler(FishinoWebServer& web_server)
{
	char* filename = FishinoWebServer::get_file_from_path(web_server.get_path());
	send_file_name(web_server, filename);
	free(filename);
	return true;
}

boolean blink_led_handler(FishinoWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/plain");
	web_server.end_headers();
	// Reverse the state of the LED.
	setLedEnabled(!getLedState());
	Client& client = web_server.get_client();
	if (client.available())
	{
		char ch = (char)client.read();
		if (ch == '0')
		{
			setLedEnabled(false);
		}
		else if (ch == '1')
		{
			setLedEnabled(true);
		}
	}
	return true;
}

boolean led_status_handler(FishinoWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/plain");
	web_server.end_headers();
	Client& client = web_server.get_client();
	client.println(getLedState(), DEC);
	return true;
}

boolean index_handler(FishinoWebServer& web_server)
{
	send_file_name(web_server, "INDEX.HTM");
	return true;
}

void file_uploader_handler(FishinoWebServer& web_server,
		FishinoWebPutHandler::PutAction action,
		char* buffer, int size)
{
	static uint32_t start_time;
	static uint32_t total_size;

	switch (action)
	{
		case FishinoWebPutHandler::START:
			start_time = millis();
			total_size = 0;
			if (!file.isOpen())
			{
				// File is not opened, create it. First obtain the desired name
				// from the request path.
				char* fname = web_server.get_file_from_path(web_server.get_path());
				if (fname)
				{
					Serial << F("Creating ") << fname << "\n";
					file.open(&root, fname, O_CREAT | O_WRITE | O_TRUNC);
					free(fname);
				}
			}
			break;

		case FishinoWebPutHandler::WRITE:
			if (file.isOpen())
			{
				file.write(buffer, size);
				total_size += size;
			}
			break;

		case FishinoWebPutHandler::END:
			file.sync();
			Serial << F("Wrote ") << file.fileSize() << F(" bytes in ")
			<< millis() - start_time << F(" millis (received ")
			<< total_size << F(" bytes)\n");
			file.close();
	}
}

void setup()
{
	Serial.begin(115200);
	Serial << F("Free RAM: ") << FreeRam() << "\r\n";

	// initialize SPI:
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	
	// reset and test wifi module
	Serial << F("Resetting Fishino...");
	while(!Fishino.reset())
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\r\n");
	
	// set PHY mode 11B
	Fishino.setPhyMode(PHY_MODE_11B);

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
	Fishino.softApConfig(F(MY_SSID), F(MY_PASS), 1, false);
	
	// restart DHCP server
	Serial << F("Starting DHCP server\r\n");
	Fishino.softApStartDHCPServer();
	
	// print current IP address
	Serial << F("IP Address :") << ip << "\r\n";

#else
	// setup STATION mode
	// imposta la modalitè STATION
	Serial << F("Setting mode STATION_MODE\r\n");
	Fishino.setMode(STATION_MODE);

	// NOTE : INSERT HERE YOUR WIFI CONNECTION PARAMETERS !!!!!!
	Serial << F("Connecting AP...");
	while(!Fishino.begin(F(MY_SSID), F(MY_PASS)))
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
	Serial << "Waiting for IP...";
	while(Fishino.status() != STATION_GOT_IP)
	{
		Serial << ".";
		delay(500);
	}
	Serial << F("OK\r\n");

	// print current IP address
	Serial << F("IP Address :") << Fishino.localIP() << "\r\n";

#endif

	pinMode(LEDPIN, OUTPUT);
	setLedEnabled(false);

	// initialize the SD card.
	Serial << F("Setting up SD card...\n");
	// Pass over the speed and Chip select for the SD card
	if (!card.init(SPI_FULL_SPEED, SD_CS))
	{
		Serial << F("card failed\n");
		has_filesystem = false;
	}
	// initialize a FAT volume.
	if (!volume.init(&card))
	{
		Serial << F("vol.init failed!\n");
		has_filesystem = false;
	}
	if (!root.openRoot(&volume))
	{
		Serial << F("openRoot failed");
		has_filesystem = false;
	}

	if (has_filesystem)
	{
		// Assign our function to `upload_handler_fn'.
		FishinoWebPutHandler::put_handler_fn = file_uploader_handler;
	}

	// Start the web server.
	Serial << F("Web server starting...\n");
	web.begin();

	Serial << F("Ready to accept HTTP requests.\n");
}

void loop()
{
	if (has_filesystem)
	{
		web.process();
	}
}
