/*
CONFIGURATION DOMOTICZ
*/
#define ESP50 // ESP test
#undef ESP53  // ESP SDB
#undef ESP54  // ESP Cuisine
#undef ESP55  // ESP MaxAux
#undef ESP56  // ESP Romeo
#undef ESP57  // ESP Parent

#ifdef ESP50
// Parametres du dispositif dans Domoticz
#define NAMEID "ESP50_Radiateur" // Nom du dispositif
// #define LOCAL_IP (192, 168, 1, 51) //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 6              //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 11              //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 19               //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP50Client"      //Nom du client sur MQTT
#endif

#ifdef ESP53
// Parametres du dispositif dans Domoticz
String NameID = "SdB_Radiateur";          // Nom du dispositif
const String ipDomoticz = "192.168.1.53"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 7                     //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 17                     //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 16                      //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP53Client"             //Nom du client sur MQTT
#endif

#ifdef ESP54
// Parametres du dispositif dans Domoticz
String NameID = "Cuisine_Radiateur";      // Nom du dispositif
const String ipDomoticz = "192.168.1.54"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 8                     //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 14                     //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 15                      //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP54Client"             //Nom du client sur MQTT
#endif

#ifdef ESP55
// Parametres du dispositif dans Domoticz
String NameID = "MaxAux_Radiateur";       // Nom du dispositif
const String ipDomoticz = "192.168.1.55"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 20                    //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 21                     //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 22                      //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP55Client"             //Nom du client sur MQTT
#endif

#ifdef ESP56
// Parametres du dispositif dans Domoticz
String NameID = "Romeo_Radiateur";        // Nom du dispositif
const String ipDomoticz = "192.168.1.56"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 23                    //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 24                     //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 25                      //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP56Client"             //Nom du client sur MQTT
#endif

#ifdef ESP57
// Parametres du dispositif dans Domoticz
String NameID = "Parent_Radiateur";       // Nom du dispositif
const String ipDomoticz = "192.168.1.57"; //Adresse IP du radiateur dans congif Domoticz
#define IDXDomoticz 26                    //Numéro IDX du radiateur dans congif Domoticz
#define IDXDS18B20 27                     //Numéro IDX de la sonde de T° dans congif Domoticz
#define IDXACS712 28                      //Numéro IDX de la mesure de courant dans congif Domoticz
#define MQTT_ID "ESP57Client"             //Nom du client sur MQTT
#endif
