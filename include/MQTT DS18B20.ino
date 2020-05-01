/*
  Projet d'apprentissage d'un objet connecté (IoT)  pour réaliser une sonde de température
  ESP8266 + DS12B20 + LED + MQTT + Home-Assistant
  Projets DIY (https://www.projetsdiy.fr) - Mai 2016
  Licence : MIT
*/

// Include the libraries we need
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Config.h"

/*
 PIN SETTINGS
 */
#define PIN_RELAIS 14
#define PIN_ONE_WIRE_BUS 13

/*
CONFIGURATION RESEAU WIFI
*/
IPAddress local_IP(192, 168, 1, 95); //Adresse IP du module

/*
CONFIGURATION MQTT
*/
#define MQTT_ID "ESP52Client"

/*
CONFIGURATION DOMOTICZ
*/
#define IDXTEMP 18        //IDX de ESPTest52 T°
#define IDXESP52RELAIS 38 //IDX de ESPTest52 Relais

/*
Création des objets
*/
// client MQTT
WiFiClient espClient;
PubSubClient clientMQTT(espClient);

OneWire oneWire(PIN_ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
    Serial.begin(115200);

    pinMode(PIN_RELAIS, OUTPUT);

    // Start LE DS18b20
    sensors.begin();

    // Connexion au réseau wifi
    setup_wifi(local_IP, gateway, subnet, SSID, PASSWORD);
    delay(500);

    //Configuration de la connexion au serveur MQTT
    clientMQTT.setServer(MQTT_SERVER, 1883);
    //La fonction de callback qui est executée à chaque réception de message
    clientMQTT.setCallback(callback);
}

/*
 * Main function, get and show the temperature
 */
void loop(void)
{
    // Connexion client MQTT
    if (!clientMQTT.connected())
    {
        reconnect(MQTT_ID, TOPIC_DOMOTICZ_OUT);
    }
    clientMQTT.loop();

    unsigned long currentMillis = millis();
    boolean teleInfoReceived;
    String cmd = "";

    // ajout d'un delais de 60s apres chaque trame envoyés pour éviter d'envoyer
    // en permanence des informations à domoticz et de créer des interférences
    if ((currentMillis - previousMillis > watchdog) || firstLoop == LOW)
    {
        previousMillis = currentMillis;
        firstLoop = HIGH;

        // Envoi MQTT température du DS18B20
        sendMqttToDomoticz(IDXTEMP, String(retourSensor()), TOPIC_DOMOTICZ_IN);

        // Demande état d'un device
        askMqttToDomoticz(IDXESP52RELAIS, "getdeviceinfo", TOPIC_DOMOTICZ_IN);
    }
}

/*
setup_wifi
Connexion du module ESP au wifi
local_ip -> adressi IP du module
gateway -> passerelle réseau
subnet -> masque de sous réseau
ssid -> nom du SSID pour la connexion Wifi
password -> mot de passe pour la connexion Wifi 
 */
void setup_wifi(IPAddress local_ip, IPAddress gateway, IPAddress subnet, const char *ssid, const char *password)
{
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println(F("Connection Wifi..."));
    Serial.println(ssid);

    WiFi.config(local_ip, gateway, subnet);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

/*
reconnect
Connexion server MQTT
id -> nom du client 
topic -> nom du topic pour envoyer les messages (domoticz/in)
*/
void reconnect(const char *id, const char *topic)
{
    //Boucle jusqu'à obtenir une reconnexion
    while (!clientMQTT.connected())
    {
        Serial.print("Connexion au serveur MQTT... Status= ");
        if (clientMQTT.connect(id, MQTT_USER, MQTT_PASSWORD))
        {
            Serial.println("OK");
            // suscribe to MQTT topics
            Serial.print("Subscribe to domoticz/out topic. Status= ");
            if (clientMQTT.subscribe(topic, 0))
                Serial.println("OK");
            else
            {
                Serial.print("KO, erreur: ");
                Serial.println(clientMQTT.state());
            };
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

/*
callback
Déclenche les actions à la réception d'un message
topic -> nom du topic de réception des message (domoticz/out)
payload -> message reçu
length -> longueur message reçu
 */
void callback(char *topic, byte *payload, unsigned int length)
{
    DynamicJsonDocument jsonBuffer(MQTT_MAX_PACKET_SIZE);
    String messageReceived = "";

    // Affiche le topic entrant - display incoming Topic
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.println("] ");

    // decode payload message
    for (int i = 0; i < length; i++)
    {
        messageReceived += ((char)payload[i]);
    }
    // display incoming message
    Serial.println("Message recu:");
    Serial.print(messageReceived);

    // if domoticz message
    if (strcmp(topic, TOPIC_DOMOTICZ_OUT) == 0)
    {
        DeserializationError error = deserializeJson(jsonBuffer, messageReceived);
        if (error)
        {
            Serial.print(F("parsing Domoticz/out JSON Received Message failed with code: "));
            Serial.println(error.c_str());
            return;
        }

        int idx = jsonBuffer["idx"];
        int nvalue = jsonBuffer["nvalue"];
        float svalue = jsonBuffer["svalue"];
        const char *name = jsonBuffer["name"];

#ifdef DEBUG
        Serial.printf("\nIDX: %i, nVALUE: %i, sVALUE: %f, name: %s", idx, nvalue, float(svalue), name);
#endif

        if (idx == IDXRELAIS)
        {
            Serial.printf("\nidx: %i, name: %s, nvalue: %i\n", idx, name, nvalue);
            if (nvalue == 0)
                digitalWrite(PIN_RELAIS, LOW);
            else if (nvalue == 1)
                digitalWrite(PIN_RELAIS, HIGH);
            else
            {
                Serial.println("\nErreur dans le message de commande du relais ");
            }
            Serial.printf("\nLe relais %s est a %i\n", name, digitalRead(PIN_RELAIS));
        }
    }
}

/*
sendMqttToDomoticz
Mise à jour valeur domoticz par messages MQTT 
idx -> adresse IDX domoticz du materiel
svalue -> donnée converti en String à envoyer 
topic -> topic pour envoyer les message(domoticz/in)
*/
void sendMqttToDomoticz(int idx, String svalue, const char *topic)
{
    char msgToPublish[MQTT_MAX_PACKET_SIZE];

    StaticJsonDocument<1024> doc;
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

/*
askMqttToDomoticz
Mise à jour valeur domoticz par messages MQTT 
idx -> adresse IDX domoticz du materiel
svalue -> donnée converti en String à envoyer 
topic -> topic pour envoyer les message(domoticz/in)
*/
void askMqttToDomoticz(int idx, String svalue, const char *topic)
{
    char msgToPublish[MQTT_MAX_PACKET_SIZE];

    StaticJsonDocument<1024> doc;
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
/*
retourSensor
Renvoie la température du DS18B20 en float
*/
float retourSensor()
{
    // call sensors.requestTemperatures() to issue a global temperature
    // request to all devices on the bus
    // Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); // Send the command to get temperatures
    // After we got the temperatures, we can print them here.
    // We use the function ByIndex, and as an example get the temperature from the first sensor only.
    float tempC = sensors.getTempCByIndex(0);

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