# LWZ303 to ESP32 – v0.3.0

Die Excel-Auswahl einschließlich der letzten Chatkorrektur ist umgesetzt: **33 Ja + 47 Optional = 80 Einträge**. Die konkrete Anlage läuft laut Nutzer mit **Geräte-Firmware 2.16**. `2.06`, `2.14` und `2.14j` sind die auswählbaren **Decoderlayouts**; daraus folgt keine pauschale Hardwarefreigabe.

## Auswahl und Bedienung in v0.3

- Beide Konfigurationen (`lwz303-sol-example.yaml` und `lwz303.yaml`) enthalten alle ausgewählten Werte. Nicht gewählte Werte sind dort nicht angelegt; bestehende Schemaoptionen bleiben zur Abwärtskompatibilität verfügbar.
- **Ja:** normal aktive Entitäten. **Optional:** mit eingebaut, in Home Assistant zunächst `disabled_by_default: true`; unter Einstellungen → Geräte & Dienste → Entitäten aktivierbar, ohne neues Firmwareupdate.
- Das lokale Webinterface gruppiert Basis und Expert. HA-Aktivierung und Webdarstellung sind getrennt; die Expert-Gruppe ist im Geräteweb sichtbar. Es gibt keinen zusätzlich implementierten Schalter zum Verbergen der Webgruppe.
- Die Abfragen laufen seriell über eine Zustandsmaschine: FB, F3, F4, E8, 09, EE, FC und der Einstellungsblock 17, zusätzlich vorhandenes 05 für P13/P14. Die Excel-Auswahl ist in `components/lwz2/selection.json` mit Dateihash nachvollziehbar; Excel wird nicht zur Laufzeit benötigt.
- **Ein Wert – eine Hauptquelle:** gemeinsame Temperaturen/Aktoren stammen aus FB. F4 schreibt diese Entitäten nicht mehr zusätzlich. FB-Innentemperatur und die zusätzliche F4-Innentemperatur sind getrennt; fehlende kurze F4-Felder ergeben `unavailable`/NaN, niemals erfundene Nullwerte.
- Globales Decoderprofil bleibt standardmäßig **2.14j**. HK1 lässt sich unabhängig mit **2.06 / 2.14 / 2.14j** dekodieren; Standard **2.06**, weil im Nutzeroutput `roomSetTemp` damit 21,4 °C statt 0 liefert. Das bleibt ein zu prüfendes Feldmapping, keine vollständige Freigabe für 2.16.
- Stufe 3 existiert nur im 2.06-FB-Layout. Bei anderen Globalprofilen bleibt die gewählte Entität unbekannt. `hcStage` ist nur im HK1-Layout 2.14j verfügbar; `hcBoosterStage` gehört zu 2.06/2.14. Beide werden angeboten, aber keine Werte aus einem anderen Layout erfunden.
- Die vier ausgewählten sFan-Drehzahlwerte sind optional eingebaut. Ihre physikalische Plausibilität/Decoderzuordnung ist weiter offen; Werte sind kein Nachweis einer funktionierenden Luftmengenmessung. Einheiten bleiben 1/s bzw. % gemäß geprüftem Decoder, nicht U/min oder Watt.
- Fehlende/abgelaufene Daten werden je Block ungültig. Ein erfolgreicher FB-Empfang hält ausgefallene F3/E8/EE/FC-Werte nicht künstlich aktuell.

### Speed200 für die Wohnraumlüftungsanalyse

OUT063 (`sFan.supplyFanSpeed200`) und OUT064 (`sFan.exhaustFanSpeed200`) sind nach der letzten Nutzerkorrektur **Ja**, standardmäßig aktiv und nicht `disabled_by_default`. Beide sind getrennte Zuluft-/Abluft-Kalibrierkennwerte, keine Dubletten der aktuellen Lüfterdrehzahl.

Der geprüfte FHEM-Decoder verwendet Hexposition 56 bzw. 60, unsigned 16 Bit Big Endian und Division durch 10. Eine Nachkommastelle bleibt erhalten (z. B. 3,1 / 460,8). Die physikalische Einheit/Bedeutung ist nicht bestätigt; es wird keine Einheit U/min oder 1/s erfunden. Bei zu kurzer E8-Antwort sind die fehlenden Felder NaN. Die explizite Chatkorrektur ist unter `selection_overrides` festgehalten, falls die geöffnete Excel-Datei noch den früheren Stand enthält.

## Erste manuelle LWZ-Schreibfunktionen

P01/P02 und P04/P05 sind echte, nicht optimistische Number-Entitäten. Daneben gibt es zwei Buttons für +1 bzw. −1 Stunde. Kein Wochenprogramm, Gradient, Fußpunkt, Lüfterparameter, Bereitschafts- oder Manuellsollwert wird damit geschrieben.

`LWZ Schreibfreigabe` ist bei jedem Neustart **aus**. Zum manuellen Test einschalten; der Schreibstatus zeigt Ablehnung, laufendes Rücklesen, Erfolg oder Fehler. Die Software wird hier nicht auf ein Gerät geflasht und hat keine physische LWZ verändert.

1. Vollständigen Block 17 bzw. FC frisch lesen und Transport/Checksumme/Befehl/Länge validieren.
2. Ausschließlich das Zielfeld ändern und alle übrigen Bytes erhalten.
3. Genau einen SET-Befehl senden (Header 01 80, FHEM-Checksumme und 10/2B-Escaping).
4. Unabhängig GET lesen und Zielwert plus erhaltene Felder vergleichen. ACK allein ist kein Erfolg.
5. Bei unklarer SET-Antwort kein SET-Retry: direkt rücklesen. Bei Abweichung Fehler melden, keine automatische Wiederherstellung oder erneute relative Zeitverschiebung.

Konservative **Bediengrenzen als Vorschlag:** Raum Tag/Nacht 15–25 °C; WW Tag/Nacht 35–55 °C; Raster 0,1 °C. Belegte FHEM-Modulgrenzen sind weiter 10–30 bzw. 10–55 °C, keine bestätigten Herstellergrenzen der Geräte-FW 2.16. Die vier vorgeschlagenen UI-Grenzen sind in der Komponente/Number-Erzeugung festgelegt und nicht als Geräteparameter zu verstehen.

Die Uhraktion verändert nur die absolute Stunde eines frisch gelesenen FC-Blocks. Datum/Wochentag und andere Bytes bleiben erhalten; Sekunden dürfen entsprechend der Übertragungsdauer weiterlaufen (Toleranz ±5 s). **Tageswechsel werden abgelehnt**, bis Kalender-/Wochentagsänderungen validiert sind; zusätzlich 30-s-Sperre gegen Mehrfachklicks. Der historische FHEM-Jahres-Schreibbereich 12–20 wird nicht als aktuelle Herstellergrenze übernommen; kein Jahr wird geschrieben. SNTP synchronisiert nur die ESP-Anzeige, nicht die LWZ.

## Softwareprüfung und Quellen

ESPHome-Schemaprüfung und C++-Codegenerierung sowie synthetische C++-Transporttests sind in `VALIDATION.md` dokumentiert. Kein bestätigter Flash-/Hardwaretest und kein neu gebautes ESP32-Firmware-Image. In der individuellen `lwz303.yaml` müssen die vorhandenen `!secret`-Verweise auf die eigenen Zugangsdaten aufgelöst werden; Netzwerk-/Board-Einstellungen wurden beibehalten.

Protokollquelle: [FHEM 00_THZ.pm, gepinnter Stand](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm). Die ausgewählten Felder und Grenzen wurden zuvor gegen den offiziellen SVN-Export r28264 abgeglichen. Der bekannte FHEM-Verweis auf undefinierten `F2type` wird nicht übernommen; sControl gehört nicht zu den hier aktivierten sieben Statusblöcken.

Die folgenden Installations- und Diagnosehinweise stammen aus dem v0.2-Bestand; der v0.3-Umfang und die Schreibfunktionen sind oben beschrieben.

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
