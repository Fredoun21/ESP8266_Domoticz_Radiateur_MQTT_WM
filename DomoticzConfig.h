const char* host = "192.168.1.100";
const String ipaddress = "192.168.1.100";
const int  port = 8080;

const int FILPILOTE_PINPLUS = 12; // N° de Pin fil pilote
const int FILPILOTE_PINMOINS = 13; // N° de Pin fil pilote
const int ACS712_PIN = A0;
const int test = 0;

const int   watchdog = 60000; // Fréquence d'envoi des données à Domoticz 60s
boolean firstLoop = LOW; // Affichage au démarrage
unsigned long previousMillis = millis();
unsigned long maintenant = millis();
unsigned long dateDernierChangement = 0;
long i = 0;

#define JETON     "123abCde"  //Code de sécurisation de comm
#define ONE_WIRE_BUS 14 // Data wire is plugged into port 14 on the ESP8266


// Parametres du dispositif dans Domoticz
//String NameID = "ESPTest_Radiateur"; // Nom du dispositif
//const String ipDomoticz = "192.168.1.51"; //Adresse IP du radiateur dans congif Domoticz
//#define IDXDomoticz 6 //Numéro IDX du radiateur dans congif Domoticz
//#define IDXDS18B20 18 //Numéro IDX de la sonde de T° dans congif Domoticz
//#define IDXACS712 19 //Numéro IDX de la mesure de courant dans congif Domoticz

//// Parametres du dispositif dans Domoticz
//String NameID = "SdB_Radiateur"; // Nom du dispositif
//const String ipDomoticz = "192.168.1.53"; //Adresse IP du radiateur dans congif Domoticz
//#define IDXDomoticz 7 //Numéro IDX du radiateur dans congif Domoticz
//#define IDXDS18B20 17 //Numéro IDX de la sonde de T° dans congif Domoticz
//#define IDXACS712 16 //Numéro IDX de la mesure de courant dans congif Domoticz

//// Parametres du dispositif dans Domoticz
//String NameID = "Cuisine_Radiateur"; // Nom du dispositif
//const String ipDomoticz = "192.168.1.54"; //Adresse IP du radiateur dans congif Domoticz
//#define IDXDomoticz 8 //Numéro IDX du radiateur dans congif Domoticz
//#define IDXDS18B20 14 //Numéro IDX de la sonde de T° dans congif Domoticz
//#define IDXACS712 15 //Numéro IDX de la mesure de courant dans congif Domoticz

//// Parametres du dispositif dans Domoticz
//String NameID = "MaxAux_Radiateur"; // Nom du dispositif
//const String ipDomoticz = "192.168.1.55"; //Adresse IP du radiateur dans congif Domoticz
//#define IDXDomoticz 20 //Numéro IDX du radiateur dans congif Domoticz
//#define IDXDS18B20 21 //Numéro IDX de la sonde de T° dans congif Domoticz
//#define IDXACS712 22 //Numéro IDX de la mesure de courant dans congif Domoticz

//// Parametres du dispositif dans Domoticz
String NameID = "Romeo_Radiateur"; // Nom du dispositif
const String ipDomoticz = "192.168.1.56"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 23 //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 24 //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 25 //Numéro IDX de la mesure de courant dans congif Domoticz

//// Parametres du dispositif dans Domoticz
//String NameID = "Parent_Radiateur"; // Nom du dispositif
//const String ipDomoticz = "192.168.1.57"; //Adresse IP du radiateur dans congif Domoticz
//#define IDXDomoticz 26 //Numéro IDX du radiateur dans congif Domoticz
//#define IDXDS18B20 27 //Numéro IDX de la sonde de T° dans congif Domoticz
//#define IDXACS712 28 //Numéro IDX de la mesure de courant dans congif Domoticz

