
/**
 *   Projet pilotge de radiateur par fil pilote avec interface web, domoticz, MQTT
 *   ESP8266 + DS12B20 + LED + MQTT + Home-Assistant
 *   Projets DIY (https://www.projetsdiy.fr) - Mai 2016
 *   Licence : MIT
 **/

// Include the libraries we need
// #include <Arduino.h>

#include "../lib/config.h"
#include "../lib/DomoticzConfig.h"

#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <ESP8266mDNS.h> // https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS
// #include <ESPAsyncTCP.h> // Dependance avec ESPAsyncWebServer.h

#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#warning						 // #define WEBSERVER_H Indispensable pour valider la compilation entre WiFiManager.h et ESPAsyncWebServer.h
#define WEBSERVER_H
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
// #include <ESP8266WebServer.h>
// #include <WiFiClient.h>
// #include <DNSServer.h>

// this needs to be first, or it all crashes and burns...
#include "FS.h"
#include <LittleFS.h>	  // https://github.com/esp8266/Arduino/tree/master/libraries/LittleFS
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#ifdef OTA
#include <ElegantOTA.h> //https://github.com/ayushsharma82/ElegantOTA
#endif

#ifdef ISR_TIMER
#include <ESP8266TimerInterrupt.h> //https://github.com/khoih-prog/ESP8266TimerInterrupt
#include <ESP8266_ISR_Timer.h>	  // https://github.com/khoih-prog/ESP8266TimerInterrupt
#endif

// #include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>

//----------------------------------------------------------------- PIN SETTINGS
#ifdef LED_BUILTIN
#undef LED_BUILTIN
#define LED_BUILTIN 2
#endif
#define PIN_FILPILOTE_PLUS 12	 // N° de Pin fil pilote
#define PIN_FILPILOTE_MOINS 13 // N° de Pin fil pilote
#define PIN_ACS712 A0			 // Mesure de courant
#define PIN_ONE_WIRE_BUS 14	 // Mesure de température

/*
variable gestion de boucle
*/
const int watchdog = 300000;				  // Fréquence d'envoi des données à Domoticz 5min
unsigned long previousMillis = millis(); // mémoire pour envoi des données
boolean firstLoop = false;
unsigned long i = 0;
volatile uint32_t startMillis = 0; // mesure du temps des interruption timerISR
#ifdef OTA
long ota_progress_millis = 0;
#endif

// Définition PIN DATA du bus oneWire
OneWire oneWire(PIN_ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

#ifdef ISR_TIMER
// Création tache tempo pour mode confort 1 et 2
ESP8266Timer ITimer;			  // Init ESP8266 timer 1
ESP8266_ISR_Timer ISR_Timer; // Init ESP8266_ISR_Timer
#endif

Ticker tickerSetHigh;
Ticker tickerSetLow;

#ifdef MQTT
// define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = MQTT_SERVER;
char mqtt_port[6] = MQTT_PORT;

// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
#endif

// Création objet WiFiManager
// Intialisation locale.Une fois son entreprise terminée, il n'est pas nécessaire de le garder
WiFiManager wifiManager;

// Création server pour webServer et Elegant OTA
AsyncWebServer server(80);

WiFiClient espClient;

// MQTT
#ifdef MQTT
// flag for saving data
bool shouldSaveConfigMQTT = true;
PubSubClient clientMQTT(espClient);
#endif

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void callback(char *topic, byte *payload, unsigned int length);
void saveConfigCallback();
void reconnect(const char *id, const char *topic); // void reconnect();
#ifdef MQTT
void askMqttToDomoticz(int idx, String svalue, const char *topic);
void sendMqttToDomoticz(int idx, String svalue, const char *topic);
#endif
void updateFilpilote(int pinPlus, int pinMoins, int svalue, int idx);
void confortSetPin(int aPinHigh, int aPinLow, float aTempoHigh, float aTempoLow);
void setPinConfort(int state);
void confortStopTask();
float retourSensor(DallasTemperature sensor);
float valeurACS712(int pin);
#ifdef OTA
void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);
#endif
#ifdef ISR_TIMER
void IRAM_ATTR TimerHandler();
#endif
void printStatus(uint16_t index, unsigned long timerDelay, unsigned long deltaMillis, unsigned long currentMillis);
void doingSomethingSec();

