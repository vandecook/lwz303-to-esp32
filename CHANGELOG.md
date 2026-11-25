# Changelog

## 0.9.2 - 2026-11-26 (lokales Quellpaket)

- Button „HK1 jetzt aktualisieren“ heißt „Heizungsparameter jetzt aktualisieren“ und liest nur noch Block `05` (P13–P20, Heizkurven HK1/HK2). Bisher markierte er zusätzlich alle Parameterblöcke als fällig; dafür gibt es seit 0.9.0 „Parameter jetzt aktualisieren“. Der Button läuft jetzt als manuelle Aktualisierung mit Anzeige in „Manuelle Aktualisierung“. „Profil Anlagenwerte jetzt lesen“ verhält sich gleich.
- Umbenennung als genehmigte Ausnahme von der Bestandssicherung (Nutzerentscheidung 26.11.2026). In Home Assistant entsteht eine neue Button-Entität; die Dashboards „Development_AI“, „Expert Corner“ und „WP Experte PC“ müssen auf sie umgestellt werden.
- Manuelle Aktualisierungen laufen vor der Firmware-Abfrage.

## 0.9.1 - 2026-11-26 (lokales Quellpaket)

- Einstellung „Heizkurven-Abfrageintervall HK1“ in „Parameter-Abfrageintervall“ umbenannt. Sie steuert seit 0.9.0 alle Parameterblöcke, die Wochenprogramme und die Betriebsstunden, nicht nur HK1. Ausnahme von der Bestandssicherung auf Nutzerentscheidung vom 26.11.2026; die Einstellung war auf keinem Dashboard und in keiner Automation eingebunden.
- Folgen in Home Assistant: Es entsteht eine neue Entität, die alte bleibt als „nicht verfügbar“ zurück und kann gelöscht werden. Der gespeicherte Wert wird nicht übernommen; die Einstellung startet mit 60 Minuten.
- Die interne ID `config_interval` und die Funktion bleiben unverändert.

## 0.9.0 - 2026-11-25 (lokales Quellpaket)

Neue Abfrageplanung mit drei Klassen. Die unveröffentlichte 0.8.3 ist hier aufgegangen.

- **Betriebsdaten** (`FB`, `F3`, `F4`, `E8`, `F2`, `F6`, `EE`): nur noch langsames (Standard-) und schnelles Intervall. Die bisherige Begrenzung auf 10 Sekunden bei Drehzahlquelle `sGlobal` entfällt; ebenso die getrennten Intervalle für `F4` und `E8`. Die YAML-Schlüssel `heating_interval`, `fan_interval`, `history_interval` und `fan_config_interval` werden noch akzeptiert, wirken aber nicht mehr.
- **Parameter** (`05`, `17`, `01`, `03`, `06`, `07`, `08`, `0A`, `0E`, `10`, Wochenprogramme `0B`–`0D`, Betriebsstunden `09`): einstellbares Intervall „Heizkurven-Abfrageintervall HK1“ (Name aus Bestandsgründen unverändert), Startwert jetzt 60 statt 1 Minute. Ein gespeicherter Wert bleibt erhalten.
- **Uhrzeit und Datum** (`FC`): fest alle 5 Minuten. Fehler-Zeitstempel rechnen die zuletzt gelesene LWZ-Uhr mit der ESP-Laufzeit weiter, auch über Mitternacht und Jahreswechsel; älter als 15 Minuten ergibt „unbekannt“.
- **Lüfterdiagnose F5/F6** hängt nicht mehr an 10 Sekunden. Drehzahl, Programmzustand und Übersteuerung gelten als frisch bis zum Dreifachen des längeren Betriebsdatenintervalls (mindestens `diagnostic_max_age`, 30 s). Die Lüfterstufen P07–P12 aus Block `17` gelten als Parameter. Bei langsamem Intervall meldet die 120-Sekunden-Regel den Fehler am ersten frischen Messpunkt danach.
- **Abfragereihenfolge:** Es wird immer der am längsten überfällige Block gelesen. Reicht der Bus nicht (etwa 1,2 s je Abfrage samt Pflichtpause), strecken sich alle Blöcke gleichmäßig. Vorher kamen bei schnellem Intervall `01`, `09` und die Parameterblöcke nie an die Reihe; in Home Assistant blieben 50 von 70 P-Parametern und die Betriebsstunden unbekannt.
- **Manuelles Aktualisieren:** Buttons „Parameter jetzt aktualisieren“ und „Messwerte jetzt aktualisieren“. Die Blöcke laufen vor den Routineabfragen, jeder einmal; Mehrfachklicks fügen nichts doppelt hinzu. Der Textsensor „Manuelle Aktualisierung“ zeigt Fortschritt und fehlgeschlagene Blöcke. „HK1 jetzt aktualisieren“ bleibt unverändert.
- Versionsanzeige im Webinterface und in Home Assistant: „Projektversion“ (aus `esphome.project.version`) und „ESPHome-Version“ mit Build-Zeitpunkt.
- Neuer Textsensor „Status-Abfrageintervall Grund“: Standard- oder Schnellintervall und der Auslöser (Verdichter, Heizbetrieb, Warmwasser).
- Nutzerbeobachtung 25.11.2026: Nach Erhöhung des Statusintervalls von 10 auf 15 Sekunden erschienen die Parameter. Wegen der damaligen 10-Sekunden-Begrenzung änderte das den Takt nicht; wahrscheinlich hat das Ende des Schnellintervalls (Heizkreispumpe aus) den Bus entlastet.
- Tests: ausgelasteter Bus über 300 s, Intervall je Blockklasse, F5 bei 30-s-Intervall, Uhr über den Jahreswechsel, Ablauf und Status der manuellen Aktualisierung.

