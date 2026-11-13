# Roadmap – LWZ303 to ESP32

Festlegung 08.11.2026 umgesetzt: F2ctrl-Heizungs-/Warmwasseranforderung als Basis; SOL-Mapping einschließlich Decoder 2.16, andere Stufenquellen Expert. [Mapping](docs/MAPPING-STUFEN.md).

**Decoder FW 2.16 vorbereitet:** eigenes Profil auf 2.06-Basis; sHC1-Innentemperatur und vier sFan-Ist-/Soll-Drehzahlen vorläufig nicht verfügbar. Relative sGlobal-Istwerte bleiben F5/F6-Quelle. [Details und Verifikation](docs/DECODER-FW2.16.md).

Stand: **08.11.2026**. Geräte-Firmware laut Nutzer: **2.16**. Die Einstellungen **2.06 / 2.14 / 2.14j** bezeichnen getrennte FHEM-Decoderlayouts.

**Aktueller Stand:** v0.5 ist lokal als Software umgesetzt: dauerhaft in ESPHome gespeicherte Profile P13/P14/P15, ausschließlich manuelle Anwendung, Profilerkennung anhand tatsächlicher Lesewerte und keine Versions-Aktivierungsschalter. Die Funktionen aus v0.4 einschließlich 1-s-Kommunikationspausen bleiben enthalten. Hosttests, ESPHome-Codegenerierung und vollständiger ESP32-C3-Build erfolgreich. Geräteprüfung und Veröffentlichung stehen aus. [Bedienung](docs/HEIZKURVENPROFILE-v0.5.md), [Validierung](docs/VALIDATION-v0.5.md).

## Versionsübersicht

| Version | Umgesetzter bzw. verbleibender Umfang | Status |
|---|---|---|
| 0.1 | Serielle Basiskommunikation, 9600 8N1, Pegelwandler, RX5/TX6 im bestätigten Aufbau, Decoderprofile, Retry-/Nonblocking-Logik | Basiskommunikation vom Anwender als funktionierend bestätigt |
| 0.2 | Basiswerte, Kommunikationsdiagnose, P13/P14 lesend, dynamische Abfrageintervalle, lokale Webbedienung und Serviceaktionen | Software umgesetzt; in v0.3 weitergeführt. Neue Hardwarefunktionen nicht vollständig geprüft |
| **0.3** | **Freigegebene Parameterauswahl aus sieben Statusblöcken, Parameterblock 17, eindeutige Hauptquellen, optionale Entitäten, Speed200-Analysewerte, P01/P02/P04/P05 schreiben und Uhr manuell ±1h mit Rücklesen** | **Software umgesetzt und synthetisch getestet; ESP32-Build und Geräteprüfung offen** |
| **0.4** | **Wohnraumlüftung:** feste Tag-/Nachtstufen dauerhaft ändern; aktuelle Stufe aus LWZ-Zustand ermitteln. F5/F6 in ESPHome bei Stufe > 0 und 120 Sekunden Istdrehzahl null erkennen und gespeichert halten; Rücksetzung nach bestätigtem LWZ-Neustart bzw. über HA-Automation, dauerhafte Fehlerhistorie und Meldung bei jedem neuen Fehler. **Zusätzlich Geräteauswahl i / SOL, sControl.hcStage / dhwStage für FW 2.16, Zeitumstellungs-Auswahl und einheitliche Entitätsnamen** | Software lokal umgesetzt; Hardwareprüfung und Veröffentlichung offen. Lüftungszeitprogramme bearbeiten und zeitlich begrenzte Übersteuerung gehören nicht zum v0.4-Umfang. Ausgewählte Lüftungswerte einschließlich Speed200 bereits in v0.3 enthalten |
| 0.5 | Heizkurven-Gradient und Fußpunkt schreiben; benannte Heizkurvenprogramme für Übergangszeit, Winter und eigene Profile als Kombination aus P13/P14/P15 in ESPHome speichern und manuell anwenden | Software lokal umgesetzt; fester bewährter Standard plus fünf bearbeitbare Speicherplätze, Erkennung und gemeinsame Rückleseprüfung. Physische Geräteprüfung und Veröffentlichung offen |
| **0.6** | **Zusätzliche Betriebsdaten und Überwachung: Verdichterstarts/-laufzeiten, Zusatzheizungsstunden soweit belegt, Sensorplausibilität und Auffälligkeiten mit Zeitpunkten** | Geplant; am 08.11.2026 von v0.9 vorgezogen. Vorhandene sHistory-Zähler und blockweise Datenalterung wiederverwenden; keine unbelegten Verdichterstarts ableiten |
| 0.7 | Alle zum jeweiligen Gerätemodell gehörenden P-Parameter auslesen und in einem eigenen Bereich „Programmierte Parameter“ verfügbar machen | Geplant; bisher v0.6. Leseansicht, keine pauschale Erweiterung der Schreibfunktionen |
| 0.8 | Heizungs- und Warmwasser-Wochenzeitprogramme mit Wochentagen und Zeitfenstern lesen | Geplant; bisher v0.7. Die sieben Statusblöcke wurden bereits nach v0.3 vorgezogen |
| 0.9 | Warmwasser-Zeitprogramme bearbeiten und schreiben | Geplant; bisher v0.8. Warmwasser-Sollwerte Tag/Nacht bereits in v0.3 umgesetzt; Lesen der Wochenzeitprogramme gehört nun zu v0.8 |
| 0.10 | Heizungszeitprogramme in Home Assistant bearbeiten, dauerhaft speichern, validieren und zurücklesen | Geplant; unverändert. Lesen der Wochenzeitprogramme gehört nun zu v0.8 |

