# Übergabe LWZ303 → ESPHome

24.11.2026 · Projektversion 0.8.0 · Geräte-Firmware 2.16 · ESPHome 2026.10.2

## Ziel und belegter Stand

Die bisherige FHEM-Anbindung einer Stiebel-Eltron LWZ 303 wird durch eine lokale ESPHome-Komponente auf einem ESP32-C3 ersetzt. Sie liest die LWZ über RS232, stellt Messwerte, Betriebszustände und Einstellungen in Home Assistant und im Geräteweb bereit und kann ausgewählte Einstellungen schreiben. Die Zeitprogramme laufen in der LWZ.

Die Praxisanlage ist eine **LWZ 303 SOL mit Firmware 2.16**, deaktivierter Solarfunktion und einem genutzten Heizkreis. Der Nutzer hat v0.8.0 geflasht und am 24.11.2026 das korrekte Auslesen der in seiner Konfiguration angezeigten Wochenprogramme für Heizung, Warmwasser und Lüftung bestätigt. Die Abtauanzeige war bereits am 16.11.2026 am Gerät bestätigt. Für v0.8.0 liefen die C++- und Python-Tests, Konfigurationsprüfung und Codegenerierung für beide YAMLs sowie der vollständige ESP32-C3-Build der lokalen Hauptkonfiguration erfolgreich. Die Bestätigungen betreffen diese Anlage und die beobachteten Funktionen. [Prüfberichte v0.8.0](../VALIDATION-v0.8.md) und [v0.7.0](../VALIDATION-v0.7.md)

## Hardware und Anschluss

| Bestandteil | Aufbau |
| --- | --- |
| Controller | ESP32-C3. Die Hauptkonfiguration `lwz303.yaml` ist als generischer ESP32-C3 Super Mini mit 4 MB Flash und ESP-IDF eingerichtet. Das SOL-Beispiel verwendet `esp32-c3-devkitm-1`. |
| Geräteschnittstelle | RS232 an der LWZ, UART am ESP32-C3. Ein 3,3-V-kompatibler RS232/UART-Pegelwandler ist erforderlich, beispielsweise ein MAX3232. Der genaue verbaute Wandler ist in den Projektquellen nicht belegt. RS232-Spannungspegel gehören ausschließlich an die RS232-Seite des Wandlers. |
| Signalrichtung | ESP-TX **GPIO6** zum UART-Eingang des Wandlers, dessen RS232-Ausgang zum LWZ-Empfang. LWZ-Senden zum RS232-Eingang des Wandlers, dessen UART-Ausgang zum ESP-RX **GPIO5**. Gemeinsame Signalmasse verwenden. |
| UART | **9600 Baud, 8N1**. Den seriellen ESPHome-Logger mit `baud_rate: 0` abgeschaltet lassen. |

Die Pins beziehen sich auf den ESP32-C3. Bei einer anderen Boardvariante Pinbelegung und LWZ-seitig den konkreten Anschlussplan prüfen. [Hauptkonfiguration](../../lwz303.yaml), [SOL-Beispiel](../../lwz303-sol-example.yaml) und [Herstellerunterlagen](../references/README.md)

## Serielles Protokoll 2.x

1. Der ESP sendet zum Wecken `02` (STX), die LWZ quittiert mit `10` (DLE).
2. Der ESP sendet den Request `01 Typ Prüfsumme Befehl [Nutzdaten] 10 03`. Für GET ist der Typ `00`, für SET `80`. SET enthält die Nutzdaten des betroffenen Blocks.
3. Die LWZ kündigt die Antwort mit `10 02` an. Die beiden Bytes können getrennt eintreffen. Der ESP akzeptiert auch ein einzelnes `02` und quittiert die Ankündigung mit `10`.
4. Die Antwort hat denselben Rahmen. Das Befehlsbyte muss zum Request passen. Die Antworttypen `00` und `80` stehen für GET bzw. SET. Andere Typbytes sind Fehlercodes. `15` ist ein NAK. Nach dem vollständigen Antwortframe sendet der ESP ein abschließendes `10`.

