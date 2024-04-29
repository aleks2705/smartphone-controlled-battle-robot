//#include <ESP32Servo.h>
#include "Remote.h"
//#include "Arduino.h"

//#include <Servo.h>

const int PWM_channel_left = 0;     // On choisit le canal 0 pour le moteur de gauche
const int PWM_channel_right = 4;    // On choisit le canal 2 pour le moteur de droite
const int resolution = 8;           // Résolution de 8 bits, dans notre cas ça n'a pas ou peu d'importance
const int PWM_channel_monte_pince = 6;    // On choisit le canal 4 pour monter le moteur pince
const int PWM_channel_descend_pince = 8;     // On choisit le canal 5 pour descend le moteur pince

int frequence_left = 1000;          // Nous initialisons la fréquence PWM du moteur de gauche à 1 KHz.// Notre moteur fera donc 5 tours par seconde                                    
int frequence_right = 1000;         // Nous initialisons la fréquence PWM du moteur de droite à 1 KHz 

const int dir_pin_left = 5;        // Constante contenant le numéro de broche sur laquelle est branchée l'entrée DIR du driver
const int step_pin_left = 18;       // Constante contenant le numéro de broche sur laquelle est branchée l'entrée STEP du driver
const int en_pin_left = 19;         // Constante contenant le numéro de broche sur laquelle est branchée l'entrée ENABLE du driver

const int dir_pin_right = 21;        // Constante contenant le numéro de broche sur laquelle est branchée l'entrée DIR du driver
const int step_pin_right = 22;        // Constante contenant le numéro de broche sur laquelle est branchée l'entrée STEP du driver
const int en_pin_right = 23;         // Constante contenant le numéro de broche sur laquelle est branchée l'entrée ENABLE du driver

// Idem pour le joystick

const int seuil_X = 0; //Position 0 joy
const int seuil_Y = 0; //Position 0 joy

//const int SERVO = 12; // broche du servo
//Servo ServoMot;  //On definit le servomot



void setup(){
    // On initialise la communication série (baudrate = 115200)
    Serial.begin(115200);

    // On lance l'initialisation de tout ce qui est lié à la commande à distance (WiFi, websocket, serveur ...) 
    // N'oubliez pas de modifier le nom et le mot de passe de votre réseau dans le fichier "Remote.h"
    initRemote();
    // Nous paramétrons les broches permettant de piloter le moteur de gauche
    // On paramètre notre broche "DIR" en sortie
    pinMode(dir_pin_left,OUTPUT);
    // On paramètre notre broche "STEP" en sortie
    pinMode(step_pin_left,OUTPUT);    
    // On paramètre notre broche "EN" en sortie
    pinMode(en_pin_left,OUTPUT);
    // On désactive l'alimentation de notre moteur (souvenez-vous l'entrée Enable du driver est en logique inverse)
    digitalWrite(en_pin_left, HIGH);

    // Nous paramétrons les broches permettant de piloter le moteur de droite
    // On paramètre notre broche "DIR" en sortie
    pinMode(dir_pin_right,OUTPUT);
    // On paramètre notre broche "STEP" en sortie
    pinMode(step_pin_right,OUTPUT);    
    // On paramètre notre broche "EN" en sortie
    pinMode(en_pin_right,OUTPUT);
    // On désactive l'alimentation de notre moteur (souvenez-vous l'entrée Enable du driver est en logique inverse)
    digitalWrite(en_pin_right, HIGH);

    // On paramètre la sortie PWM qui pilote la broche "STEP" du moteur "left"
    ledcSetup(PWM_channel_left, frequence_left, resolution);
    // On lie la sortie PWM à la broche "STEP" du moteur "left"
    ledcAttachPin(step_pin_left, PWM_channel_left);
    // On paramètre la sortie PWM qui pilote la broche "STEP" du moteur "right"
    ledcSetup(PWM_channel_right, frequence_right, resolution);
    // On lie la sortie PWM à la broche "STEP" du moteur "left"
    ledcAttachPin(step_pin_right, PWM_channel_right);

    // On initialise la broche de sortie pour le moteur pince
    pinMode(PWM_channel_monte_pince,OUTPUT);
    pinMode(PWM_channel_descend_pince, OUTPUT);

    //ServoMot.attach(SERVO);
    
}