## 0.8.2 - 2026-11-25 (lokales Quellpaket)

- Lüfterdiagnose berücksichtigt P59 „Wiederanlauf vor Absenkende“. Meldet die LWZ für die Lüftung den Programmzustand 4 oder 5 (FHEM „restart“), darf die Lüftung schon vor dem Ende der Absenkung laufen. In dieser Phase entsteht kein lokaler F5/F6, weder bei laufendem noch bei stehendem Lüfter.
- Abluft/Zuluft Zustand zeigt in dieser Phase „Laeuft (Wiederanlauf P59)“ bzw. „Aus (Wiederanlauf P59 moeglich)“. Läuft ein Lüfter bei angeforderter Stufe 0 ohne P59-Grund, erscheint „Laeuft ohne Anforderung“ als Hinweis, nicht als Fehler.
- Neuer Textsensor „Lueftung Laufgrund“ (`fan_run_reason`): Zeitprogramm Normal-/Absenkbetrieb/Bereitschaft, Wiederanlauf vor Absenkende (P59), Handbetrieb oder zeitweise Übersteuerung. Grundlage ist der von der LWZ gemeldete Programmzustand, keine eigene Zeitrechnung.
- Unverändert: Lüftung soll laufen (wirksame Stufe > 0) und steht 120 Sekunden → lokaler Fehler.

## 0.8.1 - 2026-11-24 (lokales Quellpaket)

- Zweites Heizzeitfenster aus Block `0B` als „Wochenprogramm Heizprogramm 2“ (`heating_weekly_program_hc2`) in allen mitgelieferten Konfigurationen aktiviert. Es entspricht dem Heizprogramm HP2 der LWZ-Bedienung. Der Decoder ist unverändert.
- Bestehende Entität „Wochenprogramm Heizkreis 1“ behält Namen und ID. Sie zeigt HP1.
- Webhilfe nennt für `0B` HP1 und HP2.

## 0.8.0 - 2026-11-24 (lokales Quellpaket)

- Gespeicherte Wochenprogramme für Heizkreis 1, Warmwasser und zwei Lüftungszeitfenster aus der LWZ lesen. Home Assistant und Webinterface zeigen Start, Ende, Wochentage und Aktivierung.
- Drei getrennte Frischesensoren kennzeichnen den Stand der Abfragen. Heizkreis 2 ist optional und in den mitgelieferten Standardkonfigurationen ausgeblendet.
- Die Wochenprogramme sind in dieser Version lesbar. Bearbeitung und Rückschreiben bleiben auf der Roadmap.

