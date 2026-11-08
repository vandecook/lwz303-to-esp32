# Softwarevalidierung v0.4.0

Kommunikationsanpassung 08.11.2026: Mindestpause nach erfolgreicher Antwort und Retry-Abstand von jeweils 250 auf 1000 ms erhöht. Hosttests prüfen explizit, dass nach 999 ms weder eine neue vorgemerkte Abfrage noch ein erneuter Versuch gesendet wird und beide erst bei 1000 ms beginnen. Bestehende Transport-, Schreib-, Diagnose- und Auswahltests bleiben Teil der Prüfung. Kein neuer vollständiger ESP32-Build; Wirkung auf reale Kommunikationsfehler offen.

Auslöser: Nutzer meldet 7 m RS232-Leitung mit Pegelwandler direkt beim ESP32, ACK-/STX-Timeouts über mehrere Befehle, NAK-Folge nach EE-Abbruch sowie einmal Statusbyte 0x82 bei F4. Die Bedeutung von 0x82 ist durch die verwendete FHEM-Referenz nicht belegt; die bisherige Logmeldung allein bestätigt auch keine gültige Prüfsumme dieser Antwort. Ursache nicht abschließend bestimmt. Nach Installation die Fehlerquote bei vergleichbarer Laufzeit und Betriebszuständen vergleichen, nicht nur absolute Zähler. Die festen 10-s-Diagnoseintervalle, Datenaltersgrenze und 120-s-F5/F6-Regel wurden durch diese Anpassung nicht verändert.

Ergänzung Lüfterstufenanzeige: Profil 2.06 mit Tag 1/Nacht 0, Beibehaltung nach 31 und 151 Sekunden, unabhängige Datenqualitätsmarkierung, nur teilweise aktualisierte Quellen, unbekannter Programmcode, neue bestätigte Nachtstufe und Invalidierung beim Decoderwechsel im Hosttest geprüft. Keine F5/F6-Neumeldung aus dem behaltenen Wert. Auswahltest und ESPHome-Codegenerierung beider YAMLs einschließlich lesbarer Uptime erfolgreich. Keine zusätzliche vollständige ESP32-Kompilierung oder reale NA-Ursachenbestätigung.

Heizkurven-Ergänzung 08.11.2026: P13/P14/P15-Slider durch kompilierte Hosttests und ESPHome-Codegenerierung für beide YAMLs geprüft. Alle vier Decoderprofile, Grenzen und 0,1-Raster, Erhalt aller übrigen Parameterbytes, unabhängiges Rücklesen, unsichere SET-Antwort, abweichendes Rücklesen, unvollständige Blöcke und Anzeige nach Reinitialisierung getestet. Der unten angegebene vollständige ESP32-Build liegt vor dieser Ergänzung; kein neuer vollständiger Build und kein physischer Schreibtest. [Details](HEIZKURVE-HK1.md).

Ergänzung 08.11.2026: Basis-/Expert-Zuordnung der Stufen in beiden YAML-Dateien und vollständige SOL-F2-Kommentartabelle für alle vier Profile einschließlich 2.16 getestet. Physische Zuordnung weiterhin nicht unabhängig geprüft. [Mapping](MAPPING-STUFEN.md).

**Ergänzung Decoder 2.16:** Neue Profilvererbung und fünf Feldsperren durch synthetische C++-Tests geprüft; relative sGlobal-Diagnose einschließlich 120-Sekunden-Grenze erhalten, sFan erzeugt keine neue Störung. Profil 2.16 auch in FD-Tests enthalten. Der unten genannte vollständige ESP32-C3-Build stammt vor dieser Decoderergänzung; für diese Änderung wird keine erneute vollständige Gerätekompilierung behauptet. Kein physischer Gerätetest. [Profilbeschreibung](DECODER-FW2.16.md).

Stand: 07.11.2026. Zielgerät laut Nutzer: Geräte-Firmware 2.16. Prüfungen erfolgten ohne physisch angeschlossene Wärmepumpe, ohne Flashen und ohne Änderungen am GitHub-Repository oder an der HA-Installation.

## Ausgeführte Prüfungen

