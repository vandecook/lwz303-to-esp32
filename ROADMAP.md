# Roadmap – LWZ303 to ESP32

Stand 2026-11-03. Firmwareprofile 2.06 / 2.14 / 2.14j. Inhalte ab v0.3 sind geplant und setzen verifizierte Protokollunterstützung voraus.

| Version | Inhalt | Status |
|---|---|---|
| 0.1 | Basiskommunikation, 9600 8N1, Pegelwandler, RX5/TX6 im getesteten Aufbau, Profile, Retry-/Nonblocking-Logik | Vom Anwender als funktionierend bestätigt |
| 0.2 | Basiswerte, Diagnose, HK1 P13/P14 lesend, 10/5-s-Dynamik, 60-min-Konfiguration, persistentes lokales Webinterface und Serviceaktionen | Implementiert; neue Hardwarefunktionen noch zu testen |
| 0.3 | sGlobal vollständig erfassen; sinnvolle HA-Entitäten anhand Parameterkatalog auswählen, Basis/Expert/Diagnose | Geplant |
| 0.4 | sHC1, Warmwasser, sFAN sowie Heizungs-/WW-Zeitprogramme mit Wochentagen und Zeitfenstern lesen | Geplant |
| 0.5 | Erste LWZ-Schreibfunktion: Uhr manuell +1/-1 Stunde, Rücklesen und Mehrfachauslösung verhindern; keine automatische Synchronisation | Geplant |
| 0.6 | HK1 Tag-/Nacht-Solltemperaturen, Gradient und Fußpunkt schreiben; Wertebereiche und Rücklesen | Geplant |
| 0.7 | Warmwasser Tag-/Nacht-Sollwerte und Zeitprogramme lesen und bearbeiten | Geplant |
| 0.8 | Lüftung Tag/Nacht Stufe 0–3, aktuelle Stufe, Zeitprogramme lesen/schreiben | Geplant |
| 0.9 | Beide Ventilatoren getrennt Running/Failed/Off/Unknown; stufenabhängige Drehzahlgrenzen, Verzögerung, verifizierte F5/F6-Zuordnung, Fehlerzähler/-zeiten und Benachrichtigungen | Geplant |
| 0.10 | Verdichterstarts/-laufzeiten, Zusatzheizungsstunden soweit verfügbar, Sensorplausibilität und Auffälligkeiten mit Zeitpunkten | Geplant |
| 0.11 | Heizungszeitprogramme mit Wochentagen/Start/Ende in HA bearbeiten, dauerhaft in LWZ speichern, validieren und zurücklesen | Geplant |

## Übergreifende Anforderungen

- Profile und Verfügbarkeiten getrennt prüfen; keine erfundenen Register/P-Nummern.
- Parameterkatalog schrittweise erstellen: Beschreibung, P-Nummer falls vorhanden, Einheit, MDI-Icon, Firmwarevergleich, Schreibbarkeit und Zielversion.
- HA-Entitäten einheitlich benennen und als Basis, Expert oder Diagnose einordnen.
- LWZ-Schreibfunktionen frühestens v0.5 mit belegtem Befehl, Wertebereich, manueller Freigabe und Rücklesekontrolle.
- v0.9 ohne automatischen Reset; bestehender Fingerbot bleibt unabhängig.
- Echo II bleibt optional und unabhängig für eine spätere Version.
- Energieeffizienz bleibt in der separaten Home-Assistant-Lösung außerhalb dieser Integration.
