# AT20 Automatisches Türsteuerungssystem

Dieses Projekt beinhaltet eine robuste, zustandsgesteuerte Ablaufsteuerung für ein automatisches Türsystem auf Basis der RP2040-Architektur. Das System kombiniert eine präzise Motorsteuerung per Dual-Channel-PWM, Echtzeit-Positionsrückmeldung über einen Inkrementalencoder, berührungslose Objekterkennung mittels eines Time-of-Flight (ToF) Sensors sowie optisches Feedback über zwei OLED-Display und ein dediziertes LED-Status-Framework. Die Automatik kann drahtlos via Funkkanäle oder per Taster aktiviert oder deaktiviert werden.

---

## 📊 Systemressourcen & Pin-Belegung

Das System verwaltet die Hardware-Ressourcen über dedizierte GPIO-Zuweisungen und Hardware-Interrupts, um einen blockierungsfreien Betrieb zu gewährleisten.

### GPIO-Pin-Mapping

| Peripherie / Modul | Pin / GPIO | Konfiguration | Beschreibung |
| :--- | :--- | :--- | :--- |
| **Motor R_PWM** | *R_PWM Pin* | `OUTPUT` | Rechtslauf des Motors (Tür SCHLIESSEN) |
| **Motor L_PWM** | *L_PWM Pin* | `OUTPUT` | Linkslauf des Motors (Tür ÖFFNEN) |
| **Motorencoder Phase A** | GPIO 2 | `INPUT_PULLUP` | Phase A (Löst Hardware-Interrupt aus) |
| **Motorencoder Phase B** | GPIO 3 | `INPUT_PULLUP` | Phase B (Wird zur Richtungserkennung ausgelesen) |
| **Taster / Funk B** | *TASTER_B / FUNK_B* | `INPUT_PULLUP` | Befehl: Motor Rechtslauf / Tür SCHLIESSEN |
| **Taster / Funk C** | *TASTER_C / FUNK_C* | `INPUT_PULLUP` | Befehl: Motor Linkslauf / Tür ÖFFNEN |
| **Taster / Funk D** | *TASTER_D / FUNK_D* | `INPUT_PULLUP` | Befehl: Motor STOPP / Fehler-Quittierung (Reset) |
| **Rote LED** | *ROTE_LED_PIN* | `OUTPUT` | ToF-Deaktivierungs-Anzeige (AN = ToF Deaktiviert) |
| **Grüne LED** | GPIO 8 | `OUTPUT` | ToF-Aktivierungs-Anzeige (AN = ToF Aktiv / Automatik bereit) |
| **Weiße LED** | GPIO 19 | `OUTPUT` | Unabhängige LED für Systemstart-Test & Fehler-Blinken |
| **I2C SDA** | *I2C SDA Pin* | `Communication` | Datenleitung für das u8g2 OLED-Display & den VL53L0X ToF-Sensor |
| **I2C SCL** | *I2C SCL Pin* | `Communication` | Taktleitung für das u8g2 OLED-Display & den VL53L0X ToF-Sensor |

### Genutzte Systemressourcen
* **Hardware-Interrupts (ISR):** Exklusiv auf GPIO 2 (Encoder Phase A) geschaltet. Dies garantiert, dass kein Encoder-Impuls verloren geht, selbst wenn der Hauptprozessor mit Display-Updates beschäftigt ist.
* **Hardware-Timer (`millis()`):** Das gesamte Projekt ist strikt blockierungsfrei aufgebaut. Zeitgesteuerte Prozesse (Entprellung, LED-Blinktakte, Totzeiten und der Blockier-Timeout) nutzen die Systemzeit anstelle von `delay()`.
* **I2C-Bus (Hardware):** Der I2C-Bus wird im Shared-Modus betrieben, um das OLED-Display und den VL53L0X-Sensor parallel mit hoher Taktrate zu aktualisieren.

---

## 📁 Dateistruktur & Funktionsübersicht

Das Projekt ist modular in mehrere Dateien unterteilt, um die Wartbarkeit zu maximieren:

### 1. `Nikis_Door_021026.ino` (Hauptprogramm)
Enthält das Kernprogramm, die Initialisierung (`setup()`) und die zentrale State-Machine (`loop()`).
* **Softwaregesteuerte Blockiererkennung:** Überwacht permanent den Bewegungszustand. Bleiben bei aktiver Motoransteuerung die Encoder-Impulse für mehr als **500 ms** aus, wird die Notabschaltung eingeleitet.
* **Funkgesteuerte ToF-Überwachung:** Liest einen als Kanal A gekennzeichneten Funkkanal aus. Damit wird die ToF-Objekterkennung dynamisch scharf oder inaktiv geschaltet. Eine rote LED zeigt den inaktiven Modus an und eine grüne LED zeigt den aktiven Modus entsprechend an.
* **Anti-Aliasing Filter:** Beinhaltet einen Plausibilitäts-Zähler für den ToF-Sensor. Ein Objekt muss mehrere Zyklen stabil erkannt werden, um Phasen-Spiegelungen resultierend aus Streulicht oder Staubaufwirbelungen aus größeren Distanzen (z. B. Wände bei 100 cm) herauszufiltern.

