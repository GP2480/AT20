// ==========================================
// SENSORS.H - TOF-SENSOREN MIT FENSTER-FILTER
// ==========================================
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_VL53L0X.h>

// I2C0 Pins für das OLED
const int OLED_SDA = 0; 
const int OLED_SCL = 1; 

// I2C1 Pins für die ToF-Sensoren
const int TOF_SDA = 26; // GP26
const int TOF_SCL = 27; // GP27

const int XSHUT_TOF1 = 14; 
const int XSHUT_TOF2 = 13;  

const uint8_t TOF1_ADDR = 0x30;
const uint8_t TOF2_ADDR = 0x31;

// =========================================================================
// NEU: CONFIG FÜR DAS SCHUTZ-FENSTER
// =========================================================================
const float TOF_MIN_DISTANZ = 10.0;   // Untere Grenze (alles darunter wird ignoriert)
const float TOF_MAX_DISTANZ = 30.0;   // Obere Grenze (alles darüber wird ignoriert)
const unsigned long TOF_FILTER_ZEIT = 200; // Zeit in ms, die das Objekt im Fenster sein muss

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_VL53L0X lox1;
extern Adafruit_VL53L0X lox2;

extern float tof1Abstand;
extern float tof2Abstand;
extern bool tofSensorenAktiv; // Steuerung per Funkkanal GP4

inline void initialisiereToFSensoren() {
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

inline float messeToF_Gefiltert(Adafruit_VL53L0X &sensor, int sensorNummer) {
  // Wenn per Funk abgeschaltet, I2C überspringen und Freigabe (999) senden
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

    // =========================================================================
    // FENSTER-LOGIK: Prüfen, ob der Wert im erlaubten Rahmen liegt
    // =========================================================================
    if (roheDistanz >= TOF_MIN_DISTANZ && roheDistanz <= TOF_MAX_DISTANZ) {
      if (aktuellerTimer == 0) {
        // Objekt hat das Fenster betreten -> Zeitmessung starten
        aktuellerTimer = millis();
      } else if (millis() - aktuellerTimer >= TOF_FILTER_ZEIT) {
        // Objekt blockiert das Fenster lang genug -> Wert ausgeben
        letzterWert = roheDistanz;
        return roheDistanz;
      }
      // Objekt ist im Fenster, aber die 200ms sind noch nicht um
      return letzterWert;
    }
  }

  // Weg ist frei, Messung ungültig oder außerhalb des Fensters -> Timer löschen
  aktuellerTimer = 0;
  letzterWert = 999.0;
  return 999.0;
}

#endif