## v0.4 – Wohnraumlüftung, Geräteauswahl und sControl (lokal umgesetzt)

- **Anzeige/Datenalter (08.11.):** letzte gültige Lüfterstufe behalten, fehlende/veraltete Daten in eigener Datenbasis markieren; F5/F6 weiterhin nur mit frischen Werten. ESP32-Laufzeit in Tagen und hh:mm:ss ergänzt.

- **Heizkurve vorgezogen (08.11.2026):** P13/P14 als Basis-Slider, P15 als Expert-Slider; frisches Lesen-Ändern-Schreiben-Rücklesen in Block 05. Dokumentierte Versuchswerte für Übergangszeit und Kälte; keine automatische Übernahme. Benannte Profilprogramme bleiben v0.5. [Details](docs/HEIZKURVE-HK1.md).

- **Aktuelle Optional-Auswahl:** Relative Drehzahl Hauptlüfter (OUT032) sowie beide Speed200-Werte (OUT063/OUT064) optional, in HA standardmäßig deaktiviert und im Geräteweb unter Expert. Die relativen Zuluft-/Abluftwerte bleiben für F5/F6 unverändert.

Die bisherigen Vorhaben für Lüftungssteuerung und Lüftungsdiagnose bilden eine gemeinsame Version. Die verbleibenden Punkte sind ohne doppelte Versionsnummern neu geordnet. Am 08.11.2026 wurde die bisherige v0.9 als neue v0.6 vorgezogen; die bisherigen v0.6 bis v0.8 wurden jeweils um eine Version verschoben. Die Leseansicht aller P-Parameter gehört damit zu v0.7.