### 2. `motor_control.h` / `motor_control.cpp`
Dieses Modul beinhaltet die Ansteuerung des Motortreiberse.
* `motorenStoppen()`: Setzt beide PWM-Kanäle auf `0` (Motor rollt stromlos aus).
* `motorVollbremsung()`: Schaltet beide PWM-Kanäle gleichzeitig auf das Maximum (`255`), um die Wicklungen kurzzuschließen und den Motor sofort elektronisch zu blockieren.

### 3. `led_control.h` / `led_control.cpp`
Steuert das optische Signal-Framework für die bordeigene Diagnostik der internen LED.

### 4. `display_control.h` / `display_control.cpp`
Abstrahiert die u8g2-Grafikbibliothek für das OLED-Display.
* `aktualisiereOLED()`: Gibt den Systemstatus (`BEREIT`, `RECHTS`, `LINKS`, `OEFFNEN`, `SCHLIESS`, `BLOCKIERT!`) sowie den **aktuellen ToF-Funkstatus** (z. B. `ToF: AN` / `ToF: AUS`) auf dem Bildschirm aus.

---

## 🕹️ Bedienungsanleitung (Bedienkonzept)

### 1. Manuelle Steuerung (Taster & Funkkanäle)
* **Kanal B (Taster/Funk):** Startet den Motorlauf nach rechts. Die Tür wird **geschlossen** (OLED zeigt `SCHLIESS`).
* **Kanal C (Taster/Funk):** Startet den Motorlauf nach links. Die Tür wird **geöffnet** (OLED zeigt `OEFFNEN`).
* **Kanal D (Taster/Funk):** Stoppt jegliche Motorbewegung sofort. Die Tür verbleibt in der aktuellen Position.

### 2. Normalbetrieb & Automatische Öffnung (ToF-Sensor)
* Befindet sich das System im Zustand **`BEREIT`** und der Funk-Status steht auf Aktiv, leuchtet die **Grüne LED** (ToF aktiv) und die **Rote LED ist aus**. Der VL53L0X-Sensor überwacht nun die Umgebung.
* Nähert sich eine Person oder ein Objekt im Bereich zwischen **10 cm und 50 cm**, löst die Tür automatisch aus, öffnet sich und fährt nach Ablauf des Timers wieder zu.

### 3. Deaktivierung der Automatik per Funk (ToF Ein/Aus)
* Über die Fernbedienung kann die automatische ToF-Erkennung jederzeit deaktiviert werden.
* **Optische Anzeige:** Die **Rote LED schaltet sich EIN** (ToF inaktiv) und die **Grüne LED geht AUS**. Das OLED zeigt `ToF: AUS`.
* In diesem Zustand bleibt die Tür im Stillstand geschlossen und reagiert **nicht** auf Annäherungen. Die manuelle Steuerung über die Kanäle B, C und D bleibt voll funktionsfähig.

### 4. Verhalten im Fehlerfall (Motorblockierung)
Trifft die Tür während der Fahrt auf ein mechanisches Hindernis oder blockiert, greift die softwaregesteuerte Sicherheitsabschaltung:
1. **Abschaltung:** Der Motor wird nach exakt **500 ms** stromlos geschaltet oder aktiv gebremst, um Hardwareschäden zu verhindern.
2. **Display-Warnung:** Das OLED-Display zeigt permanent den Text **`BLOCKIERT!`** an.
3. **Optischer Alarm (Weiße LED an GPIO 19):** Die weiße LED beginnt in einem schnellen, hektischen Rhythmus (200 ms Takt) zu blinken. Das System ignoriert alle ToF-Sensoren und Fahrbefehle.

### 5. Fehler zurücksetzen (Reset / Quittierung)
* Um die Tür nach einer erkannten Blockierung wieder freizugeben, drücken Sie kurz den **Taster D** oder betätigen Sie den **Funkkanal D (Stopp)**.
* Das System setzt den Fehler-Zustand zurück, die weiße LED (GPIO 19) erlischt, und das Display springt zurück auf **`BEREIT`**.

### 6. Visueller Funktionstest beim Systemstart (POST)
* Sobald das System eingeschaltet wird, leuchtet die weiße LED an **GPIO 19 für 300 ms dauerhaft auf** und erlischt danach wieder.
* **Nutzen:** Dies dient als Power-On Self-Test (POST), um die Funktionstüchtigkeit der Alarm-LED zu überprüfen.