## 0.7.0 - 2026-11-16 (lokales Quellpaket)

- Eigener Bereich „Programmierte Parameter“; 70 nummerierte P-Felder aus den belegten 2.x-Blöcken lesen. SOL-Felder modellabhängig, P25 bei i nicht auswerten; keine Kühlungsstunden.
- P1–P15, P19, P31, P32, P37–P42, P49, P54–P58 und P79 einstellbar. Bestehende neun Einstellfelder weiterverwenden, 22 ergänzen. P86 ist mangels belegtem 2.x-Zugriff weiterhin offen; kein Zugriff auf das neuere Register 0A05A3.
- Bestehende Entitätsnamen, explizite IDs, Domänen und Geräteidentität erhalten. Bisherige reine Sensoren bleiben erhalten, zusätzliche Eingabefelder ergänzen die Bedienung. Eigener automatischer Kompatibilitätsvergleich mit dem 0.6.2-Bestand.
- Vollständigen Block frisch lesen, nur Zielfeld ändern, alle übrigen Bytes erhalten und unabhängig zurücklesen; bei unklarer SET-Antwort keine automatische Wiederholung. Modell- und Decoderwechsel während eines Auftrags sperren.
- Datenfrische der Parameter getrennt anzeigen, letzte Werte bei Kommunikationslücken behalten. Neue Blöcke gestaffelt im Konfigurationsintervall abfragen; keine doppelten Abfragen für vorhandene Quellen.
- Abtau-Integration aus 0.6.2 am 16.11.2026 vom Nutzer als funktionierend bestätigt. Neue Parameter-Schreibwege benötigen ihre eigene Geräteprüfung. HA-Auswertungen und Wochenprogramme bleiben separat geplant.

## 0.6.2 - 2026-11-15 (lokales Quellpaket)

- Anzeige vereinfacht: Binärsensor „Abtauen“, Textzustand ausschließlich „Ja“ oder „Nein“ bei bekanntem Code. Hinweise zur Datenqualität bleiben in den separaten Diagnoseanzeigen.

- F2ctrl.defrostEvaporator als ganzes Rohbyte (Nutzdatenbyte 8): 0x10 aus, 0x30 Verdampfer-Abtauen, andere Codes unbekannt. Keine ungeprüfte Bitmasken-Auswertung.
- Eigene Abtauanzeige und separate Gültigkeit/Datenqualität, unabhängig von Verdichter-Ein/Aus und ohne zusätzliche Abfragen oder Schreibbefehle. Letzte Anzeige bei Datenlücken behalten; bei frischen unbekannten Codes ausdrücklich unbekannt anzeigen.
- Diagnostischer Abgleich mit heatRequest/heatRequest2: Übereinstimmung, widersprüchliche Angaben oder unbekannte Anforderung getrennt ausweisen. Die vorhandene vorläufige Verdichter-Betriebszweckanzeige bleibt erhalten.
- Beide Hauptkonfigurationen, lokal eingebettete Webhilfe und Paketversion aktualisiert. GitHub-kompatible Webressourcen-Einbindung aus der nachgebesserten 0.6.1 bleibt erhalten.
- Neue Tests für direkte Abtauzustände, unbekannte Codes, fehlendes Feld, Kommunikationslücken, Wiederverbindung, Decoderwechsel und Zeitüberlauf. Physische Bestätigung der FHEM-Zuordnung auf FW 2.16 bleibt offen.

## 0.6.1 - 2026-11-13 (lokales Quellpaket)

