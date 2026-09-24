// ==========================================
// MOTOR_CONTROL.H - ANTRIEB, ENCODER & LED
// ==========================================
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Pin-Definitionen für Motortreiber
const int R_PWM = 16; 
const int L_PWM = 17; 

// Endschalter-Pins
const int END_R = 2;  
const int END_L = 3;  

// Encoder-Pin am Pico
const int ENCODER_PIN = 22; 

// Taster-Pins
const int TASTER_WEISS = 19; 
const int TASTER_BLAU  = 20; 
const int TASTER_GRUEN = 21; 

// Globale Variablen (Deklaration)
extern volatile long encoderImpulse;
extern const float IMPULSE_PRO_UMDREHUNG; 
extern long zielImpulse; 

extern int blinkAnzahlZiel;
extern int blinkAktuell;
extern bool ledStatus;
extern unsigned long ledTimer;
extern const long BLINK_INTERVALL;

// Motoren stoppen
inline void motorenStoppen() {
  analogWrite(R_PWM, 0);
  analogWrite(L_PWM, 0);
}

// Hintergrund-Blinken starten
inline void starteBlinken(int anzahl) {
  blinkAnzahlZiel = anzahl * 2; 
  blinkAktuell = 0;
  ledStatus = true;
  digitalWrite(LED_BUILTIN, HIGH);
  ledTimer = millis();
}

// LED-Status aktualisieren
inline void aktualisiereLED() {
  if (blinkAnzahlZiel > 0) {
    if (millis() - ledTimer >= BLINK_INTERVALL) {
      ledTimer = millis();
      ledStatus = !ledStatus;
      digitalWrite(LED_BUILTIN, ledStatus);
      
      blinkAktuell++;
      if (blinkAktuell >= blinkAnzahlZiel) {
        blinkAnzahlZiel = 0; 
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }
}

#endif
