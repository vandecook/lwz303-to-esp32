# Validierung v0.9.0

Stand 25.11.2026. Der Bericht umfasst v0.8.1 (Heizprogramm 2), v0.8.2 (P59 in der Lüfterdiagnose) und v0.9.0 (Abfrageplanung in drei Klassen, manuelles Aktualisieren, Versionsanzeige). Grundlage ist der Stand auf `main` nach Commit `c67e37c`.

## Automatisierte Prüfungen

| Prüfung | Ergebnis |
| --- | --- |
| C++-Hosttests (`tests/run_host_tests.py`, lokal mit Zig, in CI mit g++) | Alle bestanden |
| Python-Tests (`test_selection`, `test_web_help`, `test_parameters`) | Bestanden. Unter Windows schlägt nur der Hash-Test der FHEM-Referenz wegen CRLF-Zeilenenden fehl, in CI nicht |
| `esphome config` für `lwz303-local.yaml` und `lwz303-sol-local.yaml` | Fehlerfrei |
| ESP32-C3-Build von `lwz303-local.yaml` (ESPHome 2026.10.2, ESP-IDF 5.5.6) | Erfolgreich |

Neue Testfälle:

- **HP2:** Konfiguration enthält „Wochenprogramm Heizprogramm 2“ in der Gruppe Wochenprogramme.
- **P59:** Programmzustand Lüftung 4 und 5 erzeugt weder bei laufendem noch bei stehendem Lüfter einen F5/F6. Laufgrund für Normal-, Absenk-, Bereitschafts- und Handbetrieb, Übersteuerung, unbekannten Code und veraltete Daten.
- **Ausgelasteter Bus:** 300 Sekunden mit 5-Sekunden-Intervall. Alle Betriebs- und Parameterblöcke einschließlich `01` und `09` werden gelesen. Ohne die neue Abfragereihenfolge schlägt der Test fehl.
- **Intervallklassen:** `FC` nach 5 Minuten, `17` und `09` nach dem Parameterintervall, `F4` und `F2` nach dem Betriebsdatenintervall wieder fällig.
- **Lüfterdiagnose ohne 10-Sekunden-Grenze:** Bei 30-Sekunden-Intervall entsteht der F5 am ersten frischen Messpunkt nach 120 Sekunden.
- **LWZ-Uhr:** Hochrechnung über Mitternacht und Jahreswechsel; nach mehr als 15 Minuten ohne Uhrzeit „unbekannt“.
- **Manuelle Aktualisierung:** 11 Parameter- bzw. 8 Messwertblöcke je einmal, Mehrfachklick ohne Doppelung, Fortschritts- und Abschlussanzeige.

## CI-Korrekturen

- Der P59-Test meldete Text-Sensoren, deren Lebensdauer vor der Komponente endete. Unter g++/Linux führte das zu einer Speicherbeschädigung. Behoben in `ada7d73`.
- Neue Entitäten standen vorne bzw. mitten in den Listen `text_sensor` und `button`. Die Bestandsprüfung gegen `entities-v0.6.2.json` adressiert über die Listenposition und schlug fehl. Die neuen Einträge stehen jetzt am Listenende. Behoben in `f24f7b0`.

## Beobachtungen am Gerät

Anlage: LWZ 303 SOL, Geräte-Firmware 2.16, Solarfunktion deaktiviert.

- **v0.8.2 geflasht (25.11.2026):** Laufgrund „Zeitprogramm: Absenkbetrieb (P08)“ bei Programmcode 2 und Stufe 0 wurde angezeigt. Nur 20 von 70 P-Parametern hatten Werte, Block `09` fehlte, ohne Kommunikationsfehler. Ursache: Die häufigen Statusabfragen belegten den Bus vollständig. Nach einer Änderung am Intervall und vermutlich dem Ende des Schnellintervalls erschienen die Werte. v0.9.0 behebt die Ursache.
- **v0.9.0:** Noch nicht am Gerät geprüft.

## Offen am Gerät

- Mit 0.9.0: alle 70 P-Parameter und die Betriebsstunden auch bei laufender Heizkreispumpe (Schnellintervall) frisch.
- „Parameter jetzt aktualisieren“ und „Messwerte jetzt aktualisieren“ bis „fertig“.
- F5/F6 mit längerem Standardintervall, z. B. 20 Sekunden.
- HP1 und HP2 mit getrennt eingestellten Zeiten abgleichen.
- Programmzustand Lüftung 4/5 vor Absenkende beobachten.
