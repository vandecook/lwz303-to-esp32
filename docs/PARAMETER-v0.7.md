# Programmierte Parameter – v0.7.0

Stand: 16.11.2026. Lokales Quellpaket für die LWZ303, Geräte-Firmware 2.16; ESPHome 2026.10.2.

## Bedienung

Der Webserver hat einen eigenen Bereich **Programmierte Parameter**. Er zeigt gespeicherte LWZ-Einstellungen, getrennt von aktuellen Messwerten. Vorhandene Entitätsnamen, explizite IDs, Domänen, Gerätebezeichnungen und Aktivierungszustände bleiben gegenüber 0.6.2 unverändert. Die Zuordnung im Webserver wurde für vorhandene Parametereinstellungen geändert; dadurch entstehen keine neuen HA-Identitäten für diese Entitäten.

Von den 32 zum Einstellen angeforderten Parametern sind **31 umgesetzt**: P1–P15, P19, P31, P32, P37–P42, P49, P54–P58 und P79. P86 bleibt offen, siehe unten. Alle im Katalog belegten nummerierten P-Felder werden lesend berücksichtigt; es wird keine lückenlose Folge bis P86 erfunden. Der Katalog umfasst 70 Felder, darunter zwei unterschiedliche P84-Zuordnungen der Referenz. Die tatsächliche Verfügbarkeit zusätzlicher Felder an Firmware 2.16 ist am Gerät zu prüfen.

Bereits vorhandene Einstellfelder werden wiederverwendet. Wo bisher nur ein Sensor vorhanden war, bleibt dieser für bestehende Dashboards erhalten; ein zusätzliches Zahleneingabefeld ermöglicht nun das Einstellen. Insbesondere werden vorhandene Sensoren für Bereitschaftswerte und Luftmengen nicht in die Domäne `number` verschoben. Neue Eingaben sind nicht optimistisch: Der angezeigte Wert stammt aus der gelesenen Geräteantwort.

1. Aktuellen Wert und Datenstatus prüfen.
2. Bestehende **Schreibfreigabe** einschalten.
3. Gewünschtes Einstellfeld manuell ändern.
4. Den bestehenden Schreibstatus auf bestätigtes Rücklesen prüfen. Ein gesendeter Befehl allein ist keine Erfolgsbestätigung.

Die Schreibfreigabe bleibt nach Neustart aus. Neustart und Wiederverbindung wenden keine Einstellungen erneut an. Gleichzeitige Aufträge werden abgewiesen. Beim Schreiben wird der vollständige betreffende Block frisch gelesen; nur das gewünschte Feld wird geändert. Alle übrigen Bytes einschließlich zusätzlicher Rückgabebytes bleiben erhalten und werden unabhängig zurückgelesen. Bei unklarer Schreibbestätigung wird gelesen, nicht blind erneut geschrieben. Ein Decoder- oder Modellwechsel während eines laufenden Auftrags wird abgewiesen.

## Datenqualität und Modelle

Die zusätzliche Anzeige **Programmierte Parameter gueltig** ist wahr, wenn alle für das gewählte Modell unterstützten Katalogfelder frisch und vollständig beobachtet sind. **Programmierte Parameter Datenstatus** zeigt deren Anzahl und weist auf P86 hin. P86 gehört nicht zum Gültigkeitszähler, da dafür kein belegter Zugriff existiert. Kommunikationslücken behalten die letzten Werte; interne Gültigkeit und Frische werden getrennt bewertet. Ein Neustart beginnt ohne angenommene Parameterwerte.

Die neuen Blöcke 03, 06, 07, 08, 0A, 0E und 10 werden gestaffelt nach dem Start und anschließend im Konfigurationsintervall gelesen. Block 08 wird ausschließlich bei gewähltem SOL-Modell abgefragt. Bestehende Abfragen 01, 05 und 17 werden wiederverwendet; zusätzliche Entitäten verursachen keine doppelten Abfragen. Statusabfragen und bestehende Kommunikationsabstände bleiben maßgeblich. Die bestehende manuelle Konfigurationsaktualisierung stößt auch die neuen Blöcke an.

P25 wird ausschließlich für SOL ausgewertet. P84 ist in der FHEM-Referenz zweifach benannt: Warmwasserpuffer in 07 und Solar-Warmwassertemperatur in 08. Deshalb bleiben diese Felder getrennt und nur lesbar. Weitere Hysteresen P26–P28 sind Referenzfelder und kein Nachweis zusätzlicher Heizstufen. Die übrigen nur lesbaren Felder erhalten ebenfalls keine neue Schreibfreigabe. Insbesondere werden Trockenheizen, Solar- oder andere Programme durch diese Erweiterung nicht gestartet. Kühlungsstunden bleiben ausgeschlossen.