- Nachbesserung ohne Versionserhöhung auf Nutzerwunsch: Webserver-Ressource direkt in der externen LWZ-Komponente mitliefern. `lwz2.web_help: true` bindet sie aus dem tatsächlichen Komponentenpfad ein; kein manuelles `web_server.js_include` und kein `/root/config/web` mehr erforderlich. Vorheriges Paket im lokalen Revisionsverzeichnis gesichert.
- Aufklappbare Hilfe im Webserver: Bedeutung von frisch/Ein/Aus und Erklaerung aller Datenbloecke, einschließlich 01 und FD als ergänzende Abfragen.
- ESPHome-v3-Weboberfläche und Hilfe gemeinsam lokal eingebettet, ohne CDN-Abfrage und ohne neue Entitäten oder Änderungen bestehender Entitätsnamen.
- Unveränderte LWZ-Abfrage- und Steuerlogik. Versionsangaben 0.6.1, eigenes Paket mit Prüfsummen; Ausgabe 0.6.0 bleibt erhalten.

## 0.6.0 - 2026-11-13 (lokales Quellpaket)

- Letzte gültige Messwerte, Statusbits und gelesene Einstellungen bei seriellen Datenlücken erhalten. Neustart der seriellen Kommunikation verwirft interne Gültigkeit, erhält aber die Anzeige. Decoderwechsel und nicht unterstützte Felder bleiben ausdrücklich unbekannt.
- Datenqualität mit Alter pro Statusblock und elf getrennten Frische-Binärsensoren. Keine veralteten Werte als frische Diagnose-/Rücklesegrundlage verwenden; bestehende F5/F6- und Schreibprüfungen erhalten.
- F2 heatRequest/heatRequest2 als Rohcodes; vorläufiger Betriebszweck nur bei frischen Quellen und übereinstimmenden Anforderungscodes. Gleichzeitiger Bedarf ist eine eigene Anzeige und beweist keine gleichzeitige Wärmelieferung.
- Vier FHEM-Betriebsstundenzähler in ganzen Stunden, neutrale Zähler 1/2; keine Kühlungsstunden-Entität. Heizungs-/Warmwasser-Anzeigenamen vereinfacht.
- Profil-Editorstatus ohne Schrägstrich benannt. Projektangaben und Startmeldung 0.6.0; versioniertes Paket und Prüfbericht. HA-Statistik bleibt separat.
- Keine erfundenen Verdichterstart-/Zusatzheizungszähler und keine unbestätigten Temperaturalarmgrenzen. Diese Erweiterungen bleiben für spätere 0.6.x offen. Neue Geräteprüfung für v0.6.0 steht aus.

## 0.5.0 - 2026-11-08

- Ergänzung 09.11.: HK1 Zusatzheizungsstufe explizit mit accuracy_decimals: 0 in beiden YAMLs und Auswahlmetadaten. Ganzzahlige Anzeige; der bestehende unsigned Stufencode bleibt unverändert. ESPHome-Sensoren verwenden intern weiterhin Float.

- Ergänzung 09.11.: manuelles device_name-Präfix aus allen 154 Entitäts-Anzeigenamen je YAML entfernt. Gerätezuordnung durch HA; ESPHome-Gerätename, Geräte-Anzeigename und explizite IDs unverändert.

- Fester zusätzlicher Eintrag Standard (bewaehrt): P13 0,3, P14 1,0 K, P15 0 nach Nutzerbestätigung. Nicht bearbeitbar; Auswahl und manuelle Anwendung mit Schreibfreigabe, Erkennung und Neustart geprüft. Speicherformat der fünf bearbeitbaren Profile unverändert.

- Heizkurvenprofile P13/P14/P15 dauerhaft in ESPHome: fünf bearbeitbare Speicherplätze, Namen und Entwürfe; getrennte Aktionen Speichern, Verwerfen, Anlagenwerte übernehmen und Anwenden.
- Manuelle Anwendung ändert alle drei Parameter in einem frisch gelesenen Block 05 und erhält alle übrigen Bytes; ein SET und unabhängige vollständige Rückleseprüfung. Keine automatische Anwendung/Umschaltung, kein SET-Retry.
- Profilerkennung anhand tatsächlicher Rohwerte einschließlich P15; Benutzerdefiniert, unbekannt oder veraltet getrennt. Letzte bestätigte Anwendung gesondert gespeichert.
- enable_v03 und enable_v04 samt internen Aktivierungsflags entfernt; keine Ersatzschalter. Separate Schreibfreigabe und bisheriger Uhr-/Fehlerhistorien-Speicher bleiben erhalten.
- Standard für HK1-Konfigurationsabfrage auf eine Minute geändert; vorhandene gespeicherte Intervallwahl bleibt erhalten. Kommunikationspausen weiterhin 1 Sekunde.
- Host-/Konfigurationstests erweitert, Dokumentation und eigenes v0.5-Quellpaket. Physische Profilprüfung und Veröffentlichung offen.