void setup()
{
	//----------------------------------------------------------------- Serial
	Serial.begin(115200);
	while (!Serial)
	{
	};
	Serial.println("Port Série OK");
	Serial.println("\n");
	Serial.print("ESP Board MAC Address: ");
	Serial.println(WiFi.macAddress());

	//----------------------------------------------------------------- GPIO
	pinMode(PIN_FILPILOTE_PLUS, OUTPUT);
	pinMode(PIN_FILPILOTE_MOINS, OUTPUT);

	// Positionne en mode Hors Gel à la mise sous tension
	digitalWrite(PIN_FILPILOTE_PLUS, HIGH);
	digitalWrite(PIN_FILPILOTE_MOINS, HIGH);

	// Start DS18b20
	DS18B20.begin();

	//----------------------------------------------------------------- SPIFFS

	// LittleFS.format(); // clean FS, for testing

	// Lire la configuration JSON avec LittleFS
	Serial.println("\nmontage LittleFS...\n");

	if (!LittleFS.begin())
	{
		Serial.println("Erreur SPIFFS....\n");
		return;
	}

	FSInfo fs_info; // création structure d’information du système de fichiers LittleFS

	// Lecture et afficage des infos du système de fichiers LittleFS
	LittleFS.info(fs_info);
	Serial.printf("totalBytes: %i octets\n", fs_info.totalBytes);
	Serial.printf("usedBytes: %i octets\n", fs_info.usedBytes);
	Serial.printf("blockSize: %i octets\n", fs_info.blockSize);
	Serial.printf("pageSize: %i octets\n", fs_info.pageSize);
	Serial.printf("maxOpenFiles: %i octets\n", fs_info.maxOpenFiles);
	Serial.printf("maxPathLength: %i octets\n\n", fs_info.maxPathLength);

	// Affiche les fichiers présent en SPIFFS
	File root = LittleFS.open("/", "r");
	File file = root.openNextFile();

	unsigned long totalSize = 0;

	while (file)
	{
		Serial.print("File: ");
		Serial.print(file.name());
		Serial.print(" ");
		Serial.println(file.size());
		totalSize += file.size();
		file.close();
		file = root.openNextFile();
	}

	root.close();
	file.close();
	root = LittleFS.open("/css/", "r");
	file = root.openNextFile();

	while (file)
	{
		Serial.print("File: ");
		Serial.print(file.name());
		Serial.print(" ");
		Serial.println(file.size());
		totalSize += file.size();
		file.close();
		file = root.openNextFile();
	}

	root.close();
	file.close();
	root = LittleFS.open("/script/", "r");
	file = root.openNextFile();

	while (file)
	{
		Serial.print("File: ");
		Serial.print(file.name());
		Serial.print(" ");
		Serial.println(file.size());
		totalSize += file.size();
		file.close();
		file = root.openNextFile();
	}

	Serial.print("\nTotal taille fichiers: ");
	Serial.print(totalSize);
	Serial.println(" octets\n");

	//----------------------------------------------------------------- JSON
	if (LittleFS.exists("/config.json"))
	{
		// le fichier existe, lecture et chargement
		Serial.println("\nLecture du fichier de configuration");
		File configFile = LittleFS.open("/config.json", "r");

		if (configFile.isFile())
		{
			Serial.println("\nContenu du fichier: ");
			while (configFile.available())
			{
				Serial.write(configFile.read());
			}
			Serial.println("\nOuverture fichier de configuration");
			// Serial.println(configFile.position());
			configFile.seek(0, SeekSet); // Positionne le pointeur dans le fichier à 0
			// Serial.println(configFile.position());
			size_t size = configFile.size();
			Serial.print("Taille du fichier :");
			Serial.print(size);
			Serial.println(" octets");

			if (size > 1024)
			{
				Serial.println("Taille de fichier trop grande\n");
				return;
			}

			std::unique_ptr<char[]> buf(new char[size]);

			// int tmp = configFile.readBytes(buf.get(), size);
			// Serial.println(tmp);
			// Serial.println(buf.get());

			// Allocate the JSON document
			StaticJsonDocument<1024> doc;

			// Parse JSON object
			auto error = deserializeJson(doc, buf.get());
			if (error)
			{
				Serial.printf("\ndeserializeJson() failed: ");
				Serial.println(error.f_str());
				Serial.println(error.code());
				// return;
			}
#ifdef MQTT
			else
			{
				// Faire une boucle à travers tous les éléments du tableau
				JsonArray repos = doc["mqtt_server"]["mqtt_port"];

				for (JsonObject repo : repos)
				{
					// Imprimez le nom, le nombre d'étoiles et le nombre de problèmes
					Serial.println(repo["mqtt_server"].as<const char *>());
					Serial.println(repo["mqtt_port"].as<const char *>());
					Serial.println(repo["local_host"].as<const char *>());
					Serial.println(repo["mqtt_local_portport"].as<const char *>());
					Serial.println(repo["local_gateway"].as<const char *>());
					Serial.println(repo["local_subnet"].as<const char *>());
				}

				Serial.println("\nparsed doc");
				strcpy(mqtt_server, doc["mqtt_server"]);
				strcpy(mqtt_port, doc["mqtt_port"]);
			}
#endif
			configFile.close();
		}
	}
	// Fin de lecture

	//----------------------------------------------------------------- WIFI
	// Définir le rappel de notif
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	// Définir IP statique
	wifiManager.setSTAStaticIPConfig(IPAddress(LOCAL_IP), IPAddress(LOCAL_GATEWAY), IPAddress(LOCAL_SUBNET));

#ifdef MQTT
	// Ajoutez tous vos paramètres ici
	wifiManager.addParameter(&custom_mqtt_server);
	wifiManager.addParameter(&custom_mqtt_port);
#endif

	// reset settings - for testing
	// wifiManager.resetSettings();

	// set minimum quality of signal so it ignores AP's under that quality defaults to 8%
	// wifiManager.setMinimumSignalQuality();

	// sets timeout until configuration portal gets turned off
	// useful to make it all retry or go to sleep in seconds
	wifiManager.setTimeout(120);

	// fetches SSID and PASS and tries to connect
	// if it does not connect it starts an access point with the specified name
	// here  "AutoConnectAP"
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager.autoConnect(NAMEID, WM_PASSWORD))
	{
		Serial.println("failed to connect and hit timeout");
		delay(3000);
		// reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(5000);
	}

	// if you get here you have connected to the WiFi
	Serial.println("\tconnected...yeey :)\n");

	Serial.println("local IP: ");
	Serial.println(WiFi.localIP());