void loop(){
    int x_Value = analogRead(joy_left_x);
    int y_Value = analogRead(joy_left_y);
    int y2_Value = analogRead(joy_right_y);
    int angle_joy_x = map(x_Value, 0, 4096, -90, 90);
    int vitesse_moteur_y = map(y_Value, 0, 4096, -255, 255);
    int angle_canon = map(y2_Value, 0, 4096, 0, 180);

    //Vitesses moteurs
    int mot_gauche_speed = vitesse_moteur_y + angle_joy_x;
    int mot_droit_speed = vitesse_moteur_y - angle_joy_x;

    mot_gauche_speed = constrain(mot_gauche_speed, -255, 255);
    mot_droit_speed = constrain(mot_droit_speed, -255, 255);

    //Serial.print("X: ");
    //Serial.println(x_Value);
    //Serial.print(", Y: ");
    //Serial.println(y_Value);

    //Ne pas bouger
    if(joy_left_y == seuil_Y && joy_left_x == seuil_X){
        digitalWrite(en_pin_left,LOW);
        digitalWrite(en_pin_right,LOW);
        // Nous utilisons un délai afin de permettre le bon fonctionnement du programme
        // Sans ce-dernier celui-ci ne fonctionne pas convenablement
        delay(50);     
    } else if(joy_left_y < seuil_Y){ // Aller derrière
        // Nous modifions la fréquence de la broche PWM correspondant au moteur "left" et "right"
        ledcWriteTone(PWM_channel_left,frequence_left);
        ledcWriteTone(PWM_channel_right,frequence_right);
        // Nous activons l'alimentation électrique du moteur "left" et "right"
        digitalWrite(en_pin_left,LOW);
        digitalWrite(en_pin_right,LOW);
        // Conformément à ce qui est indiqué dans le diagramme qui vous est fourni, nous passons l'état de la broche
        // gérant le sens de rotation à "HIGH"
        digitalWrite(dir_pin_left,LOW);
        digitalWrite(dir_pin_right, HIGH);
        // Nous utilisons un délai afin de permettre le bon fonctionnement du programme
        // Sans ce-dernier celui-ci ne fonctionne pas convenablement
        delay(50);
    }  
    else if (joy_left_y > seuil_Y) { // Aller devant
        
         // Nous modifions la fréquence de la broche PWM correspondant au moteur "left" et "right"
        ledcWriteTone(PWM_channel_left,frequence_left);
        ledcWriteTone(PWM_channel_right,frequence_right);
        // Nous activons l'alimentation électrique du moteur "left" et "right"
        digitalWrite(en_pin_left,LOW);
        digitalWrite(en_pin_right,LOW);
        // Conformément à ce qui est indiqué dans le diagramme qui vous est fourni, nous passons l'état de la broche
        // gérant le sens de rotation à "HIGH"
        digitalWrite(dir_pin_left,HIGH);
        digitalWrite(dir_pin_right, LOW);
        // Nous utilisons un délai afin de permettre le bon fonctionnement du programme
        // Sans ce-dernier celui-ci ne fonctionne pas convenablement
        delay(50);

    }
    else if (joy_left_x < seuil_X) { // Aller gauche
        
         // Nous modifions la fréquence de la broche PWM correspondant au moteur "left" et "right"
        ledcWriteTone(PWM_channel_left,frequence_left);
        ledcWriteTone(PWM_channel_right,frequence_right);
        // Nous activons l'alimentation électrique du moteur "left" et "right"
        digitalWrite(en_pin_left,LOW);
        digitalWrite(en_pin_right,LOW);
        // Conformément à ce qui est indiqué dans le diagramme qui vous est fourni, nous passons l'état de la broche
        // gérant le sens de rotation à "HIGH"
        digitalWrite(dir_pin_left,HIGH);
        digitalWrite(dir_pin_right,HIGH);
        // Nous utilisons un délai afin de permettre le bon fonctionnement du programme
        // Sans ce-dernier celui-ci ne fonctionne pas convenablement
        delay(50);

    }
    else if (joy_left_x > seuil_X) { // Aller droite
        
          // Nous modifions la fréquence de la broche PWM correspondant au moteur "left" et "right"
        ledcWriteTone(PWM_channel_left,frequence_left);
        ledcWriteTone(PWM_channel_right,frequence_right);
        // Nous activons l'alimentation électrique du moteur "left" et "right"
        digitalWrite(en_pin_left,LOW);
        digitalWrite(en_pin_right,LOW);
        // Conformément à ce qui est indiqué dans le diagramme qui vous est fourni, nous passons l'état de la broche
        // gérant le sens de rotation à "HIGH"
        digitalWrite(dir_pin_left,LOW);
        digitalWrite(dir_pin_right,LOW);
        // Nous utilisons un délai afin de permettre le bon fonctionnement du programme
        // Sans ce-dernier celui-ci ne fonctionne pas convenablement
        delay(50);
    }
    
    //ServoMot.write(angle_canon);
    //delay(50); 
     
    
}