- **Bedienung:** Nur die festen Lüfterstufen Tag/Nacht (P07/P08) dauerhaft ändern, mit gezieltem Schreiben und Rücklesen. Laut aktuellem Nutzerkenntnisstand ist zeitlich begrenzte Übersteuerung über FHEM nicht möglich; sie wird für v0.4 nicht vorgesehen. Aktuelle Stufe zur Diagnose aus der LWZ ermitteln. Lüftungszeitprogramme in v0.4 nicht bearbeiten; für später ohne feste Zielversion vormerken.
- **Analyse:** Zuluft- und Abluftventilator getrennt bewerten. Die bereits in v0.3 enthaltenen Ist-/Soll-Drehzahlwerte sowie OUT063/OUT064 (Speed200) als Datengrundlage verwenden; Einheit, Kalibrierbedeutung und Decoderzuordnung zunächst am Gerät verifizieren.
- **Zustandsbewertung in ESPHome:** F5 lokal erkennen, wenn die gültige Abluft-Istdrehzahl bei tatsächlich wirksamer Lüfterstufe > 0 ununterbrochen 120 Sekunden null ist; F6 entsprechend für Zuluft. Beide Lüfter unabhängig überwachen. Datenlücken ergeben „Unbekannt“. Die Zwei-Minuten-Regel ersetzt für diese Fehlererkennung die bisher allgemein geplanten Mindestdrehzahlgrenzen. Details und Abnahmefälle: [Lüfterdiagnose v0.4](docs/LUEFTERDIAGNOSE-v0.4.md).
- **Zeitprogramme:** Tag/Nacht und zeitweise Übersteuerungen der Wärmepumpe bei der tatsächlich wirksamen Lüfterstufe berücksichtigen. Aktuellen LWZ-Programmzustand bevorzugen; keine unabhängige HA-Zeitsteuerung zur Fehlerentscheidung. Uhrumstellungen dürfen den monoton gemessenen 120-Sekunden-Zeitraum nicht beeinflussen.
- **Fehlerdiagnose:** Herstellerzuordnung F5 = Abluft, F6 = Zuluft bestätigt. Lokale Fehler bis zur Rücksetzung gespeichert halten, auch bei später positiver Drehzahl oder Stufe 0. Wärmepumpenneustart und dessen Steuerung liegen bei HA; ESPHome stellt dafür eine Rücksetzung seines lokalen Fehlerstatus bereit. Eine eigene Neustarterkennung muss belastbar sein; Kommunikationsausfall allein genügt nicht. Je neuem Fehlereintritt melden; je Lüfter Anzahl, letzten Beginn und letztes Ende über ESP-Neustarts erhalten. Details siehe Diagnosekonzept.
- **Abfrage:** Für Diagnosewerte 10 Sekunden als Ausgangsintervall verwenden. Maximale zulässige Datenalterung im Gerätetest bestimmen; Lücken unterbrechen den Prüfzeitraum und löschen keinen bereits gespeicherten Fehler.
- **Uhrbedienung:** Manuelle Auswahl Winterzeit / Sommerzeit ergänzen. Vom Nutzer bestätigter Ausgangszustand ist Winterzeit/Normalzeit/Standardzeit. Nur ein tatsächlicher Auswahlwechsel korrigiert um +1h bzw. −1h, mit Rückleseprüfung und ohne erneute Verschiebung bei Neustart oder gleicher Auswahl. Separate Stunde-+1/−1-Korrekturknöpfe beibehalten; keine automatische jährliche Umstellung. Details und bestehende Tagesgrenzen-Beschränkung siehe Diagnosekonzept.
- **Prüfung:** Tag-/Nachtstufen, aktuellen Programmzustand und Diagnose gemeinsam gegen Geräteanzeige und Rohtelegramme der tatsächlichen Geräte-Firmware 2.16 testen. Maßgeblich sind immer LWZ-Werte und LWZ-Uhr, auch nach direkter manueller Uhrkorrektur an der Wärmepumpe. Die noch offenen v0.3-Geräteprüfungen bleiben bestehen.
- **Geräteauswahl:** LWZ 303 i / Integral oder LWZ 303 / 403 SOL; zusätzlich „Unbekannt / nur Rohwerte“. Auswahl unabhängig von Geräte-Firmware und Decoderlayout führen. Modellabhängige Darstellung gemäß [Booster-Zuordnung](docs/BOOSTER-ZUORDNUNG.md) vorsehen.
- **Entitätsnamen (aktualisiert 09.11.2026):** Das frühere manuelle Gerätenamenpräfix ist wieder entfernt. Entitäts-Anzeigenamen bleiben kurz; Gerätezuordnung erfolgt über HA. ESPHome-Gerätename, Geräte-Anzeigename und explizite interne IDs bleiben erhalten. Bestehende HA-Registrierung am System prüfen.
- **sControl:** `sControl.hcStage` (Heizungsanforderung) und `sControl.dhwStage` (Warmwasseranforderung) aus F2 für v0.4 integrieren, ausdrücklich auch für Geräte-Firmware **2.16**. Die bestehende F2ctrl-Kommentartabelle als Ausgangspunkt verwenden und den F2type-Verweisfehler der FHEM-Profile 2.14/j berücksichtigen. Rohcodes erhalten, Zuordnung je Modell prüfen und unbekannte Codes kenntlich machen. Keine gemessene Leistung aus einer angeforderten Stufe ableiten.

