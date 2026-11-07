# Roadmap – LWZ303 to ESP32

Stand: **07.11.2026**. Geräte-Firmware laut Nutzer: **2.16**. Die Einstellungen **2.06 / 2.14 / 2.14j** bezeichnen getrennte FHEM-Decoderlayouts.

**Aktueller Stand:** v0.3 ist im lokalen Projekt als Software umgesetzt. Die freigegebene Auswahl umfasst **33 Ja + 47 Optional = 80 Einträge**: 78 Werte und zwei manuelle Uhraktionen. Softwaretests und ESPHome-Codegenerierung sind erfolgt; vollständiger ESP32-Build und Hardwareprüfung stehen noch aus.

## Versionsübersicht

| Version | Umgesetzter bzw. verbleibender Umfang | Status |
|---|---|---|
| 0.1 | Serielle Basiskommunikation, 9600 8N1, Pegelwandler, RX5/TX6 im bestätigten Aufbau, Decoderprofile, Retry-/Nonblocking-Logik | Basiskommunikation vom Anwender als funktionierend bestätigt |
| 0.2 | Basiswerte, Kommunikationsdiagnose, P13/P14 lesend, dynamische Abfrageintervalle, lokale Webbedienung und Serviceaktionen | Software umgesetzt; in v0.3 weitergeführt. Neue Hardwarefunktionen nicht vollständig geprüft |
| **0.3** | **Freigegebene Parameterauswahl aus sieben Statusblöcken, Parameterblock 17, eindeutige Hauptquellen, optionale Entitäten, Speed200-Analysewerte, P01/P02/P04/P05 schreiben und Uhr manuell ±1h mit Rücklesen** | **Software umgesetzt und synthetisch getestet; ESP32-Build und Geräteprüfung offen** |
| 0.4 | Heizungs- und Warmwasser-Wochenzeitprogramme mit Wochentagen und Zeitfenstern lesen | Geplant. Die sieben Statusblöcke wurden bereits nach v0.3 vorgezogen |
| 0.5 | Nach Vorziehen der ersten Schreibfunktionen auf v0.3 neu zu bestimmen | Offen; kein neuer Umfang festgelegt |
| 0.6 | Heizkurven-Gradient und Fußpunkt schreiben | Geplant. Raum-Sollwerte Tag/Nacht bereits in v0.3 umgesetzt |
| 0.7 | Warmwasser-Zeitprogramme bearbeiten und schreiben | Geplant. Warmwasser-Sollwerte Tag/Nacht bereits in v0.3 umgesetzt; Lesen der Wochenzeitprogramme gehört zu v0.4 |
| 0.8 | Lüfterstufen Tag/Nacht und aktuelle Stufe bedienen, Lüftungszeitprogramme lesen/schreiben | Geplant. Ausgewählte Lüftungsdaten werden bereits in v0.3 gelesen; keine Lüftersteuerung in v0.3 |
| 0.9 | Wohnraumlüftung analysieren: beide Ventilatoren getrennt bewerten, Running/Failed/Off/Unknown, stufenabhängige Grenzwerte, Verzögerung, F5/F6-Zuordnung, Fehlerzähler/-zeiten und Benachrichtigungen | Geplant. Speed200-Kalibrierkennwerte sind als Datengrundlage bereits in v0.3 enthalten; die Auswertung ist noch nicht umgesetzt |
| 0.10 | Zusätzliche Betriebsdaten und Überwachung: Verdichterstarts/-laufzeiten, Zusatzheizungsstunden soweit belegt, Sensorplausibilität und Auffälligkeiten mit Zeitpunkten | Geplant. Ausgewählte sHistory-Zähler und blockweise Datenalterung bereits in v0.3 enthalten; daraus werden keine unbelegten Verdichterstarts abgeleitet |
| 0.11 | Heizungszeitprogramme in Home Assistant bearbeiten, dauerhaft speichern, validieren und zurücklesen | Geplant. Lesen der Wochenzeitprogramme gehört zu v0.4 |

## Bereits umgesetzt

### v0.2 – Basis und Kommunikation

- Außen-, Vorlauf-, Rücklauf- und Warmwassertemperatur; Verdichter und Pumpenstatus. „Heizbetrieb“ bzw. „Warmwasserbereitung“ beruhen auf den Pumpenbits und sind keine eigenständigen Nachweise aktiver Wärmeerzeugung.
- HK1-Gradient **P13** und Fußpunkt **P14** lesen; Konfiguration manuell aktualisieren.
- Normalintervall 10 s, dynamisch 5 s und Konfigurationsintervall 60 min als Standard. Intervalle, Dynamik und ODER-Trigger für Verdichter/Heizung/Warmwasser einstellbar.
- Erfolgs-, Fehler-, Timeout- und Retryzähler, Kommunikationsstatus und letzte Aktualisierung; Zähler zurücksetzen und RS232 neu initialisieren.
- Lokales Webinterface, persistente lokale Einstellungen sowie gesonderte Bestätigung vor ESP-Neustart.

### v0.3 – Gewählte Werte und Darstellung