//----------------------------------------------------------------- Save Parametres dans CONFIG.JSON
#ifdef MQTT
	// read updated parameters
	strcpy(mqtt_server, custom_mqtt_server.getValue());
	strcpy(mqtt_port, custom_mqtt_port.getValue());
	Serial.println("The values in the file are: ");
	Serial.println("\tmqtt_server : " + String(mqtt_server));
	Serial.println("\tmqtt_port : " + String(mqtt_port));

	// save the custom parameters to LittleFS
	if (shouldSaveConfigMQTT)
	{
		Serial.println("Sauvegarde configuration");
		DynamicJsonDocument doc(1024);

		doc["mqtt_server"] = mqtt_server;
		doc["mqtt_port"] = mqtt_port;

		File configFile = LittleFS.open("/config.json", "w");
		if (!configFile)
		{
			Serial.println("Impossible d'ouvrir le fichier de configuration pour l'écriture");
		}

		serializeJson(doc, Serial);
		serializeJson(doc, configFile);

		configFile.close();
		// end save
	}

	//----------------------------------------------------------------- MQTT
	clientMQTT.setServer(mqtt_server, String(mqtt_port).toInt());
	clientMQTT.setCallback(callback);
#endif

	//----------------------------------------------------------------- Server
	// Envoi la page HTML
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
				 { request->send(LittleFS, "/index.html", "text/html"); });

	// Envoi la page HTML
	server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request)
				 { request->send(LittleFS, "/config.json", "text/json"); });

	// Envoi la page HTML
	server.on("/data.json", HTTP_GET, [](AsyncWebServerRequest *request)
				 { request->send(LittleFS, "/data.json", "text/json"); });

	// Envoi la page HTML
	server.on("/lireTemperature", HTTP_GET, [](AsyncWebServerRequest *request)
				 { request->send(200, "test/plain"); });

	// Envoi la page HTML
	server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
				 { digitalWrite(PIN_FILPILOTE_PLUS,HIGH);
				 request->send(200); });

	// Envoi la page HTML
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
				 { digitalWrite(PIN_FILPILOTE_PLUS,LOW);
				 request->send(200); });

	//----------------------------------------------------------------- Elegant OTA
