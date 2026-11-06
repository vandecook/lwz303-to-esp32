# LWZ303 to ESP32 – v0.2.0

Lesende ESPHome-Komponente für Firmwareprofile **2.06, 2.14 und 2.14j**. Basis: die vom Anwender als funktionierend bestätigte v0.1 (`f936515`). Neue v0.2-Funktionen sind noch nicht an der Hardware getestet. Kein 4.x/5.x-Support, keine LWZ-Parameterschreibfunktionen vor v0.5.

## Anschluss und Installation

- **9600 Baud, 8N1, ESP-RX GPIO5, ESP-TX GPIO6**, gemäß getestetem Aufbau.
- RS232 über **3,3-V-kompatiblen Pegelwandler**, z.B. MAX3232. Niemals RS232 direkt an ESP-Pins anschließen. Gemeinsame Masse und Signalrichtungen beachten. Ein MAX232 erfordert zusätzlich passende TTL-Pegelanpassung.
- Getesteter Aufbau laut Anwender: **ESP32-C3**. Beispielboard: `esp32-c3-devkitm-1`; bei abweichendem C3-Modul die genaue Boarddefinition anpassen. GPIO6 ist bei klassischen ESP32-Modulen ein Flash-Pin; dieser Aufbau ist für ESP32-C3 vorgesehen.
- Serieller Logger bleibt aus (`baud_rate: 0`).

Repository herunterladen und `components/lwz2/` neben `lwz303-sol-example.yaml` belassen. Für die Softwareprüfung wurde ESPHome 2026.10.2 verwendet. Lokal `secrets.yaml` anlegen:

```yaml
wifi_ssid: "Mein WLAN"
wifi_password: "WLAN-Passwort"
web_username: "admin"
web_password: "Eigenes Web-Passwort"
```

```console
esphome config lwz303-sol-example.yaml
esphome compile lwz303-sol-example.yaml
esphome run lwz303-sol-example.yaml
```

Das Beispiel lädt die lokale Komponente. Firmwareprofil passend zur LWZ im Webinterface oder Home Assistant wählen; es wird persistent gespeichert. Eine abgefragte Versionsnummer überschreibt die manuelle Auswahl nie.

## Daten und Abfrageplanung

| Daten | Quelle | Standard |
|---|---|---|
| Außen-, Vorlauf-, Rücklauf-, Warmwassertemperatur | FB / sGlobal | 10 s |
| Kompressor, Heizbetrieb = HK-Pumpe, WW-Bereitung = WW-Pumpe | FB | gleicher Block |
| HK1 Gradient P13 und Fußpunkt P14 | 05 / pHeat1 | Boot, dann 60 min; manueller Refresh |
| Firmwareversion | FD | einmal beim Boot/Profilwechsel, nur Profil 2.06 |

Dynamik ist standardmäßig aktiv: 5 s bei Kompressor **ODER** Heizbetrieb **ODER** Warmwasser. Alle drei Trigger sind anfangs ausgewählt. Ohne ausgewählten aktiven Trigger gilt das Normalintervall. Im Webinterface konfigurierbar: Normal- und Dynamikintervall (5–300 s), HK1-Intervall (1–1440 min), Dynamik ein/aus und drei Trigger. Speicherung erfolgt durch ESPHome nach etwa einer Minute; Stromausfall davor kann letzte Änderungen verlieren. Diagnosezähler bleiben flüchtig.

Intervalle laufen ab Transaktionsabschluss; Kommunikation, Retries und Backoff verlängern reale Abstände. Es läuft nur eine Transaktion gleichzeitig. Manueller HK1-Refresh wird zwischen Transaktionen eingereiht; ausstehende Klicks werden zusammengefasst. Nach endgültigem HK1-Fehler wartet die Automatik das Konfigurationsintervall, ein manueller Refresh bleibt möglich.

Bestehende optionale F4-, E8-, 01- und 09-Entitäten und ihre Intervalloptionen bleiben verfügbar. Ihre Datenblöcke werden nur bei konfigurierten Verbrauchern gelesen. Das Minimalbeispiel fragt keinen dieser zusätzlichen Blöcke ab.

Die bisherigen Optionen unter `lwz2` bleiben erhalten: `heating_interval` (F4, 10 s), `fan_interval` (E8, 30 s), `fan_config_interval` (01, 5 min) und `history_interval` (09, 5 min). F4 wird für `inside_temperature` oder `raw_f4` gelesen; die anderen Blöcke bei ihren jeweiligen Sensoren oder Raw-Entitäten.