## v0.5 - Heizkurve und saisonale Heizkurvenprogramme

Festgelegt am 08.11.2026: Profile werden dauerhaft in ESPHome gespeichert. Ausschließlich manuelle Anwendung, keine automatische Umschaltung. Die nachfolgenden Profilfunktionen sind in v0.5 lokal als Software umgesetzt; physische Geräteprüfung offen.

- **Konfiguration vereinfachen:** `enable_v03` und `enable_v04` vollständig entfernen, ohne Ersatz durch einen Aktivierungsschalter. Die Funktionen sind automatisch Bestandteil der Komponente; ihre Abhängigkeiten werden intern behandelt. Bestehende Konfigurationen beim Übergang berücksichtigen. Die separate Schreibfreigabe bleibt erhalten.
- **Einzelwerte bereits vorgezogen:** Gradient **P13**, Fußpunkt **P14** und Raumeinfluss **P15** sind in der lokalen v0.4 als einzelne Slider mit Rückleseprüfung umgesetzt. Alle drei bleiben unabhängig manuell bedienbar, auch direkt an der LWZ.
- **Profilinhalt:** Jedes Heizkurvenprogramm enthält die zusammengehörige Kombination **P13 + P14 + P15**. P15 wird ausdrücklich mitgeführt, damit ein manueller Eingriff dort ebenfalls als Abweichung erkannt wird. Die bisherigen Vorschläge verwenden P15=0; ein bestehender Anlagenwert wird dadurch nicht automatisch geändert.
- **Benannte Profile:** „Übergangszeit“, „Winter“ und eigene Programme anlegen, benennen, bearbeiten und dauerhaft in ESPHome speichern. Eigene Werte in den vereinbarten Sliderbereichen mit Schritt 0,1. Ein fester bewährter Standard (P13=0,3/P14=1,0 K/P15=0), dazu fünf bearbeitbare Plätze: zwei vorbelegte Vorschläge und drei eigene. Entwurf/Speichern/Anwenden getrennt; ein ungespeicherter Entwurf blockiert Wechsel und Anwendung. Die Vorschlagswerte sind keine bereits bestätigten optimalen Anlageneinstellungen.
- **Speicherung und Anwendung:** Profile und Änderungen über ESP-Neustarts erhalten. Speichern/Bearbeiten eines Profils verändert die Wärmepumpe nicht. Nur eine ausdrückliche manuelle Profilanwendung mit Schreibfreigabe schreibt in die LWZ. ESP-Neustart, Wiederverbindung oder erkannte Abweichung dürfen kein Profil erneut anwenden. Ein nativer Mehrprofilspeicher der LWZ-Firmware wird nicht vorausgesetzt.
- **Gezieltes Schreiben:** Block 05 frisch lesen, ausschließlich P13/P14/P15 gemeinsam ändern, andere Parameter erhalten und anschließend alle drei Werte unabhängig zurücklesen. Profil erst nach erfolgreicher Kontrolle als angewendet kennzeichnen; bei Fehler keine automatische Wiederholung oder Rückkehr zu einem früheren Profil.
- **Profilerkennung:** Die tatsächlich zurückgelesenen P13/P14/P15 mit den gespeicherten Profilkombinationen vergleichen, auf dem belegten 0,1-Raster statt über ungenaue Fließkomma-Gleichheit. Passt die Kombination zu einem hinterlegten Profil, dessen Namen als erkanntes Profil anzeigen. Passt sie zu keinem, **„Benutzerdefiniert“** anzeigen. Das gilt sowohl nach Sliderbedienung als auch nach Änderungen direkt an der Wärmepumpe, sobald sie zurückgelesen wurden. Keine automatische Rückstellung oder Änderung gespeicherter Profile.
- **Anzeige:** Gespeichertes/ausgewähltes Profil, zuletzt erfolgreich angewendetes Profil und aktuell anhand der Anlagenwerte erkanntes Profil getrennt führen. Ein angewendetes Profil ist bei manueller Abweichung nicht weiter als aktuell aktiv auszugeben. Fehlende oder veraltete Lesedaten gesondert als unbekannt/veraltet markieren; sie beweisen keine benutzerdefinierte Kombination. Eine letzte bestätigte Zuordnung darf erkennbar als letzte Zuordnung erhalten bleiben.
- **Abgrenzung:** Keine automatische Umschaltung nach Außentemperatur, Datum, Uhrzeit oder Jahreszeit. „Winter“ bezeichnet ein Heizkurvenprofil, nicht die Uhrumstellung ±1h aus v0.3.