#ifdef OTA
	ElegantOTA.begin(&server); // Start ElegantOTA
	// ElegantOTA callbacks
	ElegantOTA.onStart(onOTAStart);
	ElegantOTA.onProgress(onOTAProgress);
	ElegantOTA.onEnd(onOTAEnd);
#endif

	Serial.println("\nHTTP server setup.\nHTTP server started.\nServer ready!\n");
	server.begin();
}

void loop()
{
	//----------------------------------------------------------------- Elegant OTA
#ifdef OTA
	server.handleClient();
	ElegantOTA.loop();
	delay(500);
#endif

//----------------------------------------------------------------- MQTT
#ifdef MQTT
	// Ne pas oublié de mettre MQTT_MAX_PACKET_SIZE = 2048 dans PubSubClient.h
	if (!clientMQTT.connected())
	{
		Serial.println("reconnection domoticz/out");
		reconnect(MQTT_ID, TOPIC_DOMOTICZ_OUT);
		// reconnect();
	}
	clientMQTT.loop();

	unsigned long currentMillis = millis();

	// ajout d'un delais de 60s apres chaque trame envoyés pour éviter d'envoyer
	// en permanence des informations à domoticz et de créer des interférences
	if ((currentMillis - previousMillis > watchdog) || firstLoop == LOW)
	{
		previousMillis = currentMillis;
		if (!firstLoop)
		{ // Demande état d'un device
			askMqttToDomoticz(IDXDomoticz, "getdeviceinfo", TOPIC_DOMOTICZ_IN);
			firstLoop = HIGH;
		}

		// Envoi MQTT température du DS18B20
		sendMqttToDomoticz(IDXDS18B20, String(retourSensor(DS18B20)), TOPIC_DOMOTICZ_IN);

		// Envoi MQTT mesure de courant du AC712
		sendMqttToDomoticz(IDXACS712, String(valeurACS712(PIN_ACS712)), TOPIC_DOMOTICZ_IN);
	}
#endif
}

/**
 * callback notifying us of the need to save config
 *
 **/
void saveConfigCallback()
{
	Serial.println("Should save config");
	shouldSaveConfigMQTT = true;
}

/**
 * MQTT callback
 *
 **/
void callback(char *topic, byte *payload, unsigned int length)
{
	DynamicJsonDocument jsonBuffer(1024);
	String messageReceived = "";

	Serial.print("Message arrive [");
	Serial.print(topic);
	Serial.print("] ");

	// decode payload message
	for (unsigned int i = 0; i < length; i++)
	{
		messageReceived += ((char)payload[i]);
	}
	// display incoming message
	Serial.print("Message recu: ");
	Serial.println(messageReceived);

	// if domoticz message
	if (strcmp(topic, TOPIC_DOMOTICZ_OUT) == 0)
	{
		DeserializationError error = deserializeJson(jsonBuffer, messageReceived);
		if (error)
		{
			Serial.print(F("\nparsing Domoticz/out JSON Received Message failed with code: "));
			Serial.println(error.c_str());
			return;
		}

		int idx = jsonBuffer["idx"];
		int nvalue = jsonBuffer["nvalue"];
		float svalue = jsonBuffer["svalue"];
		float svalue1 = jsonBuffer["svalue1"];
		const char *name = jsonBuffer["name"];

#ifdef DEBUG
		Serial.printf("\nIDX: %i, name: %s, nVALUE: %i, sVALUE: %f, sVALUE1: %i\n", idx, name, nvalue, float(svalue), int(svalue1));
#endif
		if (idx == IDXDomoticz)
		{
			// MAJ de l'état du radiateur
			updateFilpilote(PIN_FILPILOTE_PLUS, PIN_FILPILOTE_MOINS, int(svalue1), idx);
		}
	}
}