- Die ausgewählten Felder aus **sGlobal (FB), sDHW (F3), sFan (E8), sHC1 (F4), sHistory (09), sProgram (EE) und sTimedate (FC)** lesen. Zusätzlich Einstellungen aus **p01-p12 (17)** und P13/P14 aus **pHeat1 (05)**.
- Alle **Ja- und Optional-Einträge** der angepassten Auswahl integrieren. Nicht gewählte Werte werden in den mitgelieferten Konfigurationen nicht angelegt. Auswahl, Dateihash und die letzte Chatkorrektur sind in [selection.json](components/lwz2/selection.json) dokumentiert.
- Die **47 optionalen Entitäten** sind mit eingebaut, in Home Assistant zunächst deaktiviert und dort ohne Firmwareupdate aktivierbar. Das Geräteweb hat Basis- und Expert-Gruppen; ein eigener Schalter zum Ausblenden der Webgruppe ist nicht umgesetzt.
- **Doppelte Werte nur einmal veröffentlichen:** gemeinsame Temperaturen und Aktorzustände stammen aus sGlobal. sHC1 überschreibt sie nicht zusätzlich. Unterschiedliche Raumtemperaturquellen und aktive gegenüber gespeicherten Sollwerten bleiben getrennt.
- Globales Decoderprofil und HK1-Layout unabhängig wählen. Standard: Global **2.14j**, HK1 **2.06**. Fehlende layoutabhängige Werte bleiben unbekannt; keine pauschale Freigabe eines Profils für Geräte-Firmware 2.16.
- Fehlende bzw. veraltete Daten blockweise ungültig setzen, statt ältere Werte oder erfundene Nullwerte weiterzuverwenden.
- Ausgewählte sFan-Ist- und Soll-Drehzahlwerte optional bereitstellen. Decoderzuordnung und physikalische Plausibilität bleiben zu prüfen.
- **OUT063 / OUT064 als Ja, standardmäßig aktiv:** `sFan.supplyFanSpeed200` und `sFan.exhaustFanSpeed200` für die Wohnraumlüftungsanalyse. FHEM-Skalierung unsigned 16 Bit Big Endian /10, eine Nachkommastelle. Physikalische Einheit und Bedeutung des Kalibrierpunkts sind noch nicht bestätigt; keine Umdeutung in U/min oder 1/s.
- Vorhandene sHistory-Zähler unter ihren belegten Feldnamen lesen; ungesicherte Aggregatzuordnungen nicht ergänzen.

### v0.3 – Erste manuelle Schreibfunktionen

- **P01/P02:** Raum-Solltemperaturen Tag und Nacht lesen und manuell ändern.
- **P04/P05:** Warmwasser-Solltemperaturen Tag und Nacht lesen und manuell ändern.
- **Geräteuhr manuell +1h / −1h:** frischen Uhrblock lesen und die absolute Zielstunde schreiben; keine automatische Synchronisation durch SNTP.
- Schreibfreigabe nach jedem Neustart aus. Pro Auftrag vollständigen Block frisch lesen, nur das Zielfeld ändern und anschließend unabhängig zurücklesen. Zielwert und erhaltene Felder prüfen.
- Bei unklarer Schreibantwort keinen SET-Befehl blind wiederholen; zunächst den tatsächlichen Wert zurücklesen. Mehrfachklicks auf Uhraktionen sperren.
- Vorgeschlagene Bediengrenzen: Raum 15–25 °C, Warmwasser 35–55 °C, Raster 0,1 °C. Dies sind keine bestätigten Herstellergrenzen der Geräte-Firmware 2.16.
- **Tageswechsel werden abgelehnt.** Datum, Jahr und Wochentag werden nicht geschrieben; Kalenderunterstützung und automatische Zeitumstellung sind nicht umgesetzt.

## Als Nächstes: v0.3 am Gerät prüfen

- Vollständigen ESP32-C3-Build einschließlich Flash-/RAM-Verbrauch prüfen und die neue Version am konkreten Gerät testen.
- Feldmappings gegen frische Rohtelegramme und Geräteanzeige abgleichen, insbesondere HK1-Raum-Sollwert, zusätzliche Innentemperatur, Zusatzheizungsstufen und die ausgewählten sFan-/Speed200-Werte.
- Vier Sollwert-Schreibwege und beide Uhraktionen mit Rücklesekontrolle am Gerät testen. Tatsächlich erlaubte Gerätebereiche verifizieren; bisherige UI-Grenzen bleiben Vorschläge.
- Home-Assistant-Aktivierung optionaler Entitäten, Webgruppen, Neustartverhalten, Verbindungsunterbrechung und Langzeitbetrieb prüfen.
- In der individuellen `lwz303.yaml` die Secret-Verweise auf dem eigenen ESPHome-Host auflösen; lokal fehlt derzeit `api_encryption_key`. Die übrige Konfiguration wurde mit Prüfplatzhaltern validiert.

Prüfstand und Grenzen: [Softwarevalidierung v0.3](docs/VALIDATION-v0.3.md). Erfolgreiche Softwaretests sind kein abgeschlossener Hardwaretest.

## Übergreifende Anforderungen

- Geräte-Firmware und Decoderlayout getrennt dokumentieren; keine erfundenen Register, P-Nummern, Einheiten oder Grenzwerte.
- Parameterkatalog und einfache Auswahltabelle bei neuen Erkenntnissen fortführen. Messbeobachtung, Decoderbeleg und Hardwareprüfung getrennt kennzeichnen.
- Eindeutige Entitäten und Hauptquellen; keine zusätzlichen Ausgaben allein aufgrund mehrfacher Datenblöcke.
- Keine automatische Heizungs-/Warmwasserregelung durch die neuen manuellen Schreibfunktionen.
- Lüftungsdiagnose v0.9 ohne automatischen Fehlerreset; bestehender Fingerbot bleibt unabhängig.
- Echo II bleibt eine unabhängige Option für eine spätere Version.
- Energieeffizienz bleibt in der separaten Home-Assistant-Lösung außerhalb dieser Integration.
