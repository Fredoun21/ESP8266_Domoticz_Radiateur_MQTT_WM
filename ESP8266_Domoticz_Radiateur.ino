/*
  Sketch ESP8266_Domoticz_Radiateur

  Programme de pilotage de radiatieur par fil pilote
  Mesure de T° local par DS18B20
  Mesure du courant consommé sur entrée ADC

  The circuit:
  * list the components attached to each input
  * list the components attached to each output

  Created 26 Sept 2017
  By Fred WACHE
  Modified 01 octobre 2017
  By author's name

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
//#include <FS.h>

// Librairie pour le DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Ticker.h>

#include "DomoticzConfig.h"
#include "Config.h"

//Création server
ESP8266WebServer SERVEUR (80);

// Création tache tempo pour mode confort 1 et 21
Ticker tickerSetHigh;
Ticker tickerSetLow;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature DS18B20(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);

  pinMode(FILPILOTE_PINPLUS, OUTPUT);
  pinMode(FILPILOTE_PINMOINS, OUTPUT);

  // Positionne en mode Hors Gel à la mise sous tension
  digitalWrite(FILPILOTE_PINPLUS, HIGH);
  digitalWrite(FILPILOTE_PINMOINS, HIGH);

  // Start up the library
  DS18B20.begin();

  Serial.println(F("Connection Wifi..."));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print(F("WiFi connecté à ")); Serial.println ( ssid );
  Serial.print(F("Adrese IP: ")); Serial.println ( WiFi.localIP() );

//  if (!SPIFFS.begin())
//  {
//    // Serious problem
//    Serial.println("SPIFFS Mount failed");
//  } else {
//    Serial.println("SPIFFS Mount succesfull");
//  }
//
//  SERVEUR.serveStatic("/img", SPIFFS, "/img");
//  SERVEUR.serveStatic("/", SPIFFS, "/index.html");

  SERVEUR.on("/filpil", updateFilpilote);
  SERVEUR.begin();
}

// Procédure MAJ sortie pour mode confort 1 & 2
void setPinConfort (int state) {
  digitalWrite(FILPILOTE_PINPLUS, state);
  digitalWrite(FILPILOTE_PINMOINS, state);
  i++;
  //  Serial.print(F("Compteur: ")); Serial.println(i);
  //  Serial.print(F("STATE: ")); Serial.println(state);

  if (state == 1) {
    Serial.println(F("Tempo HIGH"));
  }
  else if (state == 0) {
    Serial.println(F("Tempo LOW"));
  }
}

// Lancement tempo pour mode confort 1 & 2
void confortSetPin( int aPinHigh, int aPinLow,  float aTempoHigh ,  float aTempoLow ) {
  tickerSetHigh.attach(aTempoHigh, setPinConfort, 1);
  tickerSetLow.attach(aTempoLow, setPinConfort, 0);
}

// Arrêt tempo pour mode confort 1 & 2
void confortStopTask() {
  tickerSetHigh.detach();
  tickerSetLow.detach();
}

// MAJ des sorties fil pilote en fonction du message Domoticz
void updateFilpilote() {
  String message;
  String url;

  Serial.println(F("Mise à jour fil Pilote depuis DOMOTICZ: "));
  for ( int i = 0 ; i < SERVEUR.args(); i++ ) {
    Serial.print(SERVEUR.argName(i)); ": "; Serial.println(SERVEUR.arg(i));
  }

  // Vérification du jeton de contrôle de comm
  String token = SERVEUR.arg("token");
  if ( token != String(JETON) ) {
    Serial.println(F("Not authentified "));
    return;
  }

  // Recupération du GET() de l'état radiateur demandé
  int etat = SERVEUR.arg("etat").toInt();
  // Etat de 00 à 10: Radiateur sur Arrêt
  // Etat de 11 à 20: Radiateur sur Hors Gel
  // Etat de 21 à 30: Radiateur sur ECO
  // Etat de 31 à 40: Radiateur sur Confort 2 (T° Confort - 2°C)
  // Etat de 41 à 50: Radiateur sur Confort 1 (T° Confort - 1°C)
  // Etat de 51 à 100: Radiateur sur Confort

  //envoie données l'accusé de réception vers Domoticz
  //dans variable Domoticz NameID
  Serial.println(F("Envois accusé de réception  vers Domoticz, pour état radiateur"));
  url = "/json.htm?type=command&param=updateuservariable&vname=" + String(NameID) + "&vtype=0&vvalue=";
  url += String(etat);
  sendToDomoticz(url);

  if ( 0 <= etat && etat <= 10  ) {
    digitalWrite(FILPILOTE_PINPLUS, HIGH);
    digitalWrite(FILPILOTE_PINMOINS, LOW);
    confortStopTask();
    Serial.println(F("Radiateur sur Arrêt"));
    message = "Pin ";
    message += String(FILPILOTE_PINPLUS);
    message += " = HIGH / Pin  ";
    message += String(FILPILOTE_PINMOINS);
    message += " = LOW";
    Serial.println(message);

  } else if ( 10 < etat && etat <= 20 ) {
    digitalWrite(FILPILOTE_PINPLUS, LOW);
    digitalWrite(FILPILOTE_PINMOINS, HIGH);
    confortStopTask();
    Serial.println(F("Radiateur sur Hors gel"));
    message = "Pin ";
    message += String(FILPILOTE_PINPLUS);
    message += " = LOW / Pin ";
    message += String(FILPILOTE_PINMOINS);
    message += " = HIGH";
    Serial.println(message);

  } else if ( 20 < etat && etat <= 30 ) {
    digitalWrite(FILPILOTE_PINPLUS, HIGH);
    digitalWrite(FILPILOTE_PINMOINS, HIGH);
    confortStopTask();
    Serial.println(F("Radiateur sur ECO"));
    message = "Pin ";
    message += String(FILPILOTE_PINPLUS);
    message += " = HIGH / Pin ";
    message += String(FILPILOTE_PINMOINS);
    message += " = HIGH";
    Serial.println(message);

  } else if ( 30 < etat && etat <= 40 ) {
    confortStopTask();
    confortSetPin(FILPILOTE_PINPLUS, FILPILOTE_PINMOINS, 7, 293);
    Serial.println(F("Radiateur sur Confort 2"));
    // Absence de courant pendant 293s, puis présence pendant 7s

  } else if ( 40 < etat && etat <= 50) {
    confortStopTask();
    confortSetPin(FILPILOTE_PINPLUS, FILPILOTE_PINMOINS, 3, 297);
    Serial.println(F("Radiateur sur Confort 1"));
    // Absence de courant pendant 297s, puis présence pendant 3s

  } else if ( 50 < etat && etat <= 100 ) {
    digitalWrite(FILPILOTE_PINPLUS, LOW);
    digitalWrite(FILPILOTE_PINMOINS, LOW);
    confortStopTask();
    Serial.println(F("Radiateur sur Confort"));
    message = "Pin ";
    message += String(FILPILOTE_PINPLUS);
    message += " = LOW / Pin ";
    message += String(FILPILOTE_PINMOINS);
    message += " = LOW";
    Serial.println(message);

  } else {
    Serial.println(F("Bad Led Value !"));
  }
}

void sendToDomoticz(String url)
{
  // initialise le client Http
  HTTPClient CLIENT;

  Serial.println();
  Serial.println(F("Send data to Domoticz"));
  Serial.print(F("connecting to "));
  Serial.println(host);
  Serial.print(F("Requesting URL: "));
  Serial.println(url);
  CLIENT.begin(host, port, url);  //server linux Domoticz sur port 8080
  int httpCode = CLIENT.GET();
  if (httpCode)
  {
    Serial.print(F("HttpCode: "));
    Serial.println(httpCode);
    if (httpCode == 200)
    {
      String payload = CLIENT.getString();
      Serial.println(F("Domoticz response "));
      Serial.println(payload);
    }
  }
  Serial.println(F("closing connection to Domoticz"));
  CLIENT.end();
}

void sendToServer(String url, String host, int  portserver)
{
  // initialise le client Http
  HTTPClient CLIENT;

  Serial.println();
  Serial.println(F("Send data to MySQL"));
  Serial.print(F("connecting to "));
  Serial.println(host);
  Serial.print(F("Requesting URL: "));
  Serial.println(url);
  CLIENT.begin(host, portserver, url);
  int httpCode = CLIENT.GET();
  if (httpCode)
  {
    Serial.print(F("HttpCode: "));
    Serial.println(httpCode);
    if (httpCode == 200)
    {
      String payload = CLIENT.getString();
      Serial.println(F("Reponse:"));
      Serial.println(payload);
    }
  }
  Serial.println(F("closing connection to SQL"));
  CLIENT.end();
}

// Effectue plusieurs lecture et calcule la moyenne pour pondérer
// la valeur obtenue.
float valeurACS712( int pin )
{
  int valeur;
  float moyenne = 0;

  int nbr_lectures = 50;
  for ( int i = 0; i < nbr_lectures; i++ ) {
    valeur = analogRead( pin );
    moyenne = moyenne + float(valeur);
  }
  moyenne = moyenne / float(nbr_lectures);
  Serial.print("Entrée ADC: ");
  Serial.println(String(moyenne));

  float amplitude_courant = abs(40 * ((float)moyenne / 1024) - 20);

  Serial.print("\n Courant = ");
  Serial.print(amplitude_courant, 2);
  Serial.println("A");

  return amplitude_courant;
}

void loop() {

  unsigned long currentMillis = millis();
  String url;

  if (test == 1)
  {
    int acs712_value = valeurACS712(ACS712_PIN);
  }

  if (( currentMillis - previousMillis > watchdog ) || firstLoop == LOW)  {
    previousMillis = currentMillis;
    firstLoop = HIGH;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("WiFi not connected !"));
    } else {

      // call sensors.requestTemperatures() to issue a global temperature
      // request to all devices on the bus
      Serial.print("Requesting temperatures...");
      DS18B20.requestTemperatures(); // Send the command to get temperatures
      Serial.println("DONE");

      // After we got the temperatures, we can print them here.
      // We use the function ByIndex, and as an example get the temperature from the first sensor only.
      Serial.print("Temperature for the device 1 (index 0) is: ");

      float t = DS18B20.getTempCByIndex(0);
      Serial.println(String(t));

      //envoie données vers Domoticz pour D18B20
      Serial.println(F("Envois T° vers Domoticz"));
      url = "/json.htm?type=command&param=udevice&idx=" + String(IDXDS18B20) + "&nvalue=0&svalue=";
      url += String(t);
      sendToDomoticz(url);

      int acs712_value = valeurACS712(ACS712_PIN);

      //envoie données vers Domoticz pour ACS712
      Serial.println(F("Envois T° vers Domoticz"));
      url = "/json.htm?type=command&param=udevice&idx=" + String(IDXACS712) + "&nvalue=0&svalue=";
      url += String(acs712_value);
      sendToDomoticz(url);

      //envoie données vers page save_data.PHP pour enregistrement data dans mySQL
      url = "/web-domoticz/public_html/save_data.php?NameID=" + NameID;
      url += "&temperature="; url += String(t);
      url += "&courant="; url += String(acs712_value);
      sendToServer(url, ipaddress, 80 );
    }
  }
  SERVEUR.handleClient();
}