/**
 * MQTT reconnect
 *
 **/
#ifdef MQTT
void reconnect(const char *id, const char *topic)
// void reconnect()
{
	// Loop until we're reconnected
	while (!clientMQTT.connected())
	{
		Serial.print("Connexion au serveur MQTT... Status= ");
		// Attempt to connect
		// if (clientMQTT.connect("ESP53_client", "_LOGIN_", "_PASSWORD_"))
		if (clientMQTT.connect(id, "_LOGIN_", "_PASSWORD_"))
		{
			Serial.println("OK");
			// suscribe to MQTT topics
			Serial.print("Subscribe to domoticz/out topic. Status= ");
			if (clientMQTT.subscribe(topic, 0))
			{
				if (clientMQTT.subscribe("#"))
				{
					Serial.println("OK");
				}
				else
				{
					Serial.print("KO, erreur: ");
					Serial.println(clientMQTT.state());
				};
			}
		}
		else
		{
			Serial.print("KO, erreur : ");
			Serial.println(clientMQTT.state());
			Serial.println(" On attend 5 secondes avant de recommencer");
			delay(5000);
		}
	}
}

/**
 * sendMqttToDomoticz
 * Mise à jour valeur domoticz par messages MQTT
 * idx -> adresse IDX domoticz du materiel
 * svalue -> donnée converti en String à envoyer
 * topic -> topic pour envoyer les message(domoticz/in)
 *
 **/
void sendMqttToDomoticz(int idx, String svalue, const char *topic)
{
	char msgToPublish[MQTT_MAX_PACKET_SIZE];

	DynamicJsonDocument doc(1024);
	doc["idx"] = idx;
	doc["nvalue"] = 0;
	doc["svalue"] = svalue;
	serializeJson(doc, msgToPublish);
	Serial.print(msgToPublish);
	Serial.print(" Published to ");
	Serial.print(topic);
	Serial.print(". Status=");
	if (clientMQTT.publish(topic, msgToPublish))
		Serial.println("OK");
	else
		Serial.println("KO");
}

/**
 * askMqttToDomoticz
 * Mise à jour valeur domoticz par messages MQTT
 * idx -> adresse IDX domoticz du materiel
 * svalue -> donnée converti en String à envoyer
 * topic -> topic pour envoyer les message(domoticz/in)
 *
 **/
void askMqttToDomoticz(int idx, String svalue, const char *topic)
{
	char msgToPublish[MQTT_MAX_PACKET_SIZE];

	DynamicJsonDocument doc(1024);
	doc["idx"] = idx;
	doc["command"] = svalue;
	serializeJson(doc, msgToPublish);
	Serial.print(msgToPublish);
	Serial.print(" Published to ");
	Serial.print(topic);
	Serial.print(". Status=");
	if (clientMQTT.publish(topic, msgToPublish))
		Serial.println("OK");
	else
		Serial.println("KO");
}
#endif
/**
 * retourSensor
 * Renvoie la température du DS18B20 en float
 *
 **/
float retourSensor(DallasTemperature sensor)
{
	// call sensors.requestTemperatures() to issue a global temperature
	// request to all devices on the bus
	// Serial.print("Requesting temperatures...");
	sensor.requestTemperatures(); // Send the command to get temperatures
	// After we got the temperatures, we can print them here.
	// We use the function ByIndex, and as an example get the temperature from the first sensor only.
	float tempC = sensor.getTempCByIndex(0);

	// Check if reading was successful
	if (tempC != DEVICE_DISCONNECTED_C)
	{
		Serial.print("Temperature mesuree: ");
		Serial.println(tempC);
	}
	else
	{
		Serial.println("Error: Pas de donnees de temperature disponible");
	}
	return tempC;
}

