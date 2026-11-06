# Validierung v0.3.0

Aktueller Bericht: [Softwarevalidierung v0.3](docs/VALIDATION-v0.3.md). Die gespeicherte Auswahl mit 33 Ja und 47 Optional ist integriert. Beispiel-Schema, C++-Codegenerierung und synthetische Transport-/Schreibtests sind geprüft; physische Hardwaretests und ein ESP32-Binärbuild stehen aus.

Der folgende Bericht beschreibt den historischen v0.2-Stand; Aussagen zu dort noch fehlenden Schreibfunktionen gelten nicht für die neue v0.3-Implementierung.

# Validierung v0.2.0

Stand: 2026-11-03. Ausgangspunkt: `f936515` (v0.1). Zielhardware laut Anwender: ESP32-C3, UART RX5/TX6, 9600 8N1, RS232-Pegelwandler. Die funktionierende v0.1 ist eine Anwenderbestätigung; es gab hier keinen Zugriff auf die physische Anlage.

## Ausgeführte Softwareprüfungen

- **ESPHome 2026.10.2, Python 3.13.16:** Beispielkonfiguration für `esp32-c3-devkitm-1` validiert. Prüfzugangsdaten liegen ausschließlich in der ignorierten lokalen `secrets.yaml`.
- **C++-Codegenerierung:** erfolgreich. Alle neuen Sensoren, Textsensoren, Nummern, Schalter und Aktionen werden von ESPHome erzeugt.
- **C++-Hosttests:** tatsächliche `components/lwz2/lwz2.cpp` gegen eine kleine simulierte ESPHome-/UART-Laufzeit kompiliert und ausgeführt (Zig C++ 0.16.0, C++17). Der Test ersetzt keinen Build gegen den ESP-IDF oder Hardwaretest.
- **Protokollprüfung:** Tabellen und Parser aus [FHEM 00_THZ.pm, Commit 2321c5704120012cfdc10d2cbee381c8d9676905](https://github.com/fhem/fhem-mirror/blob/2321c5704120012cfdc10d2cbee381c8d9676905/fhem/FHEM/00_THZ.pm), insbesondere `05pxx206`, `FBglob206`, `FBglob214`, `FDfirm`, die 2.x-Lesesätze und die Entfernung des Wire-Headers.
- **Änderungsprüfung:** `git diff --check`.
- **Vollständiger ESP32-Firmware-Build:** auf Wunsch des Anwenders nicht abgeschlossen. Der vorherige Versuch endete bei der Einrichtung der lokalen ESP-IDF-Werkzeuge; es wurde kein fertiges Firmware-Image erzeugt. WLAN- und Web-Zugangsdaten im Test waren ausschließlich Platzhalter. Ein Firmware-Build gegen die echten ESP-IDF-Header sowie Flash-/RAM-Messungen sind damit nicht nachgewiesen.

Die Hosttests verwenden synthetische Frames, keine Hardwaremitschnitte. Sie prüfen:

1. Vier Temperaturfelder, signed Big Endian / 10, Pumpen-/Kompressorbits und HK1 P13/P14 bei allen drei Profilen.
2. Alle acht Kombinationen der ODER-Trigger und das Abschalten der Dynamik.
3. Read-only-Requestbytes für `05`, Wakeup, ACK, STX, DLE-Escaping und vollständige Antwortverarbeitung.
4. Falsche Prüfsumme, falscher Antwortbefehl, verkürzte Payloads, 2B18-Decoding und übergroße unvollständige Frames.
5. Drei Timeoutversuche, zwei Retries, genau ein endgültiger Fehler, Zählerreset ohne Löschen des Recovery-Zustands.
6. RS232-Neuinitialisierung einschließlich erneuter Anwendung der UART-Einstellungen.
7. Überlauf der Intervallplanung und Firmware-Abfrage nur beim dafür belegten Profil.
8. Veraltete FB-Werte bleiben trotz erfolgreicher Konfigurationsantwort als veraltet gekennzeichnet; gültige FB-Antwort stellt den Zustand wieder her.
9. Keine optionalen Legacy-Abfragen im Minimalbetrieb und keine zusätzliche Transaktion durch wiederholtes Beobachten gecachter Werte.

## Wiederholen

```console
python tests/run_host_tests.py --compiler g++
esphome config lwz303-sol-example.yaml
esphome compile lwz303-sol-example.yaml --only-generate
esphome compile lwz303-sol-example.yaml
```

Alternativ für den Hosttest: `--compiler /pfad/zu/zig --zig`. Der GitHub-Workflow `.github/workflows/validate.yaml` führt Hosttests, ESPHome-Schemaprüfung und Codegenerierung aus. Dieser Workflow wurde hier erstellt, aber noch nicht auf GitHub ausgeführt.

## Offene Hardwareprüfungen

- Neue v0.2-Firmware auf dem konkreten C3-Modul flashen; die genaue C3-Boarddefinition und Flashgröße abgleichen.
- FB-Werte und P13/P14 für jedes tatsächlich eingesetzte Firmwareprofil mit LWZ-Anzeige/Rohframes vergleichen; insbesondere Skalierung und Werteverfügbarkeit bestätigen.
- Für 2.06 die FD-Antwort am Gerät prüfen. 2.14/2.14j werden nicht mit unbestätigten Firmware-Befehlen abgefragt.
- Leitungsunterbrechung, NAK, Timeout/Retry/Backoff, automatische Wiederverbindung und RS232-Neuinitialisierung am Pegelwandler testen.
- Normal-/Dynamikintervall unter realen Kompressor-/Pumpenwechseln messen; manuelles HK1-Refresh und Stundenintervall prüfen.
- Persistenz nach Ablauf der Speicherfrist durch echten Neustart/Stromunterbrechung bestätigen.
- Webansicht, Authentifizierung, Live-Logs, Zeit ohne/mit SNTP, Zählerreset und 30-s-Neustartfreigabe im Browser prüfen; seriellen Verkehr beim bloßen Seitenaufruf vergleichen.
- Langzeitbetrieb und Millisekundenüberlauf am Gerät wurden nicht getestet. Boot-Zeitstempel laufen nach ca. 49,7 Tagen um (dokumentierte Einschränkung); Intervalle nutzen überlaufsichere Differenzen.

LWZ-Schreibbefehle sind nicht implementiert und werden nicht getestet. ESP-Uhr/SNTP und lokale Konfiguration verändern keine LWZ-Parameter.
