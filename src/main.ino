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
#include <Ticker.h>

#include "DomoticzConfig.h"
#include "Config.h"

#define DEBUG

/*
 PIN SETTINGS
 */
#define PIN_FILPILOTE_PLUS 12  // N° de Pin fil pilote
#define PIN_FILPILOTE_MOINS 13 // N° de Pin fil pilote
#define PIN_ACS712 A0          // Mesure de courant
#define PIN_ONE_WIRE_BUS 14    // Mesure de température

/*
CONFIGURATION DOMOTICZ
*/
#define IDXTEMP 11 //IDX de ESPTest52 T°

long i = 0;

/*
Création des objets
*/
// client MQTT
WiFiClient espClient;
PubSubClient clientMQTT(espClient);

OneWire oneWire(PIN_ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);

// Création tache tempo pour mode confort 1 et 21
Ticker tickerSetHigh;
Ticker tickerSetLow;

/*
 * The setup function. We only start the sensors here
 */
void setup(void)
{
    Serial.begin(115200);

    pinMode(PIN_FILPILOTE_PLUS, OUTPUT);
    pinMode(PIN_FILPILOTE_MOINS, OUTPUT);

    // Positionne en mode Hors Gel à la mise sous tension
    digitalWrite(PIN_FILPILOTE_PLUS, HIGH);
    digitalWrite(PIN_FILPILOTE_MOINS, HIGH);

    // Start LE DS18b20
    DS18B20.begin();

    Serial.printf("\nID MQTT: %s", MQTT_ID);

    // Connexion au réseau wifi
    setup_wifi(LOCAL_IP, gateway, subnet, LocalSSID, LocalPASSWORD);
    delay(500);

    //Configuration de la connexion au serveur MQTT
    clientMQTT.setServer(MQTT_SERVER, MQTT_PORT);
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

    Serial.println("\nWiFi connected");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Puissance du signal (RSSI):");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
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

/*
 Effectue plusieurs lecture et calcule la moyenne pour pondérer
 la valeur obtenue.
*/
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

// MAJ des sorties fil pilote en fonction du message Domoticz
void updateFilpilote(int pinP, int pinM, int svalue, int idx)
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
        digitalWrite(pinP, HIGH);
        digitalWrite(pinM, LOW);
        confortStopTask();
        Serial.println(F("Radiateur sur Arret"));
        message = "Pin ";
        message += String(pinP);
        message += " = HIGH / Pin  ";
        message += String(pinM);
        message += " = LOW";
        Serial.println(message);
    }
    else if (10 <= svalue && svalue < 20)
    {
        digitalWrite(pinP, LOW);
        digitalWrite(pinM, HIGH);
        confortStopTask();
        Serial.println(F("Radiateur sur Hors gel"));
        message = "Pin ";
        message += String(pinP);
        message += " = LOW / Pin ";
        message += String(pinM);
        message += " = HIGH";
        Serial.println(message);
    }
    else if (20 <= svalue && svalue < 30)
    {
        digitalWrite(pinP, HIGH);
        digitalWrite(pinM, HIGH);
        confortStopTask();
        Serial.println(F("Radiateur sur ECO"));
        message = "Pin ";
        message += String(pinP);
        message += " = HIGH / Pin ";
        message += String(pinM);
        message += " = HIGH";
        Serial.println(message);
    }
    else if (30 <= svalue && svalue < 40)
    {
        confortStopTask();
        confortSetPin(pinP, pinM, 7, 293);
        Serial.println(F("Radiateur sur Confort 2"));
        // Absence de courant pendant 293s, puis présence pendant 7s
    }
    else if (40 <= svalue && svalue < 50)
    {
        confortStopTask();
        confortSetPin(pinP, pinM, 3, 297);
        Serial.println(F("Radiateur sur Confort 1"));
        // Absence de courant pendant 297s, puis présence pendant 3s
    }
    else if (50 <= svalue && svalue <= 100)
    {
        digitalWrite(pinP, LOW);
        digitalWrite(pinM, LOW);
        confortStopTask();
        Serial.println(F("Radiateur sur Confort"));
        message = "Pin ";
        message += String(pinP);
        message += " = LOW / Pin ";
        message += String(pinM);
        message += " = LOW";
        Serial.println(message);
    }
    else
    {
        Serial.println(F("Bad Led Value !"));
    }
}

// Procédure MAJ sortie pour mode confort 1 & 2
void setPinConfort(int state)
{
    digitalWrite(PIN_FILPILOTE_PLUS, state);
    digitalWrite(PIN_FILPILOTE_MOINS, state);
    i++;
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

// Lancement tempo pour mode confort 1 & 2
void confortSetPin(int aPinHigh, int aPinLow, float aTempoHigh, float aTempoLow)
{
    tickerSetHigh.attach(aTempoHigh, setPinConfort, 1);
    tickerSetLow.attach(aTempoLow, setPinConfort, 0);
}

// Arrêt tempo pour mode confort 1 & 2
void confortStopTask()
{
    tickerSetHigh.detach();
    tickerSetLow.detach();
}