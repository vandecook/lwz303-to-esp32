# Roadmap – LWZ303 to ESP32

Stand: **07.11.2026**. Geräte-Firmware laut Nutzer: **2.16**. Die Einstellungen **2.06 / 2.14 / 2.14j** bezeichnen getrennte FHEM-Decoderlayouts.

**Aktueller Stand:** v0.4 ist lokal als Software umgesetzt. Synthetische Tests, ESPHome-Prüfung und vollständiger ESP32-C3-Build sind erfolgreich. Hinzugekommen sind Tag-/Nacht-Lüfterstufen, lokale gespeicherte F5/F6-Fehler mit HA-Rücksetzung, Geräteauswahl, F2-Stufencodes und Uhrzeitauswahl. Die v0.3-Auswahl bleibt erhalten; P07/P08 werden als Number statt doppeltem Lesesensor angeboten. Geräteprüfung und Veröffentlichung stehen aus. Details: [Validierung v0.4](docs/VALIDATION-v0.4.md).

## Versionsübersicht

| Version | Umgesetzter bzw. verbleibender Umfang | Status |
|---|---|---|
| 0.1 | Serielle Basiskommunikation, 9600 8N1, Pegelwandler, RX5/TX6 im bestätigten Aufbau, Decoderprofile, Retry-/Nonblocking-Logik | Basiskommunikation vom Anwender als funktionierend bestätigt |
| 0.2 | Basiswerte, Kommunikationsdiagnose, P13/P14 lesend, dynamische Abfrageintervalle, lokale Webbedienung und Serviceaktionen | Software umgesetzt; in v0.3 weitergeführt. Neue Hardwarefunktionen nicht vollständig geprüft |
| **0.3** | **Freigegebene Parameterauswahl aus sieben Statusblöcken, Parameterblock 17, eindeutige Hauptquellen, optionale Entitäten, Speed200-Analysewerte, P01/P02/P04/P05 schreiben und Uhr manuell ±1h mit Rücklesen** | **Software umgesetzt und synthetisch getestet; ESP32-Build und Geräteprüfung offen** |
| **0.4** | **Wohnraumlüftung:** feste Tag-/Nachtstufen dauerhaft ändern; aktuelle Stufe aus LWZ-Zustand ermitteln. F5/F6 in ESPHome bei Stufe > 0 und 120 Sekunden Istdrehzahl null erkennen und gespeichert halten; Rücksetzung nach bestätigtem LWZ-Neustart bzw. über HA-Automation, dauerhafte Fehlerhistorie und Meldung bei jedem neuen Fehler. **Zusätzlich Geräteauswahl i / SOL, sControl.hcStage / dhwStage für FW 2.16, Zeitumstellungs-Auswahl und Gerätenamen in Entitäten** | Software lokal umgesetzt; Hardwareprüfung und Veröffentlichung offen. Lüftungszeitprogramme bearbeiten und zeitlich begrenzte Übersteuerung gehören nicht zum v0.4-Umfang. Ausgewählte Lüftungswerte einschließlich Speed200 bereits in v0.3 enthalten |
| 0.5 | Heizkurven-Gradient und Fußpunkt schreiben; benannte Heizkurvenprogramme für Übergangszeit, Winter und eigene Profile als Kombination aus P13/P14 speichern und anwenden | Geplant; von v0.7 vorgezogen und um Heizkurvenprofile erweitert. Raum-Sollwerte Tag/Nacht bereits in v0.3 umgesetzt |
| **0.6** | **Alle zum jeweiligen Gerätemodell gehörenden P-Parameter auslesen und in einem eigenen Bereich „Programmierte Parameter“ verfügbar machen** | Geplant; bisher offenen Platz belegt. Leseansicht, keine pauschale Erweiterung der Schreibfunktionen |
| 0.7 | Heizungs- und Warmwasser-Wochenzeitprogramme mit Wochentagen und Zeitfenstern lesen | Geplant; mit der bisherigen v0.5 getauscht. Die sieben Statusblöcke wurden bereits nach v0.3 vorgezogen |
| 0.8 | Warmwasser-Zeitprogramme bearbeiten und schreiben | Geplant; bisher v0.7. Warmwasser-Sollwerte Tag/Nacht bereits in v0.3 umgesetzt; Lesen der Wochenzeitprogramme gehört zu v0.7 |
| 0.9 | Zusätzliche Betriebsdaten und Überwachung: Verdichterstarts/-laufzeiten, Zusatzheizungsstunden soweit belegt, Sensorplausibilität und Auffälligkeiten mit Zeitpunkten | Geplant; bisher v0.10. Ausgewählte sHistory-Zähler und blockweise Datenalterung bereits in v0.3 enthalten; daraus werden keine unbelegten Verdichterstarts abgeleitet |
| 0.10 | Heizungszeitprogramme in Home Assistant bearbeiten, dauerhaft speichern, validieren und zurücklesen | Geplant; bisher v0.11. Lesen der Wochenzeitprogramme gehört zu v0.7 |

## v0.4 – Lokal umgesetzt: – Wohnraumlüftung, Geräteauswahl und sControl

Die bisherigen Vorhaben für Lüftungssteuerung und Lüftungsdiagnose bilden eine gemeinsame Version. Die verbleibenden Punkte sind ohne doppelte Versionsnummern neu geordnet. Der zunächst offene Platz v0.6 ist inzwischen für die Leseansicht aller P-Parameter vorgesehen.

- **Bedienung:** Nur die festen Lüfterstufen Tag/Nacht (P07/P08) dauerhaft ändern, mit gezieltem Schreiben und Rücklesen. Laut aktuellem Nutzerkenntnisstand ist zeitlich begrenzte Übersteuerung über FHEM nicht möglich; sie wird für v0.4 nicht vorgesehen. Aktuelle Stufe zur Diagnose aus der LWZ ermitteln. Lüftungszeitprogramme in v0.4 nicht bearbeiten; für später ohne feste Zielversion vormerken.
- **Analyse:** Zuluft- und Abluftventilator getrennt bewerten. Die bereits in v0.3 enthaltenen Ist-/Soll-Drehzahlwerte sowie OUT063/OUT064 (Speed200) als Datengrundlage verwenden; Einheit, Kalibrierbedeutung und Decoderzuordnung zunächst am Gerät verifizieren.
- **Zustandsbewertung in ESPHome:** F5 lokal erkennen, wenn die gültige Abluft-Istdrehzahl bei tatsächlich wirksamer Lüfterstufe > 0 ununterbrochen 120 Sekunden null ist; F6 entsprechend für Zuluft. Beide Lüfter unabhängig überwachen. Datenlücken ergeben „Unbekannt“. Die Zwei-Minuten-Regel ersetzt für diese Fehlererkennung die bisher allgemein geplanten Mindestdrehzahlgrenzen. Details und Abnahmefälle: [Lüfterdiagnose v0.4](docs/LUEFTERDIAGNOSE-v0.4.md).
- **Zeitprogramme:** Tag/Nacht und zeitweise Übersteuerungen der Wärmepumpe bei der tatsächlich wirksamen Lüfterstufe berücksichtigen. Aktuellen LWZ-Programmzustand bevorzugen; keine unabhängige HA-Zeitsteuerung zur Fehlerentscheidung. Uhrumstellungen dürfen den monoton gemessenen 120-Sekunden-Zeitraum nicht beeinflussen.
- **Fehlerdiagnose:** Herstellerzuordnung F5 = Abluft, F6 = Zuluft bestätigt. Lokale Fehler bis zur Rücksetzung gespeichert halten, auch bei später positiver Drehzahl oder Stufe 0. Wärmepumpenneustart und dessen Steuerung liegen bei HA; ESPHome stellt dafür eine Rücksetzung seines lokalen Fehlerstatus bereit. Eine eigene Neustarterkennung muss belastbar sein; Kommunikationsausfall allein genügt nicht. Je neuem Fehlereintritt melden; je Lüfter Anzahl, letzten Beginn und letztes Ende über ESP-Neustarts erhalten. Details siehe Diagnosekonzept.
- **Abfrage:** Für Diagnosewerte 10 Sekunden als Ausgangsintervall verwenden. Maximale zulässige Datenalterung im Gerätetest bestimmen; Lücken unterbrechen den Prüfzeitraum und löschen keinen bereits gespeicherten Fehler.
- **Uhrbedienung:** Manuelle Auswahl Winterzeit / Sommerzeit ergänzen. Vom Nutzer bestätigter Ausgangszustand ist Winterzeit/Normalzeit/Standardzeit. Nur ein tatsächlicher Auswahlwechsel korrigiert um +1h bzw. −1h, mit Rückleseprüfung und ohne erneute Verschiebung bei Neustart oder gleicher Auswahl. Separate Stunde-+1/−1-Korrekturknöpfe beibehalten; keine automatische jährliche Umstellung. Details und bestehende Tagesgrenzen-Beschränkung siehe Diagnosekonzept.
- **Prüfung:** Tag-/Nachtstufen, aktuellen Programmzustand und Diagnose gemeinsam gegen Geräteanzeige und Rohtelegramme der tatsächlichen Geräte-Firmware 2.16 testen. Maßgeblich sind immer LWZ-Werte und LWZ-Uhr, auch nach direkter manueller Uhrkorrektur an der Wärmepumpe. Die noch offenen v0.3-Geräteprüfungen bleiben bestehen.
- **Geräteauswahl:** LWZ 303 i / Integral oder LWZ 303 / 403 SOL; zusätzlich „Unbekannt / nur Rohwerte“. Auswahl unabhängig von Geräte-Firmware und Decoderlayout führen. Modellabhängige Darstellung gemäß [Booster-Zuordnung](docs/BOOSTER-ZUORDNUNG.md) vorsehen.
- **Entitätsnamen:** Alle in ESPHome erzeugten Entitäten sollen ab v0.4 den konfigurierten Gerätenamen enthalten, einschließlich Diagnose, Einstellungen und Aktionen. Einheitliches Präfix aus `device_name`, z. B. `lwz303 Abluftlüfter – Störung`; Präfix genau einmal ausgeben. Anzeigenamen, Home-Assistant-Entitätskennungen und bestehende Zuordnungen bei der Umstellung prüfen, damit weder doppelte Gerätenamen noch unbeabsichtigte zusätzliche Entitäten entstehen. In beiden v0.4-Konfigurationen angewendet; HA-Registrierung am System prüfen.
- **sControl:** `sControl.hcStage` (Heizungsanforderung) und `sControl.dhwStage` (Warmwasseranforderung) aus F2 für v0.4 integrieren, ausdrücklich auch für Geräte-Firmware **2.16**. Die bestehende F2ctrl-Kommentartabelle als Ausgangspunkt verwenden und den F2type-Verweisfehler der FHEM-Profile 2.14/j berücksichtigen. Rohcodes erhalten, Zuordnung je Modell prüfen und unbekannte Codes kenntlich machen. Keine gemessene Leistung aus einer angeforderten Stufe ableiten.

## v0.5 – Heizkurve und saisonale Heizkurvenprogramme

- **Einzelwerte:** Gradient **P13** und Fußpunkt **P14** der HK1-Heizkurve lesen und manuell schreiben.
- **Wertepaare:** Ein Heizkurvenprogramm enthält jeweils die zusammengehörige Kombination **Fußpunkt + Gradient**. Ein Profilwechsel übernimmt beide Werte als gemeinsames Paar.
- **Benannte Profile:** „Übergangszeit“, „Winter“ und eigene Programme anlegen, benennen, bearbeiten und speichern. Die konkreten Wertepaare werden für die Anlage festgelegt; keine ungeprüften Standardkombinationen vorgeben.
- **Speicherung und Anwendung:** Die Profile in der Integration bzw. in Home Assistant verwalten. Beim manuellen Anwenden das gewählte Wertepaar in die LWZ schreiben. Ein nativer Speicher für mehrere Heizkurvenprogramme in der Geräte-Firmware 2.16 ist damit nicht vorausgesetzt oder als belegt dargestellt.
- **Gezieltes Schreiben:** Block 05 frisch lesen, ausschließlich P13/P14 gemeinsam ändern, andere Parameter erhalten und anschließend beide Werte zurücklesen. Anlagengrenzen und Verhalten bei fehlgeschlagenem Rücklesen prüfen; Profil erst nach erfolgreicher Kontrolle als angewendet kennzeichnen.
- **Anzeige:** Gespeichertes Profil, zuletzt erfolgreich angewendetes Profil und tatsächlich zurückgelesene Werte unterscheiden. Bei manuellen Änderungen in der LWZ einen abweichenden Zustand anzeigen, statt das gespeicherte Profil weiterhin als bestätigt aktiv auszugeben.
- **Abgrenzung:** Zunächst manuelle Profilwahl. Eine automatische Umschaltung nach Außentemperatur, Datum oder Zeitplan ist noch nicht festgelegt. „Winter“ bezeichnet hier das saisonale Heizkurvenprofil, nicht die Uhrumstellung ±1h aus v0.3.

## v0.6 – Programmierte Parameter

Nutzerfestlegung vom 07.11.2026: **Alle P-Werte auslesen und in einem eigenen Bereich „Programmierte Parameter“ verfügbar machen.** Der Bereich zeigt gespeicherte Einstellungen getrennt von aktuellen Prozess- und Messwerten.

- Alle dokumentierten und am gewählten Modell tatsächlich verfügbaren P-Parameter erfassen; Varianten LWZ 303 i / Integral und LWZ 303 / 403 SOL sowie Geräte-Firmware 2.16 berücksichtigen. Keine lückenlose P-Nummernfolge erfinden und keine modellfremden Werte als vorhanden darstellen.
- Parameter mit belegter P-Nummer, verständlichem Namen, Wert, Einheit und Verfügbarkeits-/Verifikationsstatus anzeigen. Unbekannte oder nicht auslesbare Werte kenntlich machen, statt sie mit null zu füllen. Nachweislich nicht vorhandene Parameter wie P25 beim i-Modell ausblenden bzw. als nicht anwendbar dokumentieren.
- Einen eigenen Bedienbereich für die programmierten Parameter vorsehen. Vorhandene Lesequellen wiederverwenden; gleiche Werte nicht durch doppelte Abfragen oder doppelte Entitäten vervielfachen.
- Gespeicherten Sollwert und aktuell wirksamen Sollwert eindeutig unterscheiden. Datenalterung und Aktualisierung der Konfigurationswerte sichtbar berücksichtigen.
- Leseumfang, Register und Skalierung anhand Herstellerunterlagen, FHEM-Decoder und Geräteprüfung vervollständigen. Zusätzliche Schreibfreigaben sind mit dieser Festlegung nicht beauftragt; bereits vorhandene oder separat geplante Schreibfunktionen bleiben ihrem jeweiligen Versionsumfang zugeordnet.

## Modellabhängige Zuordnung P21 bis P25

Die folgende Stufenzuordnung wurde laut Nutzerangabe vom **07.11.2026** der **„Bedienungsanweisung für den Fachmann“, Seite 20, unten**, entnommen. Sie wurde inzwischen anhand einer [Herstelleranleitung, Ausgabe A 166459-37161-8835, Seiten 70/71](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-nsm.pdf), unabhängig bestätigt. Die ältere vom Nutzer herangezogene Ausgabe bleibt nicht identifiziert. Die Zuordnung der RS232-Boosterbits zu den Leistungsstufen der tatsächlichen Firmware 2.16 ist weiterhin offen; siehe [Prüfung für Heizung und Warmwasser](docs/BOOSTER-ZUORDNUNG.md).

