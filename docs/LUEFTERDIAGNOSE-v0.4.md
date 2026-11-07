# v0.4 – Lüfterdiagnose und Uhrzeitbedienung

Festgelegter Planungsumfang vom 07.11.2026. Gilt auch für Geräte-Firmware 2.16 und die geplante Geräteauswahl i / SOL. In v0.4 als Software umgesetzt; Hardwareprüfung und Veröffentlichung stehen aus.

## Fehlerregeln in ESPHome

Die Auswertung läuft lokal auf dem ESP. Home Assistant übernimmt Darstellung, Verlauf und Benachrichtigungen; die Fehlerermittlung benötigt keine HA-Verbindung.

| Lokale Diagnose | Bedingung |
|---|---|
| F5 – Abluftlüfter | Aktuell wirksame Lüfterstufe > 0 und gültige Abluft-Istdrehzahl ununterbrochen 120 Sekunden = 0 |
| F6 – Zuluftlüfter | Aktuell wirksame Lüfterstufe > 0 und gültige Zuluft-Istdrehzahl ununterbrochen mindestens 120 Sekunden = 0 |

Beide Lüfter erhalten unabhängige Zeitgeber und Fehlerzustände. Exakt null ist das Fehlerkriterium; niedrige, aber positive Drehzahlen lösen diese Regel nicht aus. Stufenabhängige Mindestdrehzahlen sind keine zusätzliche Voraussetzung dieser Festlegung. Speed200-Kalibrierkennwerte sind keine Istdrehzahl und werden nicht für diese Regel verwendet.

Die Herstelleranleitung bestätigt die Zuordnung Fehler 05 = Abluftlüfter, Fehler 06 = Zuluftlüfter. Die 120-Sekunden-Regel ist die vom Nutzer festgelegte lokale Diagnose; sie wird damit nicht als identischer interner Erkennungsalgorithmus der LWZ ausgegeben. Aus der LWZ gelesene Fehler und lokal ermittelte Fehler bleiben getrennte Entitäten, z. B. „F5 Abluft – lokal erkannt“ und „F5 – von LWZ gemeldet“. Fehlerbehebung durch Neustart der Wärmepumpe wird über HA gesteuert. ESPHome führt keine eigenständige automatische Neustartaktion aus.