Die **Prüfsumme** ist die Summe aller unmaskierten Bytes des Körpers modulo 256. Das Prüfsummenbyte selbst, das separate STX und der Abschluss `10 03` gehen nicht ein. Ein GET auf Block `05` lautet `01 00 06 05 10 03`. Im Körper wird `10` als `10 10` und `2B` als `2B 18` übertragen. Vor der Prüfung und dem Parsen werden diese Folgen wieder aufgelöst. Im C++-Decoder beginnen Nutzdaten bei Frame-Offset 4. Die Feldbreite, das Vorzeichen und der Dezimalteiler sind je Feld definiert. Mehrbytefelder sind bei den hier verwendeten Formaten Big Endian. [ESPHome-Transport](../../components/lwz2/lwz2.cpp) und [FHEM-Transportreferenz](../references/fhem/00_THZ.pm#L1507)

### Wichtige Befehlsbytes

Alle Befehle in der Tabelle sind hexadezimal. Eine P-Nummer bezeichnet ein Bedienfeld innerhalb eines Blocks.

| Block | Inhalt und Verwendung |
| --- | --- |
| `FB`, `F3`, `F4` | `sGlobal` mit Temperaturen, Pumpen, Verdichter und relativen Lüfter-Istwerten. `sDHW` für Warmwasser, `sHC1` für Heizkreis 1 mit separat wählbarem Layout. |
| `F2`, `F6`, `EE`, `E8` | `F2ctrl` für Anforderung, Stufencodes und Abtauzustand. `F6` liefert Daten zur zeitweisen Lüfterübersteuerung, `EE` Betriebsart und aktuellen Programmzustand, `E8` zusätzliche Lüfterwerte. Die Lüfterfehler F5/F6 sind Fehlerbezeichnungen. |
| `09`, `FC`, `FD` | Betriebsstunden gemäß `09his206`, LWZ-Datum/Uhrzeit, Geräte-Firmware. `FD` liefert z. B. die Zahl 216 für die Anzeige `02.16`. |
| `17`, `05` | P01–P12 einschließlich Raum-/Warmwasser-Sollwerten und Lüfterstufen. `05` enthält u. a. P13/P14/P15 für die HK1-Heizkurve. |
| `01`, `03`, `06`, `07`, `08`, `0A`, `0E`, `10` | Weitere Parameterblöcke für Lüftung, Abtauung, Heizung, Warmwasser, Solar, Pumpenzyklen, Vorverlegung und Trockenheizen. Modellgrenzen, Längen und Offsets stehen in der Parametertabelle. |
| `0B`, `0C`, `0D` | Gespeicherte Wochenprogramme für Heizung, Warmwasser und Lüftung. Der Decoder verwendet zwei Einträge in `0B` und `0D`, einen in `0C`. Die sechs Bytes pro Eintrag enthalten Start und Ende je als Big-Endian-Zahl im Dezimalformat HHMM, sieben Wochentagsbits (Bit 0 Montag bis Bit 6 Sonntag) und ein Aktivierungsbyte (0/1). |

[Parametertabelle](../../components/lwz2/parameters.h), [Wochenprogrammdecoder](../../components/lwz2/lwz2.cpp) und [FHEM-Befehlstabelle](../references/fhem/00_THZ.pm#L824)

## Was FHEM macht

Die mitgelieferte `00_THZ.pm` (v0.210, gepinnte Referenzrevision) ordnet Namen wie `sGlobal`, `pHeat1` und `pHeatProg` Befehlen und firmwareabhängigen Decodertabellen zu. `THZ_Get_Comunication`, `THZ_encodecommand` und `THZ_decode` lösen Handshake, Rahmen, Escaping und Prüfsumme. `THZ_Parse1` extrahiert Werte aus Hex-Strings anhand von Position, Länge, Typ und Teiler. Die Positionsangaben sind Hex-Zeichenpositionen ab Prüfsummenbyte und Befehlsbyte. Position 4 entspricht daher Nutzdatenbyte 0. Die `interval_*`-Attribute steuern periodische Abfragen. Bei 2.x liest FHEM für SET den übergeordneten Block und ersetzt darin das Zielfeld. Die Wertebereiche im FHEM-Katalog sind Referenzwerte für die Bedienung.

Die Referenzprofile 2.06, 2.14 und 2.14j unterscheiden sich. Der Verweis `F2type` in den FHEM-Tabellen für 2.14/2.14j ist in der unveränderten Quellenkopie enthalten. ESPHome verwendet dafür `F2ctrl`. FHEM dient als Protokollreferenz. Für den laufenden ESPHome-Betrieb wird keine FHEM-Instanz benötigt. [FHEM-Quellcode](../references/fhem/00_THZ.pm) und [Herkunft und Lizenz](../references/README.md)

## Was in ESPHome portiert ist

- **Transport und Datenfrische.** Nichtblockierende UART-Zustandsmaschine mit einer Transaktion gleichzeitig, 1.800 ms je ACK-/Frame-Phase, maximal drei GET-Versuchen insgesamt und mindestens einer Sekunde Abstand nach Erfolg oder vor dem Retry. Wiederholte Fehler erhöhen die Pause. Die letzte gültige Anzeige bleibt bei Kommunikationslücken sichtbar. Frischesensoren und Datenqualität kennzeichnen fehlende oder veraltete Daten. Home Assistant und das lokale Webinterface lesen aus dem Cache.
- **Messwerte und Diagnose.** Statusblöcke, Temperaturen, Wärmepumpen- und Lüfterdaten, vier Betriebsstundenzähler und geschätzter Verdichter-Betriebszweck. Der Verdampfer-Abtauzustand kommt aus `F2` (`0x10` aus, `0x30` aktiv). Die lokale Lüfterdiagnose erkennt bei angeforderter Stufe und 120 Sekunden frischer Null-Drehzahl je einen Abluft-/Zuluftfehler, speichert ihn und meldet ihn über die HA-Anbindung. Eine explizite Aktion setzt den gespeicherten Status nach bestätigtem LWZ-Neustart zurück.
- **Einstellungen.** 70 belegte P-Felder sind lesbar. Von den 32 gewünschten Einstellungen sind P1–P15, P19, P31, P32, P37–P42, P49, P54–P58 und P79 implementiert. Dazu kommen manuell anwendbare Heizkurvenprofile (P13/P14/P15), dauerhafte Tag-/Nacht-Lüfterstufen und eine manuelle LWZ-Uhrkorrektur um eine Stunde mit Rücklesen. SNTP stellt die ESP-Uhr. Die LWZ-Uhr hat eine eigene Bedienung.
- **Wochenprogramme v0.8.0.** Leseabfragen etwa alle fünf Minuten, je Block ein Frischesensor mit 15 Minuten Frist. Textsensoren zeigen Aktivierung, Start, Ende und Wochentage. Standardmäßig erscheinen ein Heizzeitfenster, das Warmwasserprogramm und zwei Lüftungsfenster.

[Transport und Statusdecoder](../../components/lwz2/lwz2.cpp), [Schreibablauf](../../components/lwz2/v03.cpp), [Lüfterdiagnose](../LUEFTERDIAGNOSE-v0.4.md) und [Parameterkatalog](../PARAMETER-v0.7.md)

## Getroffene Entscheidungen

- Die Auswahl für **Gerätemodell**, **Decoderprofil** und **HK1-Layout** bleibt getrennt. `FD` zeigt die gemeldete Version an. Die manuelle Profilwahl bleibt davon unabhängig und wird gespeichert. Die YAMLs starten mit `2.14j` und HK1-Layout `2.06`. Für das Zielgerät steht ein eigenes `2.16`-Profil auf Basis von 2.06 bereit.
- Schreibfreigabe startet nach jedem ESP-Neustart aus. Ein Bedienbefehl liest den vollen Block frisch, ändert die Zielbytes, sendet einmal SET und prüft das Ergebnis mit einem unabhängigen GET. Bei unklarer SET-Antwort folgt ebenfalls GET. Die Bestätigung stützt sich auf das Rücklesen einschließlich unveränderter Nachbarfelder. Profile werden manuell angewendet. Uhrkorrekturen mit Datumswechsel sind gesperrt.
- Bestehende Entitätsnamen, IDs, Domänen und Geräteidentität sollen erhalten bleiben. Die laufende Zeitsteuerung erfolgt in der LWZ. Datenalter und Gültigkeit sind getrennt vom gehaltenen Anzeigewert zu beachten.
- Ziel ist Geräte-Firmware 2.x. Mehrbyte-Register neuerer Generationen und modellabhängige Zuordnungen brauchen einen Beleg für das Zielgerät.

## Offene Punkte

1. **Zweites Heizzeitfenster HP2.** Die Herstelleranleitung für die ältere LWZ-303-Bedienung beschreibt HP1 und HP2 als zwei Absenkprogramme derselben Heizung. v0.8.0 übernimmt die FHEM-Namen `progHC1`/`progHC2` aus Block `0B` und bietet den zweiten Eintrag deshalb unter `heating_weekly_program_hc2` optional an. In der Nutzerkonfiguration fehlt diese Entität, weil nur ein Heizkreis genutzt wird. Es ist plausibel, dass der zweite Blockeintrag HP2 ist. Vor neuer Beschriftung und Standardaktivierung am Gerät mit getrennt eingestellten HP1-/HP2-Zeiten und Wochentagen abgleichen. Die Anleitung nennt auch 24:00, das der aktuelle Decoder als unbekannt behandelt. Prüfen, wie die LWZ diesen Wert tatsächlich überträgt. [Herstelleranleitung, Seite 4](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-iau.pdf#page=4) und [FHEM-Layout](../references/fhem/00_THZ.pm#L113)
2. **Firmware 2.16.** Ersatzoffsets für die sHC1-Innentemperatur sowie je zwei Ist-/Soll-Drehzahlen aus `E8` fehlen. Für die Lüfterdiagnose gilt `sGlobal` als Standard. F6-Übersteuerungspriorität, F5/F6 im realen Fehlerfall, Verdichterzweck und Booster-Zuordnung weiter prüfen. Aktive Solarfunktion und andere Modelle brauchen eigene Geräteprüfungen. [Decodergrenzen](../DECODER-FW2.16.md) und [Booster-Zuordnung](../BOOSTER-ZUORDNUNG.md)
3. **Schreibfunktionen.** Neu hinzugefügte Parameter-Schreibwege brauchen gezielte Gerätetests. Für P86 fehlt ein belegter 2.x-Zugriff. `0A05A3` aus einer neueren Protokollgeneration wird nicht verwendet. Bearbeitung und Rückschreiben der Wochenprogramme sowie HA-Auswertungen bleiben geplant. [Roadmap](../../ROADMAP.md)

## Weiterarbeiten

- Einstiegspunkte sind die beiden YAMLs, [`components/lwz2/__init__.py`](../../components/lwz2/__init__.py) für ESPHome-Konfiguration und Entitäten sowie [`lwz2.cpp`](../../components/lwz2/lwz2.cpp) für Transport und Scheduler. `v03.cpp` enthält die ausgewählten Decoder und Schreibabläufe, `v04.cpp` modellabhängige Diagnose und Uhr, `parameters.*` die P-Felder und `profiles.cpp` die Heizkurvenprofile.
- Die regulären YAMLs beziehen `lwz2` von GitHub. Für Änderungen im Arbeitsverzeichnis eine YAML mit `source: components` verwenden. Die lokalen Vorlagen liegen im v0.8.0-Quellpaket unter `dist/`. Zugangsdaten gehören in die ignorierte lokale `secrets.yaml`.
- Checks aus dem Projektstamm sind `python tests/run_host_tests.py` mit C++-Compiler und `python -m unittest discover -s tests -p "test_*.py"` im ESPHome-Umfeld. Mit lokaler Komponentenquelle zusätzlich `esphome config` und `esphome compile` ausführen. [CI-Ablauf](../../.github/workflows/validate.yaml)
- Der Arbeitsstand enthält gegenüber Git bereits Änderungen und neue Dateien. Vor einem Commit oder einer Veröffentlichung `git status` prüfen und die benötigten Quellen gezielt aufnehmen. Die Hersteller-PDFs sind separat als lokale Referenzen hinterlegt. [Quellenverzeichnis](../references/README.md)