/**
 *  Effectue plusieurs lecture et calcule la moyenne pour pondérer la valeur obtenue.
 *
 **/
float valeurACS712(int pin)
{
	int valeur;
	float moyenne = 0;

	int nbr_lectures = 50;
	for (int i = 0; i < nbr_lectures; i++)
	{
		valeur = analogRead(pin);
		moyenne = moyenne + float(valeur);
	}
	moyenne = moyenne / float(nbr_lectures);
	Serial.print("Entree ADC: ");
	Serial.println(String(moyenne));

	float amplitude_courant = abs(40 * ((float)moyenne / 1024) - 20);

	Serial.print("\n Courant = ");
	Serial.print(amplitude_courant, 2);
	Serial.println("A");

	return amplitude_courant;
}

/**
 *  MAJ des sorties fil pilote en fonction du message Domoticz
 *
 **/
void updateFilpilote(int pinPlus, int pinMoins, int svalue, int idx)
{
	String message;

	Serial.println(F("Mise a jour fil Pilote depuis DOMOTICZ: "));

	// Etat de 00 à 10: Radiateur sur Arrêt
	// Etat de 11 à 20: Radiateur sur Hors Gel
	// Etat de 21 à 30: Radiateur sur ECO
	// Etat de 31 à 40: Radiateur sur Confort 2 (T° Confort - 2°C)
	// Etat de 41 à 50: Radiateur sur Confort 1 (T° Confort - 1°C)
	// Etat de 51 à 100: Radiateur sur Confort

	if (0 <= svalue && svalue < 10)
	{
		digitalWrite(pinPlus, HIGH);
		digitalWrite(pinMoins, LOW);
		confortStopTask();
		Serial.println(F("Radiateur sur Arret"));
		message = "Pin ";
		message += String(pinPlus);
		message += " = HIGH / Pin  ";
		message += String(pinMoins);
		message += " = LOW";
		Serial.println(message);
	}
	else if (10 <= svalue && svalue < 20)
	{
		digitalWrite(pinPlus, LOW);
		digitalWrite(pinMoins, HIGH);
		confortStopTask();
		Serial.println(F("Radiateur sur Hors gel"));
		message = "Pin ";
		message += String(pinPlus);
		message += " = LOW / Pin ";
		message += String(pinMoins);
		message += " = HIGH";
		Serial.println(message);
	}
	else if (20 <= svalue && svalue < 30)
	{
		digitalWrite(pinPlus, HIGH);
		digitalWrite(pinMoins, HIGH);
		confortStopTask();
		Serial.println(F("Radiateur sur ECO"));
		message = "Pin ";
		message += String(pinPlus);
		message += " = HIGH / Pin ";
		message += String(pinMoins);
		message += " = HIGH";
		Serial.println(message);
	}
	else if (30 <= svalue && svalue < 40)
	{
		confortStopTask();
		confortSetPin(pinPlus, pinMoins, 7, 293);
		Serial.println(F("Radiateur sur Confort 2"));
		// Absence de courant pendant 293s, puis présence pendant 7s
	}
	else if (40 <= svalue && svalue < 50)
	{
		confortStopTask();
		confortSetPin(pinPlus, pinMoins, 3, 297);
		Serial.println(F("Radiateur sur Confort 1"));
		// Absence de courant pendant 297s, puis présence pendant 3s
	}
	else if (50 <= svalue && svalue <= 100)
	{
		digitalWrite(pinPlus, LOW);
		digitalWrite(pinMoins, LOW);
		confortStopTask();
		Serial.println(F("Radiateur sur Confort"));
		message = "Pin ";
		message += String(pinPlus);
		message += " = LOW / Pin ";
		message += String(pinMoins);
		message += " = LOW";
		Serial.println(message);
	}
	else
	{
		Serial.println(F("Bad Led Value !"));
	}
}