| Parameter | LWZ 303 i | LWZ 303 SOL / 403 SOL |
|---|---|---|
| P21 | Stufe 1: Verdichter | Stufe 1: Solar |
| P22 | Stufe 2: elektrische Nacherwärmung 2,6 kW | Stufe 2: Verdichter |
| P23 | Stufe 3: elektrische Nacherwärmung 5,6 kW | Stufe 3: elektrische Nacherwärmung 2,6 kW |
| P24 | Stufe 4: elektrische Nacherwärmung 8,8 kW | Stufe 4: elektrische Nacherwärmung 5,6 kW |
| P25 | Nicht vorhanden laut vom Nutzer herangezogener Fachmann-Anweisung | Stufe 5: elektrische Nacherwärmung 8,8 kW |

**Parameterbedeutung:** Der vorhandene [FHEM-Decoder](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L88) bezeichnet P21 bis P25 als `p21Hyst1` bis `p25Hyst5`; die [Parameterdefinitionen](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L670) führen die Einheit **K**. Die Leistungsangaben oben beschreiben die zugeordneten Wärmeerzeugerstufen, nicht den einzustellenden Parameterwert. Die modellspezifische Zuordnung und die genannten Leistungen werden durch diese allgemeinen Decoderdefinitionen allein nicht bestätigt.

**Für die weitere Umsetzung:** Gerätemodell getrennt von Firmware und Decoderlayout auswählen. Bei LWZ 303 i P25 ausblenden und keine Schreibmöglichkeit anbieten. P21–P25 nicht pauschal mit den Statusbits `boosterStage1/2/3` gleichsetzen und aus ihnen keine gemessene elektrische Leistung ableiten. Das Lesen der modellabhängig verfügbaren Hystereseparameter gehört zur P-Parameteransicht in v0.6; zusätzliche Schreibfunktionen hierfür sind keiner Version zugeordnet. Die Heizkurvenprofile in v0.5 bleiben zunächst auf Fußpunkt P14 und Gradient P13 begrenzt.

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
- Eigene Geräteauswahl **LWZ 303 i / Integral** bzw. **LWZ 303 / 403 SOL**, mit „Unbekannt / nur Rohwerte“ als Rückfall, sowie `sControl.hcStage` und `sControl.dhwStage` sind für **v0.4 einschließlich Geräte-Firmware 2.16** eingeplant. Modellabhängige Sichtbarkeit und P21–P25-Beschriftung; keine automatische Kopplung an ein Decoderprofil und keine ungeprüfte Bit-zu-kW-Zuordnung. Umfang siehe [Booster-Zuordnung](docs/BOOSTER-ZUORDNUNG.md); Software lokal umgesetzt, Hardwareprüfung offen. Damit ist kein zusätzliches Lesen/Schreiben von P21–P25 beauftragt.
- Parameterkatalog und einfache Auswahltabelle bei neuen Erkenntnissen fortführen. Messbeobachtung, Decoderbeleg und Hardwareprüfung getrennt kennzeichnen.
- Eindeutige Entitäten und Hauptquellen; keine zusätzlichen Ausgaben allein aufgrund mehrfacher Datenblöcke.
- Keine automatische Heizungs-/Warmwasserregelung durch die neuen manuellen Schreibfunktionen.
- Wärmepumpenneustart zur Fehlerbehebung wird durch HA gesteuert, nicht selbständig durch ESPHome. ESPHome-Fehlerstatus nach bestätigtem Neustart bzw. ausdrücklicher Rücksetzung aus der HA-Automation zurücksetzen; bestehende Neustart-/Fingerbot-Lösung bleibt bei HA.
- Lüftungszeitprogramme später bearbeiten können; aus v0.4 herausgenommen, Zielversion noch offen.
- Echo II bleibt eine unabhängige Option für eine spätere Version.
- Energieeffizienz bleibt in der separaten Home-Assistant-Lösung außerhalb dieser Integration.
