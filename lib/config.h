#ifndef CONFIG_H
#define CONFIG_H

//----------------------------------------------------------------- DEBUG OPTION DE COMPILATION

#define DEBUG // Active l'affichage du debugage sur la console
// #undef DEBUG // Désactive l'affichage du debugage sur la console
// #define MQTT // Active les fonctions MQTT
#undef MQTT // Desactive les fonctions MQTT
// #define OTA // Active Elegant OTA
#undef OTA // Deséctive OTA
#define CONNET_XIAOMI_11T
// #undef CONNET_XIAOMI_11T
#define ISR_TIMER
// #undef ISR_TIMER

//----------------------------------------------------------------- CONFIGURATION RESEAU WIFI
#ifndef CONNET_XIAOMI_11T
#define LOCAL_SSID "Bbox-39156D4C"
#define LOCAL_PASSWORD "16D2DD9977F12A97AAAD2C11ED59E2"
#define LOCAL_HOST 192, 168, 1, 100
#define LOCAL_PORT 8080
#define LOCAL_GATEWAY 192, 168, 1, 254
#define LOCAL_SUBNET 255, 255, 255, 0
#endif

#ifdef CONNET_XIAOMI_11T
#define LOCAL_SSID "XIAOMI 11T"
#define LOCAL_PASSWORD "52fnhqdknmgahst"
#define LOCAL_HOST 192, 168, 128, 211
#define LOCAL_PORT 8080
#define LOCAL_GATEWAY 192, 168, 128, 212
#define LOCAL_SUBNET 255, 255, 255, 0
#endif

/*
CONFIGURATION WifiManager
*/
#define WM_PASSWORD "password"

/*
CONFIGURATION MQTT
*/
#ifdef MQTT
#ifndef CONNET_XIAOMI_11T
#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT "1883"
#define MQTT_USER "DVES_USER"             // s'il a été configuré sur Mosquitto
#define MQTT_PASSWORD ""                  // idem
#define TOPIC_DOMOTICZ_IN "domoticz/in"   // topic d'écriture  MQTT -> Domoticz
#define TOPIC_DOMOTICZ_OUT "domoticz/out" // topic de lecture Domoticz -> MQTT
#endif

#ifdef CONNET_XIAOMI_11T
#define MQTT_SERVER "192.168.128.211"
#define MQTT_PORT "1883"
#define MQTT_USER "domoticz"             // s'il a été configuré sur Mosquitto
#define MQTT_PASSWORD "210804"                  // idem
#define TOPIC_DOMOTICZ_IN "inTopic" //"domoticz/in"   // topic d'écriture  MQTT -> Domoticz
#define TOPIC_DOMOTICZ_OUT "outTopic" //"domoticz/out" // topic de lecture Domoticz -> MQTT
#endif
#endif

/*
CONFIGURATION TIMER ISR
*/
/* These define's must be placed at the beginning before #include "ESP8266TimerInterrupt.h"
 _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
 Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
 Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.*/
#if !defined(ESP8266)
#error // Ce code est conçu pour s'exécuter sur des planches basées sur ESP8266 et ESP8266!Veuillez vérifier vos outils -> Paramètre de la carte.
#endif

#define TIMER_INTERRUPT_DEBUG 2
#define _TIMERINTERRUPT_LOGLEVEL_ 0

// Select a Timer Clock
#ifdef ISR_TIMER
#define USING_TIM_DIV1 false  // for shortest and most accurate timer
#define USING_TIM_DIV16 false // for medium time and medium accurate timer
#define USING_TIM_DIV256 true // for longest timer but least accurate. Default

#define HW_TIMER_INTERVAL_MS 50L
#define LED_TOGGLE_INTERVAL_MS 2000L

#define TIMER_INTERVAL_3S 3000L     // delais fil pilote passant pour  T° Confort -1°C
#define TIMER_INTERVAL_7S 7000L     // delais fil pilote passant pour T° Confort -2°C
#define TIMER_INTERVAL_300S 12000L //300000L // délais cycle total pout T° Confort -1 ou -2°C
#endif
#endif