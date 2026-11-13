# LWZ303 to ESP32 – v0.5.0

## Zweck des Projekts

Dieses Projekt dient der **Migration der bisherigen FHEM-Anbindung einer Stiebel-Eltron-LWZ auf einen ESP32 mit ESPHome**. Messwerte, Zustände und ausgewählte Einstellungen werden über die serielle Schnittstelle der Wärmepumpe direkt durch ESPHome bereitgestellt und können in Home Assistant verwendet werden.

## Geltungsbereich und bisheriger Testumfang

- **Nur für Geräte-Firmware der Reihe 2.x.** Die Firmwareversion bezeichnet hier die Wärmepumpe, nicht ESPHome oder die Projektversion. Firmware 4.x und andere Reihen gehören nicht zum vorgesehenen Umfang.
- Das eigene **Decoderprofil für Geräte-Firmware 2.16** basiert auf dem FHEM-Decoderlayout 2.06, praktischen Tests und plausiblen Ergebnissen aus den bisherigen Beobachtungen. Bekannte Ausnahmen werden ausdrücklich als nicht verfügbar behandelt. [Details zum Decoderprofil 2.16](docs/DECODER-FW2.16.md).
- **Praktisch getestet wurde bisher ausschließlich mit einer LWZ 303 SOL mit Geräte-Firmware 2.16. Die Solarfunktion war dabei deaktiviert.** Die aktiven Solarfunktionen und andere Gerätemodelle sind damit nicht als getestet ausgewiesen.
- Tests und Beobachtungen bestätigen jeweils die dokumentierten Teilfunktionen; sie bedeuten keine vollständige Prüfung aller Messwerte und Schreibfunktionen. Ergänzend gibt es synthetische Softwaretests. [Validierungsstand](docs/VALIDATION-v0.5.md).

## Verwendete Hardware

- **ESP32-C3** als ESPHome-Gerät.
- **RS232-Pegelwandler/Chip** für die Verbindung zwischen der RS232-Schnittstelle der LWZ und der UART-Schnittstelle des ESP32-C3.

## Grundlagen und Quellen

Die Implementierung nutzt Informationen aus dem Projekt von **Robert Penz** sowie aus der **FHEM-Implementierung des THZ/LWZ-Protokolls**:

- [Robert Penz: Monitoring a Stiebel Eltron LWZ](https://robert.penz.name/heat-pump-lwz/) – Hintergrund zur seriellen Anbindung, Protokollanalyse und bisherigen Projektarbeit. Die dort beschriebenen weiteren Firmwarevarianten erweitern nicht den hier festgelegten Umfang auf Firmware 2.x hinaus.
- [FHEM: 00_THZ.pm, verwendete Referenzrevision](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm) – Decoder, Datenfelder, Skalierungen und Schreibverfahren. Eine unveränderte Kopie mit Lizenz und Herkunftsnachweis ist als [Dokumentationsreferenz](docs/references/README.md) im Paket enthalten.

## Aktueller Funktionsstand

**v0.5:** Heizkurvenprofile P13/P14/P15 dauerhaft in ESPHome speichern, benennen, bearbeiten und ausschließlich manuell anwenden. Fester Standard „bewährt“ (0,3/1,0 K/0) plus fünf bearbeitbare Speicherplätze (Übergangszeit, Winter, drei eigene), Entwurf/Speichern/Anwenden getrennt. Tatsächliche Anlagenwerte bestimmen die Anzeige des erkannten Profils oder „Benutzerdefiniert“; fehlende/veraltete Werte sind gesondert markiert. Keine automatische Umschaltung und kein erneutes Anwenden nach Neustart. [Bedienung und Migration](docs/HEIZKURVENPROFILE-v0.5.md).

Die Optionen `enable_v03` und `enable_v04` sind vollständig entfernt. Beide Zeilen aus bestehenden YAML-Dateien löschen; es gibt keinen Ersatz-Aktivierungsschalter. Die separate Schreibfreigabe bleibt erhalten. Vorhandene Uhr-/Fehlerhistorie bleibt unter ihrem bisherigen Speicherschlüssel, Profile verwenden einen eigenen Schlüssel.

Lüfterstufenanzeige: letzte gültige Zuordnung bleibt auch über 30 Sekunden sichtbar, mit gesonderter Datenbasis-/Altersanzeige. ESP32-Laufzeit zusätzlich als Tage und Stunden:Minuten:Sekunden. [Diagnose und Gültigkeitsregeln](docs/LUEFTERDIAGNOSE-v0.4.md).

Ergänzung 08.11.2026: HK1 Gradient P13 und Fußpunkt P14 als Slider in Basis, Raumeinfluss P15 als Slider in Expert. Bereiche 0–1 / 0–5 K / 0–10, jeweils Schritt 0,1. Mit Schreibfreigabe und unabhängiger Rückleseprüfung; keine Startwerte werden in die Anlage geschrieben. [Vorschläge, Quellen und Bedienung](docs/HEIZKURVE-HK1.md).

Stufenanzeige: sControl.hcStage/dhwStage aus F2ctrl einschließlich ihrer Anforderungstexte stehen unter Basis. Das SOL-Mapping ist auch für Decoder 2.16 zugewiesen; andere Booster-/Stufenquellen stehen unter Expert. [Eigener Mapping-Eintrag](docs/MAPPING-STUFEN.md).

Vorbereitet: eigenes auswählbares Decoderprofil **2.16**, abgeleitet aus 2.06 mit fünf ausdrücklich gesperrten Ausnahmen. Bestehende Startauswahl unverändert. [Zuordnung, Grenzen und Aktivierung](docs/DECODER-FW2.16.md).

v0.4 ergänzt die freigegebene v0.3-Auswahl um dauerhafte Tag-/Nacht-Lüfterstufen, lokale F5-/F6-Diagnose mit gespeicherten Fehlern, Geräteauswahl, sControl und eine manuelle Sommer-/Winterzeit-Auswahl. Zielgerät laut Nutzer: **Geräte-Firmware 2.16**. Die Einstellungen **2.06 / 2.14 / 2.14j** bleiben Decoderlayouts und sind kein Geräte-Firmware-Schalter.

## Bedienung und Diagnose

- **Relative Drehzahl Hauptlüfter optional:** in HA standardmäßig deaktiviert, im Geräteweb unter Expert. Der Wert bleibt verfügbar und wird nicht für die F5-/F6-Bewertung von Abluft/Zuluft benötigt.
- **Speed200 optional:** Zuluft- und Abluft-Speed200 sind in HA standardmäßig deaktiviert und im Geräteweb der Expert-Gruppe zugeordnet. Bereits in HA aktivierte Entitäten werden durch `disabled_by_default` nicht automatisch deaktiviert; dies bei Bedarf in HA ändern. Die relative Drehzahlquelle für F5/F6 bleibt unverändert.
- **P07/P08 dauerhaft schreiben:** Lüfterstufe Tag und Nacht als Number 0–3. Frisch lesen, nur ein Byte des vollständigen Blocks 17 ändern, genau einen SET senden und unabhängig zurücklesen. Keine zeitweise Übersteuerung; Lüftungszeitprogramme werden nicht geschrieben.
- **F5 Abluft / F6 Zuluft:** Wirksame Stufe > 0 und jeweilige gültige Istdrehzahl ununterbrochen 120 Sekunden null. Diagnose lokal im ESP; beide Lüfter unabhängig. Eine neue Störung wird erst an einem frischen bestätigenden Messpunkt gemeldet. Fehlende Daten unterbrechen die Prüfung.
- **Gespeicherte Fehler:** Positive Drehzahl, Stufe 0, Kommunikationsunterbrechung und ESP-Neustart löschen bestätigte Fehler nicht. Fehleranzahl, letzter Beginn, letztes Ende und Rücksetzgrund werden bei Ereignissen unmittelbar gespeichert. Der Speicherstatus zeigt Fehler beim Speichern an.
- **HA-Rücksetzung:** Nach erfolgtem Wärmepumpenneustart die Taste „Lüfterfehler nach LWZ-Neustart zurücksetzen“ aus der eigenen HA-Automation aufrufen. Sie setzt nur den lokalen Fehlerstatus zurück. Kein automatischer LWZ-Neustart und keine vermeintliche Neustarterkennung aus einem Timeout. [HA-Anbindung und Benachrichtigungsvorlage](docs/HA-LUEFTERDIAGNOSE-v0.4.md).
- **Aktuelle Lüfterstufe:** Aus LWZ-Programmzustand EE, P07/P08/P09/P12 und zeitweiser Stufe/Restzeit aus F6 ableiten. Die Anzeige behält die letzte gültige Stufe; „Luefterstufe Datenbasis“ markiert veraltete oder fehlende Quellen. F5/F6 verwenden weiterhin ausschließlich frische Daten, sonst „Unbekannt“. Die bestehende LWZ-Zeitsteuerung bleibt maßgeblich; keine zweite HA-Zeitschaltlogik. Die F6-Übersteuerungspriorität muss für FW 2.16 am Gerät bestätigt werden.
- **Drehzahlquelle:** Standard `sGlobal` (relative Ist-Drehzahlen), alternativ `sFan` (FHEM-Istwerte in 1/s). Der Nutzer bestätigt am 07.11.2026 einen Abschalttest: relative Zuluft-/Abluftwerte und Speed200 gingen von > 0 auf 0. Die relativen Werte bleiben die F5-/F6-Quelle; ein vollständiger Fehlerfall bei angefordertem Betrieb ist damit noch nicht geprüft. E8-Istwerte waren in den früheren Beobachtungen auffällig; vor Verwendung prüfen. Speed200 bleibt ein gesonderter Analysewert. Diagnoseabfrage 10 s; maximale Datenalterung zunächst 30 s, einstellbar über `diagnostic_max_age` und am Gerät zu prüfen.
- **Gerätemodell:** „Unbekannt“, „LWZ 303 i / Integral“, „LWZ 303 / 403 SOL“. Keine automatische Änderung des Decoderlayouts. F2 liefert getrennte Rohcodes `hcStage` und `dhwStage`. Nur für SOL wird die vorhandene FHEM-Kommentartabelle als Anforderungstext verwendet; für i und unbekannte Codes bleibt die Rohcodeanzeige. Keine ungeprüfte Umrechnung in kW. [Quellen und Zuordnungsgrenzen](docs/BOOSTER-ZUORDNUNG.md).
- **Uhrzeit:** Bestätigter Ausgangszustand Winterzeit. Wechsel Winter → Sommer korrigiert +1 h, zurück −1 h; gleiche Auswahl oder Neustart führt keinen weiteren Schreibauftrag aus. Zustand wird erst nach Rücklesen bestätigt. Vor dem SET wird „Unbekannt“ dauerhaft gespeichert, damit Stromausfall keine relative Umstellung wiederholt. Nach unklarem Schreibausgang den tatsächlichen Zustand prüfen und mit den Bestätigungstasten ohne Uhrschreiben zuordnen. Separate Stundenkorrektur ±1 bleibt möglich. Tageswechsel werden weiterhin abgewiesen; keine automatische jährliche Umstellung.
- **Namen:** Seit 09.11.2026 enthalten die Entitäts-Anzeigenamen kein manuelles `device_name`-Präfix mehr. Gerätezuordnung und Anzeige erfolgen über die Home-Assistant-Integration. ESPHome-Gerätename, Geräte-Anzeigename und explizite interne IDs bleiben unverändert. Bestehende HA-Registrierung/benutzerdefinierte Namen beim Umstieg prüfen. P07/P08 sind jetzt bedienbare Numbers statt zusätzlicher Lesesensoren; die übrigen optionalen Werte der ursprünglichen Auswahl bleiben erhalten.

Die v0.3-Schreibwege für P01/P02/P04/P05 bleiben erhalten. Raum 15–25 °C und Warmwasser 35–55 °C sind konservative UI-Vorschläge, keine bestätigten Herstellergrenzen für Geräte-FW 2.16. Schreibfreigabe startet nach jedem ESP-Neustart aus. Es wird nichts optimistisch als geschrieben bestätigt; bei unklarer SET-Antwort erfolgt Rücklesen statt SET-Wiederholung.

## Konfiguration und Paket

Die regulären YAML-Dateien verwenden:

```yaml
external_components:
  - source: github://vandecook/lwz303-to-esp32
    refresh: 0s
    components: [lwz2]
```

Solange v0.5 noch nicht im GitHub-Repository veröffentlicht ist, lädt diese Quelle möglicherweise einen älteren Stand. Für den lokalen Paketstand liegt zusätzlich `lwz303-sol-local.yaml` mit `source: components` bei. Eigene Secrets bereitstellen und Board/Pins prüfen. Kein automatischer Upload, Flash oder Publish wird ausgeführt.

Komponenteneinstellungen: `device_model: unknown`, `fan_speed_source: sGlobal`, `diagnostic_max_age: 30s`, `fan_interval: 10s`. Die Funktionen sind ohne Versions-Aktivierungsflags integriert. Beide mitgelieferten Hauptkonfigurationen enthalten die Profilbedienung. Das gespeicherte Modell ist vor der Interpretation der F2-Texte auszuwählen; bei unbekanntem Modell werden Rohcodes angezeigt.

Für die aktive Diagnose wird das sGlobal-Abfrageintervall auf höchstens 10 Sekunden begrenzt, auch wenn ein größeres Standardintervall eingestellt ist; eine schnellere Dynamik bleibt möglich. Die Anzeige „Aktives Status-Abfrageintervall“ zeigt das tatsächlich geplante Intervall. Die serielle Übertragung kann zusätzliche Verzögerungen verursachen.

Quellen und unveränderte FHEM-/Herstellerreferenzen: [Referenzverzeichnis](docs/references/README.md). Die Stiebel-Eltron-Unterlagen sind lokal unter `docs/references/stiebel-eltron/` abgelegt und von Git ausgeschlossen; das lokale Paket enthält die Original-PDFs.

Prüfstand: [Softwarevalidierung v0.5](docs/VALIDATION-v0.5.md). Geräteprüfung einschließlich physischer Schreibtests, Drehzahlquelle, Betriebsartenpriorität und F2-Codes steht aus. Ausgeführte Softwaretests belegen keine Hardwarefreigabe für alle Modelle. Die folgenden Anschluss- und Diagnoseabschnitte stammen aus dem v0.2-Bestand und sind im Zusammenhang mit den v0.4-Regeln oben zu lesen.

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
| HK1 Gradient P13, Fußpunkt P14 und Raumeinfluss P15 | 05 / pHeat1 | Boot, ab v0.5 standardmäßig jede Minute; manueller Refresh. Vorhandene gespeicherte Intervallwahl bleibt erhalten |
| Firmwareversion | FD | beim Boot/Profilwechsel und manuell, für alle drei Decoderprofile |

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

Nicht blockierender DLE-Handshake, 1800-ms-ACK-/Frame-Timeouts und Fehler-Backoff bleiben erhalten. Seit der Anpassung vom 08.11.2026 gelten 1000 ms Mindestpause nach erfolgreicher Antwort und 1000 ms Abstand vor einem erneuten Leseversuch (zuvor jeweils 250 ms). Empfangsframe und UART-Arbeit pro Schleife sind begrenzt. Veraltete numerische Livewerte werden NaN: Schwelle ist Maximum aus `stale_after` (30 s) und dreifachem effektivem FB-Intervall. Eine Konfigurationsantwort macht veraltete FB-Werte nicht aktuell. Binäre Betriebszustände behalten ihren letzten Wert: immer mit „Kommunikation OK“ bewerten.

## Urheberrecht und Lizenzen

Quelltexte und Dokumentation unterliegen dem Urheberrecht ihrer jeweiligen Ersteller bzw. Rechteinhaber. Für die eigenen Projektdateien gilt **Copyright © 2026 vandecook** und die [MIT-Lizenz](LICENSE).

Beigefügte fremde Quelltexte und Unterlagen bleiben Eigentum ihrer jeweiligen Rechteinhaber und unterliegen ihren eigenen Lizenz- und Nutzungsbedingungen. Dies betrifft insbesondere den FHEM-Perl-Decoder mit seinen ursprünglichen Autoren- und GPL-Hinweisen sowie die Handbücher von STIEBEL ELTRON. Durch die Aufnahme als Dokumentationsreferenz werden keine Urheberrechte am Fremdmaterial beansprucht. Die MIT-Projektlizenz wird dadurch nicht auf diese Unterlagen übertragen.

Originale Urheberrechts- und Lizenzhinweise sind beizubehalten. Quellen, Lizenzhinweise und unveränderte Referenzdateien sind im [Referenzverzeichnis](docs/references/README.md) aufgeführt.

## Protokollbelege und Grenzen

Geprüfte Primärquelle: [FHEM 00_THZ.pm, Commit 2321c570](https://github.com/fhem/fhem-mirror/blob/2321c5704120012cfdc10d2cbee381c8d9676905/fhem/FHEM/00_THZ.pm).

- FBglob206/FBglob214: Temperaturen bei Payloadoffset 2/4/6/10, Aktorbits bei 20.
- 05pxx206: P13 bei Payloadoffset 0, P14 bei 2, jeweils unsigned Big Endian / 10. pHeat1 gehört zum gemeinsamen 2.x-Lesesatz, einschließlich 2.14/2.14j. Keine Übernahme der 4.x-Befehle 0B010E/0B059E.
- FDfirm: Ausleseverfahren des 2.06-Profils für alle vier Decoderprofile verwenden. Zwei Bytes unsigned Big Endian, dezimale Aufteilung durch 100: z. B. `00 D8` = 216 → `02.16`. Transport, Prüfsumme, Befehl und Mindestlänge werden geprüft. Die Anzeige akzeptiert genau zwei Nutzdatenbytes und als Software-Plausibilitätsbereich `01.00` bis `09.99`; dies ist keine belegte Herstellergrenze. Andere Längen oder Werte ergeben „unbekannt“, bei unplausiblen Zahlen mit Rohwert. Die Taste „Firmware jetzt abfragen“ ermöglicht einen neuen Versuch. Ein j-Suffix ist aus der Zahl nicht erkennbar; das gewählte Decoderlayout wird niemals automatisch geändert. Der reale FD-Rückgabewert der Geräte-Firmware 2.16 bleibt am Gerät zu prüfen.
- FHEM-Hexpositionen beginnen bei Prüfsumme/Befehlsbyte: Umrechnung zum Payload = Hexposition / 2 - 2.

Quellenseitige Verifikation ersetzt keinen Hardwaretest auf allen Profilen. Ausgeführte Tests und offene Punkte: [VALIDATION.md](VALIDATION.md). Planung: [ROADMAP.md](ROADMAP.md). Detaillierte Deltas: [CHANGELOG.md](CHANGELOG.md).
