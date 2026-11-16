"""Produce the parameter catalogue and release notes for the repository."""
from pathlib import Path
import json

root=Path(__file__).resolve().parents[1]
catalogue=json.loads((root/'components/lwz2/parameters.json').read_text(encoding='utf-8'))
rows=catalogue['parameters']
text='''# Programmierte Parameter – v0.7.0

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
'''
for r in rows:
    mode='Einstellen (vorhanden)' if r['legacy'] else 'Einstellen' if r['writable'] else 'Lesen'
    limits=f'{r["min"]}–{r["max"]}; Schritt {r["step"]:g}' if r['writable'] and not r['legacy'] else '–'
    label=r['label'] if not r['legacy'] else r['key']+' (Name unverändert)'
    text+=f'| P{r["number"]:02d} | {label} | {r["block"]} | {mode} | {limits} | {r["unit"]} |\n'
text+='\n[Prüfbericht](VALIDATION-v0.7.md).\n'
(root/'docs/PARAMETER-v0.7.md').write_text(text,encoding='utf-8')

p=root/'README.md';s=p.read_text(encoding='utf-8')
s=s.replace('LWZ303 to ESP32 – v0.6.2','LWZ303 to ESP32 – v0.7.0',1)
s=s.replace('## Aktueller Funktionsstand\n','''## Aktueller Funktionsstand

**v0.7.0:** Eigener Bereich „Programmierte Parameter“ mit 70 belegten P-Feldern. 31 der 32 gewünschten Parameter sind einstellbar: P1–P15, P19, P31, P32, P37–P42, P49, P54–P58 und P79. **P86 bleibt mangels belegtem 2.x-Zugriff offen.** Bestehende Entitätsnamen, IDs und HA-Zuordnungen bleiben erhalten; vorhandene reine Sensoren erhalten bei Bedarf ein zusätzliches Einstellfeld. Schreiben mit frischem Lesen, gezielter Änderung und unabhängiger Rückleseprüfung. [Bedienung, Katalog und Grenzen](docs/PARAMETER-v0.7.md), [Prüfbericht](docs/VALIDATION-v0.7.md).
''',1)
s=s.replace('Zuordnung für FW 2.16 weiterhin vorläufig.','Abtau-Integration am 16.11.2026 vom Nutzer am Gerät als funktionierend bestätigt.')
s=s.replace('Solange v0.6.2 noch nicht','Solange v0.7.0 noch nicht')
s=s.replace('[Validierungsstand](docs/VALIDATION-v0.6.2.md)','[Validierungsstand](docs/VALIDATION-v0.7.md)')
s=s.replace('Prüfstand: [Softwarevalidierung v0.6.2](docs/VALIDATION-v0.6.2.md).','Prüfstand: [Softwarevalidierung v0.7.0](docs/VALIDATION-v0.7.md).')
p.write_text(s,encoding='utf-8')
p=root/'CHANGELOG.md';s=p.read_text(encoding='utf-8');at=s.index('\n## ')
s=s[:at]+'''
## 0.7.0 - 2026-11-16 (lokales Quellpaket)

- Eigener Bereich „Programmierte Parameter“; 70 nummerierte P-Felder aus den belegten 2.x-Blöcken lesen. SOL-Felder modellabhängig, P25 bei i nicht auswerten; keine Kühlungsstunden.
- P1–P15, P19, P31, P32, P37–P42, P49, P54–P58 und P79 einstellbar. Bestehende neun Einstellfelder weiterverwenden, 22 ergänzen. P86 ist mangels belegtem 2.x-Zugriff weiterhin offen; kein Zugriff auf das neuere Register 0A05A3.
- Bestehende Entitätsnamen, explizite IDs, Domänen und Geräteidentität erhalten. Bisherige reine Sensoren bleiben erhalten, zusätzliche Eingabefelder ergänzen die Bedienung. Eigener automatischer Kompatibilitätsvergleich mit dem 0.6.2-Bestand.
- Vollständigen Block frisch lesen, nur Zielfeld ändern, alle übrigen Bytes erhalten und unabhängig zurücklesen; bei unklarer SET-Antwort keine automatische Wiederholung. Modell- und Decoderwechsel während eines Auftrags sperren.
- Datenfrische der Parameter getrennt anzeigen, letzte Werte bei Kommunikationslücken behalten. Neue Blöcke gestaffelt im Konfigurationsintervall abfragen; keine doppelten Abfragen für vorhandene Quellen.
- Abtau-Integration aus 0.6.2 am 16.11.2026 vom Nutzer als funktionierend bestätigt. Neue Parameter-Schreibwege benötigen ihre eigene Geräteprüfung. HA-Auswertungen und Wochenprogramme bleiben separat geplant.
''' + s[at:]
p.write_text(s,encoding='utf-8')
p=root/'VALIDATION.md';s=p.read_text(encoding='utf-8');p.write_text('Aktueller Stand: [Validierung v0.7.0](docs/VALIDATION-v0.7.md).\n\n'+s,encoding='utf-8')
print('Wrote v0.7 parameter documentation, release notes and current report links.')