## 0.4.0 – 2026-11-07

- OUT063/OUT064 Speed200 auf jüngste Nutzerfestlegung Optional: in HA standardmäßig deaktiviert und im Web unter Expert; F5-/F6-Drehzahlquelle unverändert.
- FD-Firmwareabfrage aus Profil 2.06 für alle Decoderlayouts, mit Zwei-Byte-/Zahlen-Plausibilitätsprüfung und manueller Abfragetaste; keine automatische Decoderumschaltung.
- F2 sControl und F6 lesen, auch für das Zielgerät mit tatsächlicher FW 2.16; unabhängig von den Decoderlayouts 2.06/2.14/2.14j.
- Geräteauswahl unbekannt / i / SOL. SOL-Anforderungstexte gemäß FHEM-Kommentar; bei i und unbekannten Codes Rohwerte, keine erfundenen kW-Zuordnungen.
- P07/P08 als dauerhaft schreibbare Number, Bereich 0–3, gezieltes Lesen–Ändern–Schreiben–Rücklesen; bisherige Lesesensoren ersetzt.
- Getrennte F5-/F6-Nullprüfung für 120 Sekunden bei wirksamer Stufe > 0; gültige frische Werte und unabhängige Timer. Datenquelle wählbar, Standard sGlobal.
- Gespeicherte Fehler und Historie über ESP-Neustarts; ausdrückliche lokale Rücksetzung für die bestehende HA-Neustartautomation. Keine eigenständige Neustartaktion.
- Nichtoptimistische Uhrzeit-Auswahl Winter/Sommer, initial Winter. Persistierte Unsicherheit vor SET, Bestätigung nach Rücklesen, keine Wiederholung nach Neustart. Stundenkorrekturtasten getrennt erhalten.
- Entitätsnamen mit Gerätenamen, plausiblere Abfragebezeichnungen, reguläre GitHub-Quelle und lokale Paketkonfiguration.
- HA-Benachrichtigungsvorlage, v0.4-Tests, Referenzdateien und Herstellerunterlagen im lokalen Paket. Hardwareprüfung und Veröffentlichung bleiben offen.


## 0.3.0 – 2026-11-06

- Nach Nutzerkorrektur OUT063/OUT064 (Zuluft/Abluft Speed200) als Ja standardmäßig aktiv ergänzt: FHEM uint16 BE /10, eine Nachkommastelle, keine unbestätigte physikalische Einheit.

- Benutzergeänderte Auswahl übernommen: 33 Ja und 47 Optional; Dateihash und 80 Einträge in `components/lwz2/selection.json`.
- F3, EE, FC und 17 in die serielle Abfrageplanung aufgenommen; ausgewählte F4-/FB-Felder mit profilierten FHEM-Offsets ergänzt. Vorhandene E8-/09-/05-Parser weiterverwendet.
- Optionale Entitäten vollständig mit erzeugt, in Home Assistant zunächst deaktiviert; Basis-/Expert-Gruppen im Geräteweb. Nicht gewählte Entitäten aus den beiden Beispiel-/Nutzerkonfigurationen entfernt.
- Unabhängige Global- und HK1-Decoderlayouts; Raum-/Stufenwerte ohne belegtes Layout bleiben ungültig. FB bleibt alleinige Quelle gemeinsamer Mess-/Aktorzustände; kein F4-Überschreiben.
- Pro Block Ablauf-/Längenprüfung für neue Werte, separate Innentemperaturquellen und Unknown statt erfundener Nullwerte bei fehlenden Daten.
- Vier Number-Sollwerte P01/P02/P04/P05 und manuelle Uhrbuttons ±1h; Schreibfreigabe nach Boot aus. Frischer Read–Modify–Write, vollständiges Rücklesen, kein blindes SET-Retry und keine doppelte Zeitverschiebung.
- UI-Grenzen 15–25 °C Raum und 35–55 °C WW ausdrücklich Vorschläge. Kalender-/Tageswechsel abgelehnt; weitere Schreibbereiche unverändert ausgeschlossen.
- Hosttests für Decodermappings, Blockalterung, alle vier Schreibfelder, ACK-Verlust, Fremdfeldänderung, Uhrvergleich, Tageswechsel und Freigaberücknahme ergänzt. Hardwaretest und kompletter ESP32-Build weiter offen.

## 0.2.0 – 2026-11-03

### Detaillierte Deltas v0.1 → v0.2

- Minimalbeispiel auf vier Temperaturen und drei Zustände reduziert; Heizbetrieb/WW folgen weiterhin den Pumpenbits. Vorhandene optionale Entitäten bleiben im Schema.
- FB-Normalintervall 5 → 10 s, dynamisch 5 s mit ODER-Triggern Kompressor/Heizen/WW. Beide Intervalle, Trigger und Dynamik ein/aus über persistente Template-Entitäten konfigurierbar.
- Lesender Block 05 für HK1 P13 Gradient/P14 Fußpunkt ergänzt: alle drei Profile, Big Endian / 10, Boot + 60 min, Intervall einstellbar und manueller Refresh mit Warteschlange.
- Erfolgs-, endgültiger Fehler-, Timeout- und Retryzähler ergänzt, lokaler Reset ohne Einfluss auf Recovery/Backoff. Letzter Erfolg und HK1-Aktualisierung als Zeitpunkte seit Boot.
- FD-Firmwareabfrage ausschließlich für das quellenseitig belegte Profil 2.06, einmal bei Boot/Profilwechsel. Keine unbestätigte Abfrage bei 2.14/2.14j; manuelle persistente Auswahl bleibt maßgeblich.
- Lokal eingebetteter ESPHome Web Server v3 mit Authentifizierung, gecachten Werten, Diagnose, Uptime, ESP-Uhrzeit nach SNTP und Live-Logs. Keine seriellen Abfragen durch Seitenaufrufe.
- RS232-Neuinitialisierung und ESP-Neustart ergänzt. Neustart benötigt separate Freigabe, die nach 30 s verfällt.
- UART-Beispiel RX16/TX17 → RX5/TX6 bei 9600 8N1. Beispielboard esp32dev → esp32-c3-devkitm-1 gemäß vom Anwender bestätigtem ESP32-C3-Aufbau.
- F4/E8/01/09 nur noch bei passenden konfigurierten Verbrauchern; bestehende Intervalloptionen bleiben verfügbar. Minimalbeispiel fragt keinen dieser Blöcke ab.
- DLE/2B18-Transport, drei Versuche, 1800-ms-Timeouts, 250-ms-Retryabstand und Fehler-Backoff erhalten.
- Empfangsframe auf 512 Byte und UART-Verarbeitung pro Schleife auf 512 Byte begrenzt; checksum-gültige verkürzte Payloads werden abgewiesen.
- Stale-Erkennung anhand FB-Alter, mit mindestens dreifachem effektivem Intervall. Konfigurationserfolg stellt FB-Kommunikationsstatus nicht wieder her.
- README aktualisiert; ROADMAP 0.1–0.11, VALIDATION und Softwaretests ergänzt. Neue Hardwarefunktionen ausdrücklich noch nicht am Gerät getestet.
- Keine LWZ-Parameterschreibfunktion; solche Funktionen frühestens ab v0.5.



## 0.1.0

- Initial read-only release.
- Firmware profiles 2.06, 2.14 and 2.14j.
- Non-blocking DLE communication state machine.
- FB 5 s status poll with pump/compressor states.
- Firmware-specific F4 parsing.
- E8 detailed fan parsing.
- Firmware-specific 01 fan configuration parsing.
- 09 operating-hour parsing.
- Retry, timeout, backoff and stale-data handling.
- Optional raw-frame text sensors and VERBOSE raw logging.
- Runtime firmware dropdown via ESPHome template select.
