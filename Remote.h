#include <FS.h>
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true


#include "WiFi.h"
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>



// A modifier avec le nom de votre réseau
const char *ssid = "Лёха";
// A modifier avec le mot de passe de votre réseau
const char *password = "PavlodarOblysy7182";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

int joy_left_x = 0;
int joy_left_y = 0;

int joy_right_x = 0;
int joy_right_y = 0;

#define CONNECTION_TIMEOUT 10

// On déclare le prototype de la fonction permettant d'initialiser la communication WiFi
void initWifi();

// On paramètre la page Web à renvoyer lorsque l'on se connecte sur le serveur
void onRootRequest(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}

// On paramètre notre websocket : c'est un type de connexion permanente entre notre serveur (le robot) et notre client (le téléphone du pilote)
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

        const uint8_t size = JSON_OBJECT_SIZE(10);
        StaticJsonDocument<size> json;
        DeserializationError err = deserializeJson(json, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

        // On récupère les valeurs de notre joystick de gauche, on les stockes dans les variables globales joy_left_x et joy_left_y (accessibles de partout dans notre programme).
        if(json["joystick_left_x"] || json["joystick_left_y"]) // || correspondent à OU json c'est JavaScript Object Notation Cette ligne sert à verifier si l'utilisateur a bougé le joystick 
        {
          joy_left_x = json["joystick_left_x"];
          joy_left_y = json["joystick_left_y"];

          // On affiche leurs valeurs
          Serial.print("Joystick left x : ");
          Serial.println(joy_left_x);
          Serial.print("Joystick left y : ");
          Serial.println(joy_left_y);          
        }

        // On récupère les valeurs de notre joystick de droite, on les stockes dans les variables globales joy_right_x et joy_right_y (accessibles de partout dans notre programme).
        if(json["joystick_right_x"] || json["joystick_right_y"])
        {
          joy_right_x = json["joystick_right_x"];
          joy_right_y = json["joystick_right_y"];

          // On affiche leurs valeurs
          Serial.print("Joystick right x : ");
          Serial.println(joy_right_x);
          Serial.print("Joystick right y : ");
          Serial.println(joy_right_y);          
        }

    }
}

// On paramètre le comportement à avoir selon le type d'évenement websocket
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
    
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
    
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
    break;
    
    case WS_EVT_PONG:
      //Do something
    break;
    
    case WS_EVT_ERROR:
      //Do something
    break;
  }
}

// On déclare notre fonction permettant d'initialiser tout ce dont on a besoin
void initRemote()
{
  // On vérifie que la mémoire qui va stocker notre page Web est formatée (utilisable)
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    // Si ce n'est pas le cas, on formate
    bool formatted = SPIFFS.format();
    if(formatted){
      Serial.println("\n\nSuccess formatting");
    }
    else{
      Serial.println("\n\nError formatting");
    }
    return;
  }

  // On appelle la fonction initialisant la connexion WiFi
  initWifi();

  // On paramètre le serveur
  server.on("/", onRootRequest);
  server.serveStatic("/", SPIFFS, "/");

  // On lie le websocket au serveur
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // On démarre le serveur
  server.begin();
}

// On déclare la fonction permettant d'initialiser le WiFi
void initWifi()
{
    // On choisit le mode station, c'est à dire qui se connectera à un réseau existant
    WiFi.mode(WIFI_STA);
    
    // On indique le nom du réseau et le mot de passe
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");
    int timeout_counter = 0;

    // On vérifie que la connexion s'établit, au bout d'un certain temps on redémarre la carte si nous ne parvenons pas à nous connecter
    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(200);
        timeout_counter++;
        if(timeout_counter >= CONNECTION_TIMEOUT*5){
        ESP.restart();
      }
    }

    // Si la connexion est établie, on affiche l'adresse IP dans le moniteur série (elle nous permettra de nous connecter au serveur)
    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}