- Decoderergänzung 2.16: Hosttests und Auswahltests erfolgreich. Beide YAML-Dateien zusätzlich mit ausdrücklich gewähltem Profil `2.16`, synthetischen Zugangsdaten und lokaler Quelle durch ESPHome 2026.10.2 validiert und C++-Code erfolgreich generiert. Keine neue vollständige ESP32-Kompilierung für diese Ergänzung.

- ESPHome 2026.10.2: Individuelle YAML und SOL-Beispielkonfiguration mit ausschließlich synthetischen Zugangsdaten und lokaler Komponentenquelle validiert; C++-Codegenerierung für beide Konfigurationen erfolgreich. Persönliche Secrets wurden nicht zur Prüfung benötigt.
- C++-Codegenerierung mit aktivierter v0.4, nativen Number-/Select-Entitäten und beiden Decoder-Einstellungen erfolgreich.
- Hosttests kompilieren die tatsächlichen Dateien `lwz2.cpp`, `v03.cpp` und `v04.cpp` gegen einen kleinen ESPHome-/UART-/Preferences-Prüfrahmen. Bestehende v0.2-/v0.3-Transport- und Schreibtests bleiben Bestandteil.
- v0.4-Prüffälle: F5 erst ab 120 Sekunden, Abluft/Zuluft unabhängig, geplante Nachtabschaltung, Datenlücken, gespeicherte Fehler trotz wieder positiver Drehzahl, Erhalt nach ESP-Neustart, HA-Rücksetzung ohne Löschen der Historie und erneuter Fehlereintritt.
- Betriebsartenprüfungen: EE Tag/Nacht, manueller Betrieb mit P12, zeitweise LWZ-Übersteuerung aus F6, unbekannter Programmcode, Wechsel der Drehzahlquelle.
- F2-Prüfung für alle drei Decoderprofile, getrennte Heizungs-/Warmwasser-Rohcodes, SOL-Kommentartabelle und i-Rohcodeanzeige.
- P07/P08: Integer 0–3, Änderung ausschließlich des betreffenden Bytes im erhaltenen Block 17, unabhängiges Rücklesen; kein optimistischer Number-Erfolg.
- Uhr: gleicher Saisonstatus erzeugt keinen SET, Unsicherheit vor SET dauerhaft speichern, bestätigter Zustand nach Rücklesen, Erhalt nach Neustart, keine Wiederholung bei unterbrochener Transaktion, Ablehnung von Tageswechseln und Speicherfehlern; 32-Bit-Laufzeitüberlauf für die 120-Sekunden-Prüfung.
- Auswahl-/Konfigurationstest: ursprüngliche 80 Freigaben nachvollziehbar erhalten; P07/P08 durch bedienbare Numbers ersetzt. Alle konfigurierten Entitätsnamen tragen `device_name` genau einmal. Die reguläre GitHub-Quelle bleibt gesetzt.
- Neueste Auswahlkorrekturen: OUT032 Relative Drehzahl Hauptlüfter und OUT063/OUT064 Speed200 sind Optional, in HA standardmäßig deaktiviert und der Expert-Gruppe zugeordnet. Der Freigabekatalog enthält damit 30 Ja und 50 Optional; die in v0.4 zu bedienbaren Numbers angehobenen P07/P08 bleiben aktiv. F5/F6 und die Decoderregeln wurden durch diese Konfigurationsänderungen nicht verändert. Auswahltest erfolgreich; kein zusätzlicher vollständiger Build für diese reinen Sichtbarkeitseinstellungen.
- Firmwareabfrage erweitert: FDfirm aus dem Profil 2.06 wird unabhängig vom gewählten Decoderlayout verwendet. Synthetische Antworten 02.06, 02.14, 02.16 und 04.39 werden in allen Profilen korrekt angezeigt; null, zu kleine/zu große Werte, unerwartete Länge und ungültige Prüfsumme werden nicht als gültige Version ausgegeben. Eine erneute manuelle Abfrage verändert das Decoderprofil nicht. Der Zahlenbereich 01.00–09.99 ist eine ausdrücklich gewählte Software-Plausibilitätsregel, kein Herstellerbeleg. Es wurde keine echte FD-Antwort vom Zielgerät empfangen.

## Vollständiger ESP32-Build