## Grenzen und Herkunft

**P86 ist nicht umgesetzt:** In der mitgelieferten FHEM-Referenz findet sich `p86OutTempCorrection` mit dem mehrbyteigen Register `0A05A3` für eine neuere Protokollfamilie. Ein 2.x-Block samt Offset und Schreibverfahren für die LWZ303 mit Firmware 2.16 ist nicht belegt. Dieses Register wird nicht an die LWZ gesendet; es gibt kein funktionsloses P86-Einstellfeld. Ein passender Quellenbeleg oder eine belastbare Geräteanalyse ist für die Ergänzung erforderlich.

Die Definitionen folgen den unverändert mitgelieferten FHEM-2.x-Decodern. Für Firmware 2.16 gilt weiterhin das dokumentierte 2.06-basierte Layout; 2.14/2.14j verwenden für Block 01 andere Feldbreiten. Die Softwaretests prüfen alle vier Profile, ersetzen aber keine Gerätebestätigung der neu hinzugefügten Parameter.

Wertebereiche neuer Einstellungen folgen den FHEM-`pclean`-Definitionen. Für P31 ist das 2.x-Feld belegt, die konservative Eingabe 0–3 folgt der FHEM-Zusatzheizstufenbegrenzung; am Gerät prüfen. Bereits bestehende engere UI-Bereiche bleiben erhalten: P1/P2 15–25 °C, P4/P5 35–55 °C, P13 0–1 und P14 0–5 K. Diese Bereiche werden durch das Update nicht stillschweigend erweitert. P15 bleibt 0–10, P7/P8 bleiben 0–3.

Quellen: [mitgelieferte FHEM-Referenz](references/fhem/00_THZ.pm), [maschinenlesbarer Katalog](../components/lwz2/parameters.json). Der Katalog enthält Quellenhash, Block, Offset, Feldbreite, Skalierung, Vorzeichen und Grenzen. Er wird mit `research/prepare_v070.py` reproduzierbar aus dieser Referenz erzeugt.

## Parameterkatalog

Die folgende Tabelle zeigt den Softwareumfang. Modellabhängige oder am Gerät nicht bestätigte Felder sind keine zugesicherte Gerätefunktion. Die Katalogbereiche der wiederverwendeten Felder werden in der UI wie oben beschrieben bewusst enger gehalten.

