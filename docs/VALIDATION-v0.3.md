# Softwarevalidierung v0.3.0

Stand 2026-11-06. Geräte-Firmware laut Nutzer 2.16; 2.06/2.14/2.14j sind Decoderlayouts, keine drei physisch getesteten Gerätefirmwares.

## Auswahl und Konfiguration

- Die vom Nutzer gespeicherte Auswahl enthält 31 Einträge `Ja` und 47 `Optional`, insgesamt 78. `components/lwz2/selection.json` hält IDs, Herkunft, gewählte Einstellungen und SHA-256 der Excel-Datei fest.
- `tests/test_selection.py` prüft die vollständige Konfiguration aller 76 Werte plus 2 Uhraktionen, eindeutige Schlüssel, 47 deaktivierte optionale HA-Entitäten, die Ausschlüsse und die vier gezielten Number-Schreibfelder. Die gespeicherte Excel-Datei wird, sofern lokal vorhanden, gegen den Hash geprüft; Firmware und CI brauchen keine Excel-Datei.
- ESPHome **2026.10.2**: Beispielkonfiguration gültig und C++-Codegenerierung erfolgreich. Kein neues ESP32-Firmware-Image erstellt; Flash/RAM und ESP-IDF-Build sind nicht nachgewiesen.
- Die individuelle `lwz303.yaml` hat einen bereits vorhandenen, lokal nicht auflösbaren Verweis `!secret api_encryption_key`. Die übrige Konfiguration wurde deshalb in einer isolierten Kopie mit Prüfplatzhaltern validiert. Zugangsdaten, Netzwerk- und Board-Einstellungen der Originaldatei wurden nicht ersetzt. Vor Einsatz auf dem eigenen ESPHome-Host müssen dessen Secret-Verweise vorhanden sein.

## Ausgeführte C++-Tests

Die tatsächlichen Dateien `lwz2.cpp` und `v03.cpp` wurden gegen den simulierten ESPHome-/UART-Laufzeitrahmen mit **Zig C++ 0.16.0 / C++17** kompiliert und die Tests ausgeführt. Alle Tests sind synthetisch; die Nutzer-Auszüge sind dekodierte Texte und keine aufgezeichneten Transporttelegramme.

Geprüft sind:

1. Bestehende v0.2-Tests: drei Profile, signed Temperaturen, HK1-Konfiguration, ODER-Trigger, Polling, Frameprüfung, Escaping, Timeouts/Retry/Backoff, Reset, Millisekundenüberlauf und Firmware-Abfragebeschränkung.
2. Neue F3/F4/FC-Felder: Signed-Wert −49, Solltemperatur 47, Betriebsart `setback`, unbekannte Enumwerte, Wochentag, unabhängiges HK1-Layout, Raum-Sollwert an der zum Layout gehörenden Position.
3. Kein Überschreiben der FB-Innentemperatur durch F4. Eigenständige F4-Innentemperatur bei kurzem Block NaN. Dritte Heizstufe nur im vorhandenen 2.06-FB-Layout, andernfalls ungültiger Binärzustand.
4. Unabhängiges Veralten von F3- und E8-Entitäten; neue kurze FB-Antwort setzt fehlende Lüfter-Prozentwerte auf NaN statt ältere Werte zu behalten.
5. Alle vier Schreibziele: deaktivierte Freigabe, NaN, Werte außerhalb der vorgeschlagenen Grenzen und ungültiges 0,1-°C-Raster abgelehnt; ein aktiver Auftrag verhindert parallele Schreibaufträge.
6. Vollständiger frischer GET-17, Erhalt aller anderen Bytes einschließlich 10-/2B-Escaping, genau ein SET, unabhängiger GET zur Verifikation, kein Erfolg allein aufgrund SET-ACK.
7. Verlorene SET-Antwort: kein SET-Retry, direkte Rücklesekontrolle. Veränderter anderer Parameter führt zu Fehler, keine automatische Reparatur oder Wiederholung. Zu kurzer Parameterblock wird abgelehnt.
8. Uhr ±1h, Datum/Reservebytes erhalten, laufende Sekunden berücksichtigt, Wiederholungssperre, Ablehnung beider Tagesgrenzen. Rücknahme der Schreibfreigabe vor SET verhindert das Schreiben.

## Wiederholen

```console
python tests/test_selection.py
python tests/run_host_tests.py --compiler g++
esphome config lwz303-sol-example.yaml
esphome compile lwz303-sol-example.yaml --only-generate
```

Unter Windows wurde der Hosttest mit `--compiler .validation/zig/ziglang/zig.exe --zig` ausgeführt; der Zig-Cache lag im Arbeitsverzeichnis. Die Testumgebung ist kein Teil der Firmware.

## Nicht geprüft / nicht freigegeben

- Physische RS232-Abfragen, Schreiben, Rücklesen, Neustart-Persistenz und Browser-/HA-Bedienung an der realen Anlage.
- Reale Zustandswechsel der Zusatzheizungsstufen, tatsächliche Verwendung von Stufe 3 in Geräte-FW 2.16, HK1-Enum-Semantik und Sensorverfügbarkeit.
- Plausibilität des ausgewählten sFan-Drehzahlmappings; die Werte sind auf Wunsch des Nutzers optional enthalten, nicht als physikalisch bestätigt gekennzeichnet.
- Sommer-/Winterzeit über Datumsgrenzen: absichtlich nicht implementiert. Kein Schreiben von Jahr, Monat, Tag oder Wochentag; kein automatischer Uhrabgleich.
- Herstellergrenzen der Geräte-FW 2.16. Raum 15–25 °C und WW 35–55 °C bleiben konservative Software-/Bedienvorschläge, keine bestätigten technischen Grenzwerte.
- Vollständiger ESP32-C3-Build und Speicherverbrauch. Es wurde weder ein Binärimage geliefert noch geflasht oder auf GitHub veröffentlicht.
