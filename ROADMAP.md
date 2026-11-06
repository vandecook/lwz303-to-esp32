# Roadmap – LWZ303 to ESP32

Stand 2026-11-06. Geräte-Firmware laut Nutzer 2.16; separate Decoderlayouts 2.06 / 2.14 / 2.14j. Softwareintegration v0.3 implementiert, Hardwareprüfung offen.

| Version | Inhalt | Status |
|---|---|---|
| 0.1 | Basiskommunikation, 9600 8N1, Pegelwandler, RX5/TX6 im getesteten Aufbau, Profile, Retry-/Nonblocking-Logik | Vom Anwender als funktionierend bestätigt |
| 0.2 | Basiswerte, Diagnose, HK1 P13/P14 lesend, 10/5-s-Dynamik, 60-min-Konfiguration, persistentes lokales Webinterface und Serviceaktionen | Implementiert; neue Hardwarefunktionen noch zu testen |
| 0.3 | Benutzerauswahl: 33 Ja + 47 Optional, sieben Statusblöcke, Parameter 17, P01/P02/P04/P05 und Uhr ±1h schreiben, Rücklesen, unabhängige Decoderlayouts | Software implementiert; Hardware-/ESP32-Build offen |
| 0.4 | Heizungs-/WW-Zeitprogramme mit Wochentagen und Zeitfenstern lesen; Statusblöcke bereits v0.3 | Geplant |
| 0.5 | Nach Vorziehen der Uhr-Schreibfunktion auf v0.3 neu zu bestimmen | Offen |
| 0.6 | Gradient/Fußpunkt schreiben; Tag-/Nacht-Sollwerte bereits v0.3 | Geplant |
| 0.7 | Warmwasserzeitprogramme lesen/bearbeiten; Tag-/Nacht-Sollwerte bereits v0.3 | Geplant |
| 0.8 | Lüftung Tag/Nacht Stufe 0–3, aktuelle Stufe, Zeitprogramme lesen/schreiben | Geplant |
| 0.9 | Beide Ventilatoren getrennt Running/Failed/Off/Unknown; stufenabhängige Drehzahlgrenzen, Verzögerung, verifizierte F5/F6-Zuordnung, Fehlerzähler/-zeiten und Benachrichtigungen | Geplant |
| 0.10 | Verdichterstarts/-laufzeiten, Zusatzheizungsstunden soweit verfügbar, Sensorplausibilität und Auffälligkeiten mit Zeitpunkten | Geplant |
| 0.11 | Heizungszeitprogramme mit Wochentagen/Start/Ende in HA bearbeiten, dauerhaft in LWZ speichern, validieren und zurücklesen | Geplant |

## Übergreifende Anforderungen

- Profile und Verfügbarkeiten getrennt prüfen; keine erfundenen Register/P-Nummern.
- Parameterkatalog schrittweise erstellen: Beschreibung, P-Nummer falls vorhanden, Einheit, MDI-Icon, Firmwarevergleich, Schreibbarkeit und Zielversion.
- HA-Entitäten einheitlich benennen und als Basis, Expert oder Diagnose einordnen.
- Manuelle LWZ-Schreibfunktionen ab v0.3; Gerätevalidierung, vorgeschlagene Grenzen und Rücklesekontrolle bleiben erforderlich.
- v0.9 ohne automatischen Reset; bestehender Fingerbot bleibt unabhängig.
- Echo II bleibt optional und unabhängig für eine spätere Version.
- Energieeffizienz bleibt in der separaten Home-Assistant-Lösung außerhalb dieser Integration.