## v0.6 – Betriebsdaten und Überwachung

Am 08.11.2026 von der bisherigen v0.9 vorgezogen. Noch nicht umgesetzt.

- Zusätzliche Verdichterstarts und -laufzeiten sowie Zusatzheizungsstunden erfassen, soweit die betreffenden Quellen und Bedeutungen belegt sind. Vorhandene sHistory-Zähler wiederverwenden; unterschiedliche Zähler nicht ohne Nachweis gleichsetzen.
- Sensorwerte auf Plausibilität prüfen und erkannte Auffälligkeiten mit Zeitpunkten sichtbar machen. Fehlende, veraltete und tatsächlich auffällige Werte unterscheiden.
- Keine unbelegten Verdichterstarts aus Kommunikationsunterbrechungen, Abfrageausfällen oder unbekannten Zuständen ableiten. Bereits umgesetzte Datenalterung und F5/F6-Diagnose berücksichtigen.
- Konkrete zusätzliche Datenfelder und Bewertungsgrenzen vor der Umsetzung quellenbasiert festlegen.

## v0.7 – Programmierte Parameter

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

**Für die weitere Umsetzung:** Gerätemodell getrennt von Firmware und Decoderlayout auswählen. Bei LWZ 303 i P25 ausblenden und keine Schreibmöglichkeit anbieten. P21–P25 nicht pauschal mit den Statusbits `boosterStage1/2/3` gleichsetzen und aus ihnen keine gemessene elektrische Leistung ableiten. Das Lesen der modellabhängig verfügbaren Hystereseparameter gehört zur P-Parameteransicht in v0.7; zusätzliche Schreibfunktionen hierfür sind keiner Version zugeordnet. Die Heizkurvenprofile in v0.5 umfassen Gradient P13, Fußpunkt P14 und Raumeinfluss P15.

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
- **OUT063 / OUT064:** In v0.3 zunächst als Ja integriert; auf neue Nutzerfestlegung in v0.4 **Optional**, in HA standardmäßig deaktiviert und im Web der Expert-Gruppe zugeordnet. `sFan.supplyFanSpeed200` und `sFan.exhaustFanSpeed200` bleiben für die Wohnraumlüftungsanalyse verfügbar. FHEM-Skalierung unsigned 16 Bit Big Endian /10, eine Nachkommastelle. Physikalische Einheit und Bedeutung des Kalibrierpunkts sind noch nicht bestätigt; keine Umdeutung in U/min oder 1/s. Die F5-/F6-Quelle bleibt unverändert.
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