/**
 *  Procédure MAJ sortie pour mode confort 1 & 2
 *
 **/
void setPinConfort(int state)
{
	digitalWrite(PIN_FILPILOTE_PLUS, state);
	digitalWrite(PIN_FILPILOTE_MOINS, state);
	// i++;
	//  Serial.print(F("Compteur: ")); Serial.println(i);
	//  Serial.print(F("STATE: ")); Serial.println(state);

	if (state == 1)
	{
		Serial.println(F("Tempo HIGH"));
	}
	else if (state == 0)
	{
		Serial.println(F("Tempo LOW"));
	}
}

/**
 *  Lancement tempo pour mode confort 1 & 2
 *
 **/
void confortSetPin(int aPinHigh, int aPinLow, float aTempoHigh, float aTempoLow)
{
	tickerSetHigh.attach(aTempoHigh, setPinConfort, 1);
	tickerSetLow.attach(aTempoLow, setPinConfort, 0);
}

/**
 * Arrêt tempo pour mode confort 1 & 2
 *
 **/
void confortStopTask()
{
	tickerSetHigh.detach();
	tickerSetLow.detach();
}

/**
 *
 *
 **/
#ifdef OTA
void onOTAStart()
{
	// Log when OTA has started
	Serial.println("OTA update started!");
	// <Add your own code here>
}

/**
 *
 *
 **/
void onOTAProgress(size_t current, size_t final)
{
	// Log every 1 second
	if (millis() - ota_progress_millis > 1000)
	{
		ota_progress_millis = millis();
		Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
	}
}

/**
 *
 *
 **/
void onOTAEnd(bool success)
{
	// Log when OTA has finished
	if (success)
	{
		Serial.println("OTA update finished successfully!");
	}
	else
	{
		Serial.println("There was an error during OTA update!");
	}
	// <Add your own code here>
}
#endif

/**
 * Déclenchement des timers
 *
 **/
#ifdef ISR_TIMER
void IRAM_ATTR TimerHandler()
{
	static bool toggle = false;
	static bool started = false;
	static int timeRun = 0;

	ISR_Timer.run();

	// Toggle LED every LED_TOGGLE_INTERVAL_MS = 2000ms = 2s
	if (++timeRun == (LED_TOGGLE_INTERVAL_MS / HW_TIMER_INTERVAL_MS))
	{
		timeRun = 0;

		if (!started)
		{
			started = true;
			pinMode(LED_BUILTIN, OUTPUT);
		}

		// timer interrupt toggles pin LED_BUILTIN
		digitalWrite(LED_BUILTIN, toggle);
		toggle = !toggle;
	}
}
#endif

/**
 * affiche la valeur des timer pour mode confort 1 & 2
 *
 **/
#if (TIMER_INTERRUPT_DEBUG > 0)
void printStatus(uint16_t index, unsigned long timerDelay, unsigned long deltaMillis, unsigned long currentMillis)
{
	Serial.print(timerDelay / 1000);
	Serial.print("s: Delta ms = ");
	Serial.print(deltaMillis);
	Serial.print(", ms = ");
	Serial.println(currentMillis);
}
#endif

/*
 *Sous programme d'interruption du timer 1
 *
 */
#ifdef ISR_TIMER
void doingSomethingSec()
{
#if (TIMER_INTERRUPT_DEBUG > 0)
	static unsigned long previousMillis = startMillis;

	unsigned long currentMillis = millis();
	unsigned long deltaMillis = currentMillis - previousMillis;

	printStatus(0, TIMER_INTERVAL_3S, deltaMillis, currentMillis);

	previousMillis = currentMillis;
#endif

	static bool toggle = false;

	if (toggle)
	{
		ISR_Timer.changeInterval(0, TIMER_INTERVAL_3S);
	}
	else
	{
		ISR_Timer.changeInterval(0, (TIMER_INTERVAL_300S - TIMER_INTERVAL_3S));
	}

	toggle = !toggle;
}
#endif