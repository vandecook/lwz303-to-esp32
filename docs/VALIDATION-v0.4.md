# Softwarevalidierung v0.4.0

Stand: 07.11.2026. Zielgerät laut Nutzer: Geräte-Firmware 2.16. Prüfungen erfolgten ohne physisch angeschlossene Wärmepumpe, ohne Flashen und ohne Änderungen am GitHub-Repository oder an der HA-Installation.

## Ausgeführte Prüfungen

- ESPHome 2026.10.2: Individuelle YAML und SOL-Beispielkonfiguration mit ausschließlich synthetischen Zugangsdaten und lokaler Komponentenquelle validiert; C++-Codegenerierung für beide Konfigurationen erfolgreich. Persönliche Secrets wurden nicht zur Prüfung benötigt.
- C++-Codegenerierung mit aktivierter v0.4, nativen Number-/Select-Entitäten und beiden Decoder-Einstellungen erfolgreich.
- Hosttests kompilieren die tatsächlichen Dateien `lwz2.cpp`, `v03.cpp` und `v04.cpp` gegen einen kleinen ESPHome-/UART-/Preferences-Prüfrahmen. Bestehende v0.2-/v0.3-Transport- und Schreibtests bleiben Bestandteil.
- v0.4-Prüffälle: F5 erst ab 120 Sekunden, Abluft/Zuluft unabhängig, geplante Nachtabschaltung, Datenlücken, gespeicherte Fehler trotz wieder positiver Drehzahl, Erhalt nach ESP-Neustart, HA-Rücksetzung ohne Löschen der Historie und erneuter Fehlereintritt.
- Betriebsartenprüfungen: EE Tag/Nacht, manueller Betrieb mit P12, zeitweise LWZ-Übersteuerung aus F6, unbekannter Programmcode, Wechsel der Drehzahlquelle.
- F2-Prüfung für alle drei Decoderprofile, getrennte Heizungs-/Warmwasser-Rohcodes, SOL-Kommentartabelle und i-Rohcodeanzeige.
- P07/P08: Integer 0–3, Änderung ausschließlich des betreffenden Bytes im erhaltenen Block 17, unabhängiges Rücklesen; kein optimistischer Number-Erfolg.
- Uhr: gleicher Saisonstatus erzeugt keinen SET, Unsicherheit vor SET dauerhaft speichern, bestätigter Zustand nach Rücklesen, Erhalt nach Neustart, keine Wiederholung bei unterbrochener Transaktion, Ablehnung von Tageswechseln und Speicherfehlern; 32-Bit-Laufzeitüberlauf für die 120-Sekunden-Prüfung.
- Auswahl-/Konfigurationstest: ursprüngliche 80 Freigaben nachvollziehbar erhalten; P07/P08 durch bedienbare Numbers ersetzt. Alle konfigurierten Entitätsnamen tragen `device_name` genau einmal. Die reguläre GitHub-Quelle bleibt gesetzt.

## Vollständiger ESP32-Build

**Vollständiger ESP32-C3-Build erfolgreich**, ESPHome 2026.10.2 mit ESP-IDF 5.5.6 und GCC 14.2.0. Die finale Konfiguration wurde mit maximal zwei parallelen Compilerprozessen gebaut.

| Belegung | Ergebnis |
|---|---|
| RAM | 109.709 / 321.296 Byte, 34,1 % |
| Programm-Flash | 1.058.737 / 1.835.008 Byte, 57,7 % |

Geprüft wurde die individuelle ESP32-C3-Konfiguration mit synthetischen Zugangsdaten und lokalem Komponentenstand. Der Abgleich der in den Build kopierten C++-/Header-Dateien mit den ausgelieferten Quellen ist Bestandteil der Abschlussprüfung. Factory-/OTA-Binärdateien wurden lokal zur Buildprüfung erzeugt; sie enthalten Testzugangsdaten und werden deshalb nicht als einsatzfertiges Geräteimage ausgeliefert. Das Paket enthält den Quelltext und eine lokale Beispielkonfiguration zum eigenen Bauen.

Die anfängliche Blockade durch eine abweichende Windows-Cache-Pfadzuordnung wurde durch eine zusätzliche ESP32-C3-Registrierung im vorhandenen ESP-IDF-Cache behoben; die ursprünglichen Metadaten wurden gesichert. Die Korrektur betrifft die lokale Buildumgebung, nicht das LWZ-Protokoll.

## Verbleibende Geräte- und Systemprüfung

- Istdrehzahlquelle und Einheiten, F6-Übersteuerungspriorität und EE-Modi auf der tatsächlichen FW 2.16 gegen Geräteanzeige/Rohtelegramme prüfen. Die vorhandenen Nutzerbeobachtungen zeigen auffällige E8-Nullwerte; sGlobal ist deshalb die Standardquelle. 30 Sekunden maximale Datenalterung ist ein konfigurierbarer Software-Ausgangswert.
- F2-Anforderungscodes für i und SOL physisch prüfen. Die i-Zuordnung bleibt bis zum Nachweis roh; SOL-Texte sind FHEM-Kommentare, keine gemessenen Heizleistungen.
- Keine belastbare autonome LWZ-Neustartkennung belegt. Rücksetzung wird ausdrücklich durch die eigene HA-Automation nach erfolgtem Neustart übermittelt. Die bereitgestellte Taste ändert nur lokale Statuswerte.
- Physische P07/P08- und vorhandene Sollwert-/Uhr-Schreibwege prüfen. Rücklesen eines Blocks und dessen erhaltene Felder ist implementiert; reale Regelungsreaktion und Hardwarefehlerfälle sind noch zu prüfen.
- Dauerhaftigkeit auf realem ESP-Flash, Erkennungsverzögerung unter serieller Last, Kommunikation und Langzeitbetrieb prüfen. Speicherfehler werden als Diagnose angezeigt; simulierte Speicherung ersetzt keinen Stromausfalltest am Gerät.
- HA-Benachrichtigungsvorlage, Entitätsregistrierung, optionale Entitäten und bestehende Automationen im eigenen HA prüfen. HA-Versionen und bereits gespeicherte Entitätskennungen können Anzeigenamen beeinflussen. Es wurde keine reale HA-Automation angelegt oder verändert.
- Für die individuelle `lwz303.yaml` alle eigenen `!secret`-Verweise auflösen. Die Komponente wurde mit Platzhaltern validiert; keine tatsächlichen Zugangsdaten oder Geheimnisse sind im Paket enthalten.

Die v0.4-Software ist lokal erstellt. Softwareprüfung ist von Hardwarefreigabe, Veröffentlichung und dem Flashen des eigenen Geräts getrennt.
