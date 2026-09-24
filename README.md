---

# Projekt: Nikis Finest Automatic Door
v24_09_26_gp
# Eine intelligente, mikrocontrollergestützte Türsteuerung für automatische Schiebetüren basierend auf dem RP2040.

---

## 📌 GPIO-Pinbelegung (RP2040 W)


| GPIO Pin  | Signal / Komponente            | Modus          | Funktion / Beschreibung                      |

| **GP2**   | Motortreiber R_PWM             | `OUTPUT`       | PWM-Signal für Rechtslauf                    |
| **GP3**   | Motortreiber L_PWM             | `OUTPUT`       | PWM-Signal für Linkslauf                     |
| **GP4**   | Endschalter Rechts (`END_R`)   | `INPUT_PULLUP` | Schließt gegen GND bei Kontakt rechts        |
| **GP5**   | Endschalter Links (`END_L`)    | `INPUT_PULLUP` | Schließt gegen GND bei Kontakt links         |
| **GP6**   | Drehgeber CLK                  | `INPUT_PULLUP` | Impulskanal A für Geschwindigkeitswahl       |
| **GP7**   | Drehgeber DT                   | `INPUT_PULLUP` | Impulskanal B für Drehrichtungserkennung     |
| **GP8**   | Taster Weiß                    | `INPUT_PULLUP` | Manuelle Fahrt nach Links (gegen GND)        |
| **GP9**   | Taster Blau                    | `INPUT_PULLUP` | Manuelle Fahrt nach Rechts (gegen GND)       |
| **GP10**  | Funkmodul D1                   | `INPUT`        | Funkkanal Fahrt Links (aktiv `HIGH`)         |
| **GP11**  | Funkmodul D2                   | `INPUT`        | Funkkanal Fahrt Rechts (aktiv `HIGH`)        |
| **GP12**  | Funkmodul D0                   | `INPUT`        | Funkkanal Not-Stopp (aktiv `HIGH`)           |
| **GP13**  | Funkmodul D3                   | `INPUT`        | Funkkanal ToF Automatik Toggle (aktiv HIGH)  |
| **GP14**  | Taster Grün                    | `INPUT_PULLUP` | Manueller Not-Stopp (gegen GND)              |
| **GP15**  | Motor-Encoder                  | `INPUT_PULLUP` | Hall-Sensor / Inkrementalgeber (Interrupt)   |
| **GP16**  | OLED / I2C SDA                 | `I2C`          | SDA für SH1106 Display & ToF-Sensoren        |
| **GP17**  | OLED / I2C SCL                 | `I2C`          | SCL für SH1106 Display & ToF-Sensoren        |
| **GP20**  | ToF 1 XSHUT                    | `OUTPUT`       | Shutdown-Pin zur I2C-Adressvergabe Sensor 1  |
| **GP21**  | ToF 2 XSHUT                    | `OUTPUT`       | Shutdown-Pin zur I2C-Adressvergabe Sensor 2  |

---

## 🚀 Hauptfunktionen & Features

### 1. **Referenzfahrt (Homing beim Systemstart)**

* Beim Einschalten führt die Steuerung automatisch eine langsame **Sicherheitsfahrt nach rechts** aus, bis der
* rechte Endschalter ausgelöst wird.
* Setzt den Nullpunkt für den Positionscoder und wechselt anschließend in den betriebsbereiten Zustand (`IDLE`).

### 2. **Automatikbetrieb via ToF-Sensoren (Time-of-Flight)**

* **Zwei VL53L0X Laser-Distanzsensoren** erfassen Objekte in einem präzisen Nahbereich (10 cm bis 30 cm).
* Erkanntes Objekt löst die automatische Türöffnung/Schließung aus, die sicher bis zum jeweiligen Endschalter fährt.
* **Sicherheits-Feature:** Sensoren werden beim Booten automatisch auf Erreichbarkeit geprüft. Fehlen die Sensoren,
* wird die Automatik deaktiviert, um unkontrollierte Motorfahrten zu verhindern.

### 3. **Manuelle Steuerung & Funkfernbedienung**

* **3 Taster / 4-Kanal-Funkmodul (parallel geschaltet):**
* **Taste Weiß / Funk D1:** Manuelle Fahrt nach Links (fährt eine definierte Anzahl an Encoder-Umdrehungen).
* **Taste Blau / Funk D2:** Manuelle Fahrt nach Rechts (fährt eine definierte Anzahl an Encoder-Umdrehungen).
* **Taste Grün / Funk D0:** **Sofort-Stopp / Not-Aus** in jedem Betriebszustand.
* **Funk D3 (Erweiterung):** Ein-/Ausschalten der ToF-Automatik (z. B. zum Offenhalten der Tür oder beim Reinigen).


### 4. **Geschwindigkeitsregelung per Drehgeber**

* Über einen **Rotary Encoder** kann die Motorgeschwindigkeit live während der Fahrt oder im Leerlauf angepasst werden.
* **Regelbereich:** Feinstufig von **30 % bis 80 % PWM** (in 2er-Schritten), um eine Überlastung oder zu schnelles Anschlagen der Tür zu vermeiden.

### 5. **OLED-Statusdisplay (SH1106 I2C)**

* **Echtzeit-Anzeige auf dem Display:**
* Aktueller Systemstatus (`BEREIT`, `T-RECHTS`, `GAR. OEFFNEN`, etc.)
* Gemessene Motorumdrehungen seit Start
* Distanzwerte der beiden ToF-Sensoren (oder Status `[INAKTIV]`)
* Aktuell eingestellte Motorgeschwindigkeit in %
* Schaltzustand beider Endschalter (Frei / Aktiv)


### 6. **Hardware & Sicherheit**

* **Inkrementallgeber / Hall-Encoder:** Exakte Erfassung der Motorumdrehungen.
* **Entprellung:** Softwareseitige Entprellung aller Taster, Endschalter und Funkkanäle ohne blockierende `delay()`-Schleifen.
* **Optisches Feedback:** Status-Blinken über die integrierte Board-LED bei Zustandswechseln.
