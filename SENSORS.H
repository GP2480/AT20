#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ==========================================
// PIN-DEFINITIONS FÜR SENSOR-XSHUT
// ==========================================
const int TOF1_XSHUT = 20; // GP20 -> Shutdown Pin Sensor 1
const int TOF2_XSHUT = 21; // GP21 -> Shutdown Pin Sensor 2

// I2C Adressen für die beiden Sensoren
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// Extern deklarierte Instanzen aus der .ino
extern Adafruit_VL53L0X lox1;
extern Adafruit_VL53L0X lox2;

// ==========================================
// INITIALISIERUNG DER TOF SENSOREN
// ==========================================
// Gibt 'true' zurück, wenn BEIDE Sensoren erfolgreich erkannt wurden.
// Gibt 'false' zurück, wenn mindestens ein Sensor fehlt oder fehlerhaft ist.
inline bool initialisiereToFSensoren() {
  pinMode(TOF1_XSHUT, OUTPUT);
  pinMode(TOF2_XSHUT, OUTPUT);

  // Beide Sensoren zuerst in den Reset/Shutdown schicken
  digitalWrite(TOF1_XSHUT, LOW);
  digitalWrite(TOF2_XSHUT, LOW);
  delay(10);

  bool sensor1_ok = false;
  bool sensor2_ok = false;

  // Sensor 1 aktivieren & Adresse vergeben
  digitalWrite(TOF1_XSHUT, HIGH);
  delay(10);
  if (lox1.begin(LOX1_ADDRESS)) {
    sensor1_ok = true;
    Serial.println(F("ToF Sensor 1 erfolgreich initialisiert (0x30)."));
  } else {
    Serial.println(F("Fehler: ToF Sensor 1 nicht gefunden!"));
  }

  // Sensor 2 aktivieren & Adresse vergeben
  digitalWrite(TOF2_XSHUT, HIGH);
  delay(10);
  if (lox2.begin(LOX2_ADDRESS)) {
    sensor2_ok = true;
    Serial.println(F("ToF Sensor 2 erfolgreich initialisiert (0x31)."));
  } else {
    Serial.println(F("Fehler: ToF Sensor 2 nicht gefunden!"));
  }

  // Rückgabe: Nur true, wenn BEIDE Sensoren bereit sind
  return (sensor1_ok && sensor2_ok);
}

// ==========================================
// GEFILTERTE MESSUNG (ToF)
// ==========================================
// Gibt den Abstand in cm zurück.
// Liefert 999.0 zurück, wenn kein Objekt im Bereich (10-30 cm) ist
// oder wenn I2C-Lese-Fehler auftreten.
inline float messeToF_Gefiltert(Adafruit_VL53L0X &sensor, int sensorNummer) {
  VL53L0X_RangingMeasurementData_t measure;
  sensor.getRangingMeasurement(&measure, false);

  // RangeStatus != 4 bedeutet: Messung gültig
  // RangeMilliMeter > 0 fängt Fehlerwerte bei nicht angeschlossenen Sensoren ab
  if (measure.RangeStatus != 4 && measure.RangeMilliMeter > 0) {
    float dist = measure.RangeMilliMeter / 10.0; // Umrechnung mm -> cm

    // Fokus-Fenster: Nur Werte zwischen 10.0 cm und 30.0 cm akzeptieren
    if (dist >= 10.0 && dist <= 30.0) {
      return dist;
    }
  }

  // Kein Objekt im Target-Fenster oder Sensor-Fehler
  return 999.0; 
}

#endif // SENSORS_H