Quelle für die Gerätenamen: [Herstelleranleitung, Seite 55](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-nsm.pdf#page=55), FHEM `faultmap` in der beigefügten Referenzdatei. Kopien und Herkunft: [Referenzdokumentation](references/README.md).

### Ablauf und Datenqualität

- Erste gültige Beobachtung mit wirksamer Stufe > 0 und Istdrehzahl 0 startet die Überwachung. Der Zustand während der Wartezeit lautet „Anlauf/Prüfung“; erst nach 120 Sekunden besteht ein lokaler Fehler.
- Nur wiederholt gültige, hinreichend aktuelle Werte dürfen die Bedingung bestätigen. Ein einzelnes Nulltelegramm und anschließender Kommunikationsausfall dürfen keinen Fehler erzeugen. Der Fehler wird am ersten frischen bestätigenden Messpunkt ab 120 Sekunden gemeldet; die Abfrageperiode bestimmt die zusätzliche Erkennungsverzögerung.
- Vor einem Fehlereintritt beendet Istdrehzahl > 0 die Nullphase, und Stufe 0 beendet die laufende Prüfung. Fehlende/veraltete Daten, nicht verfügbare Stufe oder ungültige Drehzahl (z. B. NaN oder negativ) setzen den laufenden Prüfzeitraum zurück und ergeben „Unbekannt“. Ein bereits gespeicherter F5/F6 bleibt davon unberührt; Datenqualität und gespeicherter Fehlerstatus werden getrennt angezeigt.
- Ein bestätigter lokaler Fehler bleibt gespeichert, bis ein zuverlässig erkannter Wärmepumpenneustart oder die zugehörige Rücksetzung durch HA erfolgt. Positive Drehzahl, Stufe 0, HA-Wiederverbindung oder ESP-Neustart löschen ihn nicht automatisch. Das ist kein Löschen des LWZ-Fehlerspeichers.
- Der Übergang Stufe 0 → > 0 startet einen neuen Prüfzeitraum. Ein Wechsel zwischen zwei positiven Stufen unterbricht eine bestehende Nullphase nicht, solange alle Daten gültig bleiben. Nach ESP-Neustart oder Datenlücke beginnt die Überwachung neu; Ausfallzeiten werden nicht als beobachtete Nullphase angerechnet.
- Die 120 Sekunden mit einer monotonen Laufzeituhr messen. Geräteuhrkorrektur, Sommer-/Winterzeit, NTP und Kalenderwechsel dürfen die Dauer nicht verkürzen oder verlängern.
- Diagnosewerte zunächst alle **10 Sekunden** abfragen. Die maximale zulässige Datenalterung bleibt im Gerätetest festzulegen. Bei Messlücken darf die Datenalterung die Regel nicht umgehen. Erst die bestätigte Istdrehzahlquelle zur Fehlerbewertung freigeben; vorhandene auffällige E8-Werte sind noch gegen Anzeige/Rohtelegramm zu prüfen.

### Gespeicherte Fehler, HA-Rücksetzung und Historie

- Je Ventilator einen gespeicherten lokalen Fehlerstatus führen. HA meldet jeden neuen Übergang in diesen Fehlerzustand. Ein fortbestehender Fehler zählt als ein Ereignis; wiederholte Messungen und Wiederverbindungen erzeugen weder zusätzliche Zählerstände noch neue Fehlermeldungen. Nach Rücksetzung kann ein erneuter, vollständig bestätigter Fehlereintritt eine neue Meldung auslösen.
- Eine explizite, über HA aufrufbare Aktion zum Zurücksetzen der lokalen F5-/F6-Statuswerte bereitstellen. Die HA-Automation für den Wärmepumpenneustart ruft diese im Rahmen der erfolgten Rücksetzung auf; bloßes Aktivieren der Automation oder Starten eines Neustartversuchs gilt nicht als bestätigter Neustart. Keine bestehende HA-Automation wird im Rahmen dieser Planung verändert.
- Wenn ein LWZ-Neustart selbst belastbar erkannt werden kann, lokale Fehler daraufhin zurücksetzen. Ein serieller Timeout, WLAN-Ausfall oder Neustart des ESP ist dafür kein Beleg. Bis zur Verifikation einer solchen Gerätekennung ist die ausdrückliche Rückmeldung der HA-Automation der vorgesehene Weg.
- Rücksetzung löscht die gespeicherten lokalen Fehler und startet die Prüfung mit frischen Daten neu. Bleibt Stufe > 0 und Drehzahl 0, entsteht nach erneut beobachteten 120 Sekunden wieder ein Fehler. Die Rücksetzung schaltet die Wärmepumpe nicht selbst und löscht weder deren Fehlerspeicher noch die lokale Historie.
- Je Lüfter **Fehleranzahl, letzten Fehlerbeginn und letztes Fehlerende** dauerhaft über ESP-Neustarts speichern. Das Ende entspricht hier der Rücksetzung des gespeicherten Fehlerstatus; es ist kein Beleg, dass der Lüfter bereits wieder läuft. Resetgrund zur Unterscheidung HA-Rücksetzung / bestätigter LWZ-Neustart festhalten.
- Zeitstempel nur mit gültiger Zeitquelle speichern; fehlt eine solche, Zeit als unbekannt kennzeichnen und den Fehler trotzdem zählen. Keine Zeitstempel erfinden. Nach Neustart gespeicherte Fehler und Historie laden, aber keinen zuvor laufenden 120-Sekunden-Prüfzeitraum fortschreiben.

## Tatsächlich wirksame Lüfterstufe und Zeitprogramme

Tag-/Nachtfenster der Wärmepumpe sind verbindlich. Keine zweite, unabhängig von der LWZ laufende HA- oder ESP-Zeitschaltlogik zur Diagnose anlegen.

**Bedienumfang v0.4:** Nur feste Tag-/Nachtstufen P07/P08 ändern. Änderungen bleiben dauerhaft bestehen, bis sie erneut geändert werden. Nach aktuellem Nutzerkenntnisstand ist zeitlich begrenzte Übersteuerung über FHEM nicht möglich; eine solche Bedienfunktion ist nicht vorgesehen. **Lüftungszeitprogramme bleiben unverändert.** Eine spätere Bearbeitung ist vorgemerkt, aber keiner Version zugeordnet. Das Lesen des aktuellen LWZ-Programmzustands für die Diagnose bleibt erforderlich.

1. Einen am Gerät bestätigten aktuellen Stufenstatus bevorzugen. Falls dieser fehlt, die wirksame Stufe aus dem von der LWZ gemeldeten Betriebs-/Programmzustand und den passenden Einstellungen ableiten.
2. Im zeitgesteuerten Normal-/Absenkbetrieb sind `sProgram.ProgStateFAN` und Tag-/Nachtwerte P07/P08 mögliche Quellen. Bereitschaft P09, manuelle Stufe P12 und zeitweise Übersteuerungen müssen entsprechend der tatsächlich wirksamen Betriebsart berücksichtigt werden.
3. `sSystem.userSetFanStage` und Restzeit sind mögliche Quellen für Übersteuerungen; ihre Bedeutung und Priorität für FW 2.16 vor Verwendung prüfen. Weder eine gespeicherte Tagstufe noch die allgemeine Heizungsbetriebsart allein belegt die aktuelle Lüfterstufe.
4. Wenn aus Status und Einstellungen keine eindeutige aktuelle Anforderung hervorgeht, „Unbekannt“ ausgeben. Dadurch wird z. B. eine geplante Nachtabschaltung nicht als Defekt bewertet.
5. Für den nötigen Abgleich der Wochenzeitprogramme die Uhr der LWZ aus `sTimedate` verwenden. Wenn die LWZ ihren aktuellen Programmzustand bereits liefert, diesen nicht nochmals wegen Sommerzeit um eine Stunde verschieben.

Decoderquellen: [P07/P08/P09/P12](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L151), [Programmstatus](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L197), [zeitweise Lüfterstufe](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L281).

## Sommer-/Winterzeit-Auswahl und Korrekturknöpfe

**Bestätigter Ausgangszustand laut Nutzer: Normalzeit = Standardzeit = Winterzeit.** Das sind drei Bezeichnungen desselben Zustands, keine drei Auswahloptionen.

Für v0.4 ist eine manuelle Auswahl **Winterzeit / Sommerzeit** vorgesehen:

| Auswahlwechsel | Aktion an der LWZ-Uhr |
|---|---|
| Winterzeit → Sommerzeit | +1 Stunde |
| Sommerzeit → Winterzeit | −1 Stunde |
| Gleiche Auswahl erneut | Kein Schreibauftrag, keine weitere Verschiebung |

- Die initiale Zuordnung „Winterzeit“ beruht auf der ausdrücklichen Nutzerfestlegung, nicht auf einer vermeintlich ausgelesenen DST-Kennung. Ein frischer Uhrblock bestätigt die Uhrwerte, aber allein nicht den Sommer-/Winterzeitstatus.
- Sommer-/Winterzeit ist hier die Uhrzeitumstellung. `sHC1.seasonMode` bezeichnet den Heizungs-Sommer-/Winterbetrieb und ist dafür kein Nachweis. Ein eigener DST-Schreibparameter ist in den geprüften Decoderfeldern nicht belegt; die Auswahl verwendet die geprüfte Stundenkorrektur.
- Vor dem Wechsel frisch lesen, nur die nötige Stunde ändern, unabhängig zurücklesen. Auswahl erst nach erfolgreicher Kontrolle als angewendet speichern; nach Fehlschlag den tatsächlichen Zustand prüfen, bevor erneut geschrieben wird. Neustart, Wiederverbindung oder Wiederherstellen der Auswahl dürfen keinen erneuten Stundenversatz auslösen.
- Kein optimistisches Bestätigen der Auswahl. Schreibfreigabe und Schutz vor parallelen Aufträgen bleiben bestehen. Ein gleichzeitiger Auftrag über einen Korrekturknopf wird abgewiesen.
- Die vorhandenen **Stunde +1 / Stunde −1**-Knöpfe bleiben als separate manuelle Uhrkorrektur vorgesehen. Sie wählen keine Saison. Für die Lüfterbewertung sind stets die aus der Wärmepumpe gelesenen Werte, Uhrzeit und Programmzustände maßgeblich. Wird die Uhr direkt an der LWZ korrigiert, deren aktualisierte Werte verwenden; keine zusätzliche Bestätigung des Sommer-/Winterzeitstatus für die Diagnose verlangen und keinen zweiten Zeitversatz anwenden. Die gespeicherte Saison-Auswahl ist kein Ersatz für eine aktuelle LWZ-Zeit und löst bei externer Uhrkorrektur keine automatische Gegenkorrektur aus.
- Keine automatische jährliche Umstellung beauftragt. Die bestehende Ablehnung von Uhrkorrekturen über eine Tagesgrenze bleibt, bis Kalenderänderungen ausdrücklich umgesetzt und geprüft sind; ein abgewiesener Wechsel verändert auch die bestätigte Auswahl nicht.

## Abnahmefälle für v0.4

| Fall | Erwartung |
|---|---|
| Stufe > 0, Abluft 0 für < 120 s | Noch kein F5 |
| Stufe > 0, Abluft 0 für ≥ 120 s, fortlaufend frische Daten | Lokaler F5; Zuluft unabhängig |
| Stufe > 0, Zuluft 0 für ≥ 120 s, fortlaufend frische Daten | Lokaler F6; Abluft unabhängig |
| Nachtprogramm verlangt Stufe 0, beide Drehzahlen 0 | Keine neue lokale Störung; bereits gespeicherte Fehler bleiben erhalten |
| Tag-/Nachtwechsel oder zeitweise Stufe weicht vom Grundprogramm ab | Tatsächlich wirksame Stufe verwenden |
| Daten fehlen, werden veraltet oder sind unplausibel | Datenqualität unbekannt; Prüfzeitraum zurücksetzen, gespeicherte Fehler erhalten |
| Drehzahl wird nach bestätigtem F5/F6 positiv | Gespeicherten Fehler bis zur Rücksetzung erhalten |
| HA bestätigt erfolgten LWZ-Neustart und setzt lokal zurück | Fehlerstatus löschen, Historie erhalten, Prüfung neu beginnen |
| ESP oder WLAN wird neu gestartet, LWZ-Neustart nicht belegt | Gespeicherten Fehler erhalten; keine fälschliche Neustarterkennung |
| Wiederholte Nullwerte während desselben gespeicherten Fehlers | Ein Ereignis und eine Meldung; nach Rücksetzung und erneuten 120 s neuer Fehler möglich |
| Feste Tag-/Nachtstufe wird geändert | Wert bleibt nach erfolgreichem Rücklesen dauerhaft geändert |
| Uhr wird während der Prüfung umgestellt | 120-Sekunden-Dauer unverändert |
| Sommerzeit mehrfach gewählt oder ESP neu gestartet | Insgesamt nur ein bestätigter +1h-Wechsel |
| Uhrschreiben scheitert / Tagesgrenze wäre betroffen | Kein bestätigter Auswahlwechsel |

Diese Fälle bilden die Abnahmegrundlage. Ausgeführte synthetische Tests und die Grenzen der Geräteprüfung sind im [Validierungsbericht](VALIDATION-v0.4.md) dokumentiert.

## Konkretisierung der v0.4-Software

Standardquelle für die Nullprüfung ist sGlobal; sFan ist alternativ auswählbar. `diagnostic_max_age` hat zunächst 30 Sekunden als konfigurierbaren Softwarestandard, keine bestätigte Herstellergrenze. Ohne frische EE-/17-/F6-Daten bleibt die wirksame Stufe unbekannt. Zeiten der Historie stammen aus dem zuletzt frischen LWZ-Uhrblock und sind entsprechend auf das Abfrageintervall genau.

Für v0.4 begrenzt die aktive sGlobal-Diagnose das effektive Statusintervall auf höchstens 10 Sekunden; ein kürzeres dynamisches Intervall bleibt möglich. E8 wird ebenfalls höchstens alle 10 Sekunden eingeplant. Die tatsächliche Ankunft hängt zusätzlich von serieller Auslastung und Antworten ab. Bei Fehlern gelten Datenalterung und Abbruch der Prüfung, keine garantierte Antwortfrist.

Ein eigenständiger zuverlässiger LWZ-Neustartindikator ist nicht belegt; deshalb wird die ausdrückliche HA-Rücksetztaste umgesetzt. Sie setzt beide lokalen Fehler zurück und erhält Zähler/Historie. Der Zustand „Anlauf/Prüfung“ und ein eventuell bereits gespeicherter Fehler werden getrennt dargestellt. [HA-Vorlage](HA-LUEFTERDIAGNOSE-v0.4.md).

Nach unterbrochenem Uhrschreiben oder separater Stundenkorrektur steht die lokale Saisonzuordnung auf „Unbekannt“. Nur dann können die Bestätigungstasten die zuvor am Gerät geprüfte Winter-/Sommerzeitzuordnung ohne Uhrschreiben wiederherstellen. Diese Metadaten werden für die Lüfterdiagnose nicht benötigt; dafür bleiben stets LWZ-Programm und LWZ-Werte maßgeblich.
