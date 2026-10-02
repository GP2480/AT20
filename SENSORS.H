#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_VL53L0X.h>

// Pins für I2C0 (Display) - Passend zur Hauptdatei
#define OLED_SDA 0
#define OLED_SCL 1

// Pins für I2C1 (ToF-Sensoren)
#define TOF_SDA 26 
#define TOF_SCL 27 

#define XSHUT_TOF1 14 
#define XSHUT_TOF2 13  

#define TOF1_ADDR 0x30
#define TOF2_ADDR 0x31

#define TOF_DISABLE_PIN 4  // GPIO4: Impuls-Eingang vom Funkmodul (Aktiv HIGH)
#define TOF_STATUS_LED 28  // GPIO28: LED für Inaktivitäts-Anzeige

// Filter-Konfiguration für das Schutz-Fenster
const float TOF_MIN_DISTANZ = 10.0;   // Untere Grenze in cm
const float TOF_MAX_DISTANZ = 30.0;   // Obere Grenze in cm
const unsigned long TOF_FILTER_ZEIT = 200; // Zeit in ms

// Variablen sind in der Hauptdatei bereits definiert (verhindert Redefinition Errors)
extern bool tofSensorenAktiv; 
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_VL53L0X lox1;
extern Adafruit_VL53L0X lox2;

// Blitzschnelle Hardware-Interrupt-Service-Routine (ISR) für den Funk-Toggle
void funkToF_Interrupt() {
  static unsigned long letzteInterruptZeit = 0;
  unsigned long aktuelleZeit = millis();

  // 350 ms Software-Entprellung direkt im Interrupt
  if (aktuelleZeit - letzteInterruptZeit > 350) {
    tofSensorenAktiv = !tofSensorenAktiv; // Zustand umkehren
    
    // LED an GP28 direkt im Interrupt mitschalten für minimale Latenz
    // LED leuchtet auf (HIGH), wenn ToF inaktiv (false) ist.
    digitalWrite(TOF_STATUS_LED, !tofSensorenAktiv ? HIGH : LOW);
    
    letzteInterruptZeit = aktuelleZeit; // Buchstabendreher korrigiert!
  }
}

// Diagnose-Funktion für das OLED (Y-Koordinate auf Pixel 58 angepasst)
void zeigeGPIO4StatusAufOLED() {
 // u8g2.setFont(u8g2_font_6x10_tr);
  
  // Löscht gezielt den schmalen Bereich der Zeile 58
 // u8g2.setDrawColor(0);
 // u8g2.drawBox(0, 51, 128, 8);
//  u8g2.setDrawColor(1);
  
  // Live-Signal am Pin
 // u8g2.drawStr(0, 58, "GP4:");
 // u8g2.setCursor(28, 58);
//  u8g2.print(digitalRead(TOF_DISABLE_PIN) == HIGH ? "HIGH" : "LOW ");
  
  // Gespeicherter Modus
 // u8g2.drawStr(60, 58, "ToF:");
 // u8g2.setCursor(88, 58);
 // u8g2.print(tofSensorenAktiv ? "AKTIV" : "INAKT");
//  digitalWrite(TOF_STATUS_LED, !tofSensorenAktiv ? HIGH : LOW);
}

  


// Initialisierung der Sensoren und Aktivierung des Interrupts
inline void initialisiereToFSensoren() {
  // GP28 als digitalen Ausgang für die LED definieren
  pinMode(TOF_STATUS_LED, OUTPUT);
  // Da ToF beim Start standardmäßig AKTIV ist, bleibt die Warn-LED zuerst aus
  digitalWrite(TOF_STATUS_LED, LOW); 

  // INPUT_PULLDOWN, da das Funkmodul aktiv HIGH liefert!
  pinMode(TOF_DISABLE_PIN, INPUT_PULLDOWN); 

  // Registriere den Hardware-Interrupt auf GPIO4 bei einer steigenden Flanke
  attachInterrupt(digitalPinToInterrupt(TOF_DISABLE_PIN), funkToF_Interrupt, RISING);

  pinMode(XSHUT_TOF1, OUTPUT);
  pinMode(XSHUT_TOF2, OUTPUT);

  digitalWrite(XSHUT_TOF1, LOW);
  digitalWrite(XSHUT_TOF2, LOW);
  delay(20);

  Wire1.setSDA(TOF_SDA);
  Wire1.setSCL(TOF_SCL);
  Wire1.begin();

  // Sensor 1 (Rechts)
  digitalWrite(XSHUT_TOF1, HIGH); 
  delay(20);
  if (lox1.begin(TOF1_ADDR, false, &Wire1)) { 
    lox1.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_DEFAULT);
  } else {
    Serial.println(F("Fehler: ToF 1 fehlt!"));
  }

  // Sensor 2 (Links)
  digitalWrite(XSHUT_TOF2, HIGH); 
  delay(20);
  if (lox2.begin(TOF2_ADDR, false, &Wire1)) { 
    lox2.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_DEFAULT);
  } else {
    Serial.println(F("Fehler: ToF 2 fehlt!"));
  }
}

// Filter-Funktion für die ToF-Sensoren
inline float messeToF_Gefiltert(Adafruit_VL53L0X &sensor, int sensorNummer) {
  // Wenn der Modus durch den Hardware-Interrupt auf false gesetzt wurde, Messung sofort abbrechen
  if (!tofSensorenAktiv) {
    return 999.0; 
  }

  static unsigned long tof1_Timer = 0;
  static unsigned long tof2_Timer = 0;
  static float lastValid1 = 999.0;
  static float lastValid2 = 999.0;

  unsigned long &aktuellerTimer = (sensorNummer == 1) ? tof1_Timer : tof2_Timer;
  float &letzterWert = (sensorNummer == 1) ? lastValid1 : lastValid2;

  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false);
  
  if (measure.RangeStatus != 4) { 
    float roheDistanz = measure.RangeMilliMeter / 10.0; // in cm

    if (roheDistanz >= TOF_MIN_DISTANZ && roheDistanz <= TOF_MAX_DISTANZ) {
      if (aktuellerTimer == 0) {
        aktuellerTimer = millis();
      } else if (millis() - aktuellerTimer >= TOF_FILTER_ZEIT) {
        letzterWert = roheDistanz;
        return roheDistanz;
      }
      return letzterWert;
    }
  }

  aktuellerTimer = 0;
  letzterWert = 999.0;
  return 999.0;
}

#endif


