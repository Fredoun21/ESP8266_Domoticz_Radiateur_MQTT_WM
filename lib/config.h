#ifndef CONFIG_H
#define CONFIG_H

/*
DEBUG
*/
#define DEBUG // Active l'affichage du debugage sur la console
// #undef DEBUG // Désactive l'affichage du debugage sur la console
#define MQTT // Active les fonctions MQTT
// #undef MQTT // Desactive les fonctions MQTT
// #define OTA // Active Elegant OTA
#undef OTA  // Deséctive OTA

/*
CONFIGURATION RESEAU WIFI
*/
#define LOCAL_SSID "SSID"
#define LOCAL_PASSWORD "PASSWORD"
#define LOCAL_HOST "192.168.1.100"
#define LOCAL_PORT 8080
#define LOCAL_GATEWAY "192.168.1.254"
#define LOCAL_SUBNET "255.255.255.0"

/*
CONFIGURATION WifiManager
*/
#define WM_PASSWORD "password"

/*
CONFIGURATION MQTT
*/
#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT "1883"
#define MQTT_USER "DVES_USER"             // s'il a été configuré sur Mosquitto
#define MQTT_PASSWORD ""                  // idem
#define TOPIC_DOMOTICZ_IN "domoticz/in"   // topic d'écriture  MQTT -> Domoticz
#define TOPIC_DOMOTICZ_OUT "domoticz/out" // topic de lecture Domoticz -> MQTT
// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifdef MQTT_MAX_PACKET_SIZE
#undef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 1024
#endif

#endif