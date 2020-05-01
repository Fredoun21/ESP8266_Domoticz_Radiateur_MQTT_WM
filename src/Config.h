/*
DEBUG
*/
// #define DEBUG // Active l'affichage du debugage sur la console
#undef DEBUG // Désactive l'affichage du debugage sur la console
// #define MQTT // Active les fonctions MQTT
#undef MQTT // Desactive les fonctions MQTT

/*
CONFIGURATION RESEAU WIFI
*/
#define LocalSSID "Bbox-39156D4C"
#define LocalPASSWORD "16D2DD9977F12A97AAAD2C11ED59E2"
#define LocalHOST "192.168.1.100"
#define LocalPORT 8080

IPAddress LOCAL_IP(192, 168, 1, 51);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

/*
CONFIGURATION MQTT
*/
#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USER "guest"                 //s'il a été configuré sur Mosquitto
#define MQTT_PASSWORD "guest"             //idem
#define TOPIC_DOMOTICZ_IN "domoticz/in"   // topic d'écriture  MQTT -> Domoticz
#define TOPIC_DOMOTICZ_OUT "domoticz/out" // topic de lecture Domoticz -> MQTT

/*
variable gestion de boucle
*/
const int watchdog = 300000;              // Fréquence d'envoi des données à Domoticz 5min
unsigned long previousMillis = millis(); // mémoire pour envoi des données
boolean firstLoop = LOW;                 // Affichage au démarrage