## Lokales Webinterface

`http://lwz303-sol.local/` oder Geräte-IP öffnen und mit Web-Zugangsdaten anmelden. ESPHome Web Server v3 ist lokal eingebettet. Angezeigt werden ausschließlich gecachte Entitäten: **Seitenaufrufe erzeugen keine seriellen Abfragen.** Basiswerte, HK1, Diagnose, Uptime und effektives Intervall sind sichtbar. ESP-Uhrzeit erscheint nach SNTP-Synchronisierung (Europe/Vienna), sonst „nicht verfügbar“. SNTP stellt ausschließlich die ESP-Uhr, niemals die LWZ-Uhr.

Letzter Erfolg und HK1-Zeitpunkt sind ausdrücklich `boot+…s`, unabhängig von Netzzeit. Der Millisekundenzähler läuft nach etwa 49,7 Tagen um; Zeitstempel beginnen dann erneut. Die Intervallplanung berücksichtigt den Überlauf. Logs sind ein Live-Stream während der geöffneten Verbindung, kein dauerhaftes Archiv.

- **HK1 jetzt aktualisieren:** lesende 05-Abfrage einreihen.
- **Kommunikationszähler zurücksetzen:** vier Zähler löschen; Backoff, Zustand und letzter Erfolg bleiben erhalten.
- **RS232 neu initialisieren:** laufende Transaktion abbrechen, Empfangspuffer und Protokollzustand zurücksetzen, UART-Einstellungen erneut anwenden und frisch abfragen. Kein Reset der LWZ.
- **ESP32 neu starten:** zuerst Bestätigungsschalter aktivieren, dann innerhalb von 30 s Neustarttaste drücken. Ohne Freigabe wirkungslos; Freigabe wird nicht gespeichert.

## Diagnose und Recovery

Erfolgreiche Abfragen zählen gültige Antworten mit passendem Befehl, Prüfsumme und Mindestlänge. Fehlgeschlagene Abfragen zählen endgültige Fehler nach maximal drei Versuchen. Timeouts zählen einzelne abgelaufene Wartezeiten; Retries zählen eingeplante Wiederholungen (maximal zwei je Transaktion). Letzter Fehler, letzter Erfolg und Kommunikationszustand ergänzen die Zähler.

Nicht blockierender DLE-Handshake, 1800-ms-ACK-/Frame-Timeouts, 250-ms-Retryabstand und Fehler-Backoff aus v0.1 bleiben erhalten. Empfangsframe und UART-Arbeit pro Schleife sind begrenzt. Veraltete numerische Livewerte werden NaN: Schwelle ist Maximum aus `stale_after` (30 s) und dreifachem effektivem FB-Intervall. Eine Konfigurationsantwort macht veraltete FB-Werte nicht aktuell. Binäre Betriebszustände behalten ihren letzten Wert: immer mit „Kommunikation OK“ bewerten.

## Protokollbelege und Grenzen

Geprüfte Primärquelle: [FHEM 00_THZ.pm, Commit 2321c570](https://github.com/fhem/fhem-mirror/blob/2321c5704120012cfdc10d2cbee381c8d9676905/fhem/FHEM/00_THZ.pm).

- FBglob206/FBglob214: Temperaturen bei Payloadoffset 2/4/6/10, Aktorbits bei 20.
- 05pxx206: P13 bei Payloadoffset 0, P14 bei 2, jeweils unsigned Big Endian / 10. pHeat1 gehört zum gemeinsamen 2.x-Lesesatz, einschließlich 2.14/2.14j. Keine Übernahme der 4.x-Befehle 0B010E/0B059E.
- FDfirm: uint16 als Version, z.B. 206 → 02.06. In FHEM nur für Profil 2.06 im Lesesatz belegt. Bei 2.14/2.14j deshalb „unsupported (manual profile)“ und keine Probe. Ein j-Suffix ist so nicht erkennbar.
- FHEM-Hexpositionen beginnen bei Prüfsumme/Befehlsbyte: Umrechnung zum Payload = Hexposition / 2 - 2.

Quellenseitige Verifikation ersetzt keinen Hardwaretest auf allen Profilen. Ausgeführte Tests und offene Punkte: [VALIDATION.md](VALIDATION.md). Planung: [ROADMAP.md](ROADMAP.md). Detaillierte Deltas: [CHANGELOG.md](CHANGELOG.md).