| Parameter | Anzeige/Funktion | Block | Bedienung | Bereich neuer Eingaben | Einheit |
|---|---|---|---|---|---|
| P01 | heating_day_setpoint (Name unverändert) | 17 | Einstellen (vorhanden) | – | °C |
| P02 | heating_night_setpoint (Name unverändert) | 17 | Einstellen (vorhanden) | – | °C |
| P03 | Raum-Sollwert Bereitschaft einstellen | 17 | Einstellen | 10–30; Schritt 0.1 | °C |
| P04 | dhw_day_setpoint (Name unverändert) | 17 | Einstellen (vorhanden) | – | °C |
| P05 | dhw_night_setpoint (Name unverändert) | 17 | Einstellen (vorhanden) | – | °C |
| P06 | Warmwasser Bereitschaft einstellen | 17 | Einstellen | 10–55; Schritt 0.1 | °C |
| P07 | fan_stage_day (Name unverändert) | 17 | Einstellen (vorhanden) | – |  |
| P08 | fan_stage_night (Name unverändert) | 17 | Einstellen (vorhanden) | – |  |
| P09 | Luefterstufe Bereitschaft einstellen | 17 | Einstellen | 0–3; Schritt 1 |  |
| P10 | Heizung Manuell Sollwert | 17 | Einstellen | 10–65; Schritt 0.1 | °C |
| P11 | Warmwasser Manuell Sollwert | 17 | Einstellen | 10–65; Schritt 0.1 | °C |
| P12 | Luefterstufe Manuell | 17 | Einstellen | 0–3; Schritt 1 |  |
| P13 | hc1_gradient (Name unverändert) | 05 | Einstellen (vorhanden) | – |  |
| P14 | hc1_low_end (Name unverändert) | 05 | Einstellen (vorhanden) | – | K |
| P15 | hc1_room_influence (Name unverändert) | 05 | Einstellen (vorhanden) | – |  |
| P16 | Heizkurve HK2 Gradient | 05 | Lesen | – |  |
| P17 | Heizkurve HK2 Fusspunkt | 05 | Lesen | – | K |
| P18 | Heizkurve HK2 Raumeinfluss | 05 | Lesen | – |  |
| P19 | Vorlaufanteil HK1 | 05 | Einstellen | 0–100; Schritt 1 | % |
| P20 | Vorlaufanteil HK2 | 05 | Lesen | – | % |
| P21 | Hysterese Heizstufe 1 | 06 | Lesen | – | K |
| P22 | Hysterese Heizstufe 2 | 06 | Lesen | – | K |
| P23 | Hysterese Heizstufe 3 | 06 | Lesen | – | K |
| P24 | Hysterese Heizstufe 4 | 06 | Lesen | – | K |
| P25 | Hysterese Heizstufe 5 (SOL) | 06 | Lesen | – | K |
| P26 | Weitere Hysterese 6 | 06 | Lesen | – |  |
| P27 | Weitere Hysterese 7 | 06 | Lesen | – |  |
| P28 | Weitere Hysterese 8 | 06 | Lesen | – |  |
| P29 | Hysterese Asymmetrie | 06 | Lesen | – |  |
| P30 | Heizungsregelung Integralanteil | 06 | Lesen | – | Kmin |
| P31 | Maximale Zusatzheizstufen Heizung | 06 | Einstellen | 0–3; Schritt 1 |  |
| P32 | Warmwasser Hysterese | 07 | Einstellen | 2–10; Schritt 0.1 | K |
| P33 | Zusatzheizung Wartezeit Warmwasser | 07 | Lesen | – | min |
| P34 | Warmwasser Zusatzheizung Temperaturgrenze | 07 | Lesen | – | °C |
| P35 | Legionellenprogramm Intervall | 07 | Lesen | – | Days |
| P36 | Warmwasser maximale Ladedauer | 07 | Lesen | – | h |
| P37 | Zuluft Stufe 1 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P38 | Zuluft Stufe 2 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P39 | Zuluft Stufe 3 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P40 | Abluft Stufe 1 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P41 | Abluft Stufe 2 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P42 | Abluft Stufe 3 einstellen | 01 | Einstellen | 60–250; Schritt 1 | m³/h |
| P43 | Lueftung Stufe 3 Zeitvorgabe | 01 | Lesen | – | min |
| P44 | Lueftung Stufe 2 Zeitvorgabe | 01 | Lesen | – | min |
| P45 | Lueftung Stufe 1 Zeitvorgabe | 01 | Lesen | – | min |
| P46 | Lueftung Stufe 0 Zeitvorgabe | 01 | Lesen | – | min |
| P47 | Verdichter Wiederanlaufverzoegerung | 03 | Lesen | – | min |
| P48 | Hauptluefter Vorgabe | 03 | Lesen | – | % |
| P49 | Sommerbetrieb Temperatur | 06 | Einstellen | 10–24; Schritt 0.1 | °C |
| P50 | Sommerbetrieb Hysterese | 06 | Lesen | – | K |
| P54 | Pumpenzyklen Minimum | 0A | Einstellen | 1–24; Schritt 1 | /Day |
| P55 | Pumpenzyklen Maximum | 0A | Einstellen | 25–288; Schritt 1 | /Day |
| P56 | Aussentemperatur maximale Pumpenzyklen | 0A | Einstellen | 0–20; Schritt 0.1 | °C |
| P57 | Aussentemperatur minimale Pumpenzyklen | 0A | Einstellen | 0–25; Schritt 0.1 | °C |
| P58 | Temperaturerfassung nach Pumpenstart sperren | 0A | Einstellen | 0–120; Schritt 1 | s |
| P59 | Wiederanlauf vor Absenkende | 0E | Lesen | – |  |
| P70 | Trockenheizen Startzustand | 10 | Lesen | – |  |
| P71 | Trockenheizen Sockeltemperatur | 10 | Lesen | – |  |
| P72 | Trockenheizen Maximaltemperatur | 10 | Lesen | – |  |
| P73 | Trockenheizen Temperaturdauer | 10 | Lesen | – |  |
| P74 | Trockenheizen Temperaturanstieg | 10 | Lesen | – |  |
| P75 | Passive Kuehlung Vorgabe | 01 | Lesen | – |  |
| P77 | Aussentemperatur Filterzeit | 06 | Lesen | – | h |
| P78 | Bivalenzpunkt | 06 | Lesen | – | °C |
| P79 | Zusatzheizung Wartezeit Heizung | 06 | Einstellen | 0–60; Schritt 1 | min |
| P80 | Solar Freigabe | 08 | Lesen | – |  |
| P81 | Solar Ladung Temperaturdifferenz | 08 | Lesen | – |  |
| P82 | Solar Verdichterstart Verzoegerung | 08 | Lesen | – |  |
| P84 | Warmwasserpuffer Freigabe | 07 | Lesen | – |  |
| P84 | Solar Warmwassertemperatur | 08 | Lesen | – |  |

[Prüfbericht](VALIDATION-v0.7.md).