**Vollständiger ESP32-C3-Build erfolgreich**, ESPHome 2026.10.2 mit ESP-IDF 5.5.6 und GCC 14.2.0. Die finale Konfiguration wurde mit maximal zwei parallelen Compilerprozessen gebaut.

| Belegung | Ergebnis |
|---|---|
| RAM | 109.765 / 321.296 Byte, 34,2 % |
| Programm-Flash | 1.059.031 / 1.835.008 Byte, 57,7 % |

Geprüft wurde die individuelle ESP32-C3-Konfiguration mit synthetischen Zugangsdaten und lokalem Komponentenstand. Der Abgleich der in den Build kopierten C++-/Header-Dateien mit den ausgelieferten Quellen ist Bestandteil der Abschlussprüfung. Factory-/OTA-Binärdateien wurden lokal zur Buildprüfung erzeugt; sie enthalten Testzugangsdaten und werden deshalb nicht als einsatzfertiges Geräteimage ausgeliefert. Das Paket enthält den Quelltext und eine lokale Beispielkonfiguration zum eigenen Bauen.

Die anfängliche Blockade durch eine abweichende Windows-Cache-Pfadzuordnung wurde durch eine zusätzliche ESP32-C3-Registrierung im vorhandenen ESP-IDF-Cache behoben; die ursprünglichen Metadaten wurden gesichert. Die Korrektur betrifft die lokale Buildumgebung, nicht das LWZ-Protokoll.

## Verbleibende Geräte- und Systemprüfung

- Ergänzende Nutzerbeobachtung vom 07.11.2026: Im durchgeführten Abschalttest gingen relative Zuluft-/Abluftwerte und Speed200-Werte von > 0 auf 0. Damit ist das Nullverhalten der verwendeten relativen Standardquelle beim Abschalten laut Nutzer bestätigt. Dies ersetzt keinen Test mit angefordertem Betrieb bei stillstehendem Ventilator und keinen vollständigen F5-/F6-Test einschließlich 120 Sekunden und Rücksetzung. Kein unabhängiger Rohtelegramm-Mitschnitt dieses Tests liegt vor.
- Istdrehzahlquelle und Einheiten, F6-Übersteuerungspriorität und EE-Modi auf der tatsächlichen FW 2.16 gegen Geräteanzeige/Rohtelegramme prüfen. Die vorhandenen Nutzerbeobachtungen zeigen auffällige E8-Nullwerte; sGlobal ist deshalb die Standardquelle. 30 Sekunden maximale Datenalterung ist ein konfigurierbarer Software-Ausgangswert.
- F2-Anforderungscodes für i und SOL physisch prüfen. Die i-Zuordnung bleibt bis zum Nachweis roh; SOL-Texte sind FHEM-Kommentare, keine gemessenen Heizleistungen.
- Keine belastbare autonome LWZ-Neustartkennung belegt. Rücksetzung wird ausdrücklich durch die eigene HA-Automation nach erfolgtem Neustart übermittelt. Die bereitgestellte Taste ändert nur lokale Statuswerte.
- Physische P07/P08- und vorhandene Sollwert-/Uhr-Schreibwege prüfen. Rücklesen eines Blocks und dessen erhaltene Felder ist implementiert; reale Regelungsreaktion und Hardwarefehlerfälle sind noch zu prüfen.
- Dauerhaftigkeit auf realem ESP-Flash, Erkennungsverzögerung unter serieller Last, Kommunikation und Langzeitbetrieb prüfen. Speicherfehler werden als Diagnose angezeigt; simulierte Speicherung ersetzt keinen Stromausfalltest am Gerät.
- HA-Benachrichtigungsvorlage, Entitätsregistrierung, optionale Entitäten und bestehende Automationen im eigenen HA prüfen. HA-Versionen und bereits gespeicherte Entitätskennungen können Anzeigenamen beeinflussen. Es wurde keine reale HA-Automation angelegt oder verändert.
- Für die individuelle `lwz303.yaml` alle eigenen `!secret`-Verweise auflösen. Die Komponente wurde mit Platzhaltern validiert; keine tatsächlichen Zugangsdaten oder Geheimnisse sind im Paket enthalten.

Die v0.4-Software ist lokal erstellt. Softwareprüfung ist von Hardwarefreigabe, Veröffentlichung und dem Flashen des eigenen Geräts getrennt.
