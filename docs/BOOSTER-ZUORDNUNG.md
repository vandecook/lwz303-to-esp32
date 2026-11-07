# Heizstufen und Booster-Zuordnung

Prüfstand: 07.11.2026. Zielgerät laut Nutzer: Firmware 2.16. Gerätevariante und FHEM-Decoderlayout sind getrennte Merkmale.

## Durch Herstellerunterlagen bestätigt

Die Herstelleranleitung A 166459-37161-8835, Seite 70/71, bestätigt die Heiz-Hysteresenzuordnung:

| Parameter | LWZ 303 i / Integral | LWZ 303 SOL / 403 SOL |
|---|---|---|
| P21 | Stufe 1: Wärmepumpe/Verdichter | Stufe 1: Solar |
| P22 | Stufe 2: NE 2,6 kW | Stufe 2: Wärmepumpe/Verdichter |
| P23 | Stufe 3: NE 5,6 kW | Stufe 3: NE 2,6 kW |
| P24 | Stufe 4: NE 8,8 kW | Stufe 4: NE 5,6 kW |
| P25 | Nur SOL; bei i nicht vorgesehen | Stufe 5: NE 8,8 kW |

P21–P25 sind Hysteresen in K. Die kW-Werte bezeichnen Leistungsniveaus der elektrischen Nacherwärmung, keine fünf unterschiedlichen Heizpatronen und keine Messwerte. Die Werte 2,6 / 5,6 / 8,8 kW werden nicht addiert.

Heizung: Freigabe über P31. Warmwasser: eigene Einstellung der freigegebenen NE-Stufen; Solar hat bei ausreichendem Ertrag Vorrang vor Verdichter und elektrischer Nacherwärmung. Die Warmwasserregelung verwendet nicht einfach die Heiz-Hysteresentabelle P21–P25.

Quelle: [Herstelleranleitung, Seiten 47, 48, 62, 70 und 71](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-nsm.pdf). Seiten 47 und 71 wurden auch visuell geprüft. Die vom Nutzer genannte ältere Fachmann-Anweisung, Seite 20 unten, bleibt ein zusätzlicher Quellenhinweis; ihre konkrete Ausgabe wurde nicht identifiziert. Die neu geprüfte Anleitung ist kein Nachweis identischer Protokollcodes für Firmware 2.16.

Die Schaltpläne zeigen bei i und SOL die elektrische Nacherwärmung E1 und deren DHC-Anschlüsse X17. Für Heizung und Warmwasser wird damit keine separate zweite elektrische Heizpatrone belegt. Die Zuordnung einzelner RS232-Bits zu Relais oder Widerstandszweigen fehlt im Schaltplan.

Quelle: [Gebrauchs- und Montageanweisung 166459-34227-8160, Seiten 17/18, Abb. 14a/14b](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-iau.pdf); beide Schaltpläne visuell geprüft.

## Was der tatsächliche FHEM-Decoder belegt

Basis: lokal archivierte `research/00_THZ.pm`, Mirror-Commit `0178ba5567d8b203db51215d29406454f1c7d001`; kein pauschaler Hardware-Kompatibilitätsnachweis.

| Feld | Quellstelle / belegte Aussage | Grenze der Aussage |
|---|---|---|
| `sControl.hcStage` | F2ctrl, Kommentar: 0 aus, 1 Solar, 2 Wärmepumpe, 3 boost1, 4 boost2, 5 boost3 | Kommentar passt zur SOL-Heizstufenfolge; keine eigene i-Tabelle und kein Geräteversuch für FW 2.16 |
| `sControl.dhwStage` | F2ctrl, Kommentar: 0 aus, 1 Solar, 2 Wärmepumpe, 3 boostMax | Code 3 bezeichnet laut Kommentar maximale Booster-Anforderung; daraus folgt keine feste Leistung, unabhängig von der Freigabe |
| `sDHW.dhwBoosterStage` | F3dhw: ein Byte als unsigned Integer | Keine Wertetabelle; nicht als identisch mit F2.dhwStage oder als bestätigte Ist-Leistungsstufe behandeln |
| `sHC1.hcBoosterStage` / `hcStage` | F4: dieselbe Position, Hex-Zeichen 36, bei 2.06/2.14 bzw. 2.14j unterschiedlich benannt; 2.14j kommentiert 0 aus, 1 Solar, 2 V1 | Der Name „BoosterStage“ allein beweist keine reine elektrische Stufe 0–3; auch keine ungeprüfte Gleichsetzung mit F2.hcStage |
| `sGlobal.boosterStage1/2/3` | Einzelne Bitwerte, kein gemeinsamer Integer | Nicht mit P21/P22/P23 oder mit 2,6/5,6/8,8 kW gleichsetzen |

Decoderquellen: [F2/F3/F4, Zeilen 200–278](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L200), [FB-Layouts, Zeilen 306–340](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm#L306).

Für die folgende Tabelle bedeutet Bit jeweils das Bit innerhalb des Nibbles an Hex-Zeichenposition 44 der FHEM-internen Antwortzeichenfolge (inklusive Prüfsummen- und Befehlsbyte). Das ist kein absolutes Byte im seriellen Drahttelegramm.

| Feld | Decoder 2.06 | Decoder 2.14 und 2.14j |
|---|---|---|
| Verdichter | Bit 0 | Bit 0 |
| boosterStage1 | Bit 1 | Bit 1 |
| boosterStage2 | Bit 2 | Bit 3 |
| boosterStage3 | Bit 3 | Nicht verfügbar (`n.a.`) |
| Solarpumpe | Nicht verfügbar | Bit 2 |

**Folgerung:** Beim Wechsel des Decoderlayouts kann dasselbe Rohbit einmal „Booster 2“ und einmal „Solarpumpe“ heißen. Die Anzahl der im Decoder verfügbaren Boosterbits beweist nicht die Anzahl der elektrischen Leistungsniveaus. Zwei Bits könnten drei aktive Zustände codieren; eine konkrete Zuordnung 01/10/11 zu Leistungen ist hier ausdrücklich nicht nachgewiesen.

Die bislang vom Nutzer gelieferten Beobachtungen enthalten nur ausgeschaltete Booster. Sie belegen keine aktive Kombination. Der bereits dokumentierte F2type/F2ctrl-Verweisfehler bei 2.14/j betrifft die Verfügbarkeit von sControl; ein reparierter Verweis bestätigt noch keine physikalische Zuordnung.

## Konsequenz für Anzeige und Geräteprüfung

### v0.4 – Geräteauswahl und sControl einschließlich FW 2.16

Nutzerfestlegung vom 07.11.2026: eigene Auswahl **LWZ 303 i / Integral** oder **LWZ 303 / 403 SOL** sowie **sControl.hcStage und sControl.dhwStage** für **v0.4** einplanen, ausdrücklich **auch für Geräte-Firmware 2.16**. Für noch nicht zugeordnete Anlagen zusätzlich **Unbekannt / nur Rohwerte** anbieten. Geräteauswahl und F2-Rohcodes sind in v0.4 als Software umgesetzt; eine Bestätigung am Gerät steht aus. SOL-Texte folgen den FHEM-Kommentaren, i bleibt bis zum Nachweis bei Rohcodes.

- Gerätevariante, tatsächliche Geräte-Firmware und Decoderlayout unabhängig führen. Die Modellwahl darf insbesondere nicht automatisch 2.06 oder 2.14/j erzwingen.
- Bei i Solar-Funktionen und P25 in der modellbezogenen Bedienung ausblenden. Bei SOL Solar-Funktionen als verfügbar behandeln; die Auswahl SOL allein bedeutet nicht, dass tatsächlich eine Solaranlage angeschlossen ist.
- P21–P25 gemäß der bestätigten Herstellerzuordnung beschriften. Diese Parameterzuordnung darf nicht ungeprüft auf die Stufencodes des seriellen Protokolls übertragen werden.
- `sControl.hcStage` und `sControl.dhwStage` in v0.4 für die getrennte Anzeige der Heizungs- und Warmwasseranforderung aus F2 integrieren, einschließlich Geräte-Firmware 2.16. Der Nutzer bewertet die vorhandenen F2ctrl-Kommentare als plausibel; sie dienen als Ausgangspunkt. Die Bestätigung der Codes für beide Modelle und Firmware 2.16 ist Teil der Umsetzung und Geräteprüfung.
- Modellabhängige Tabellen für F2-Heizung und F2-Warmwasser getrennt verifizieren. Die SOL-Kommentartabelle nicht durch bloßes Abziehen von 1 in eine i-Tabelle umrechnen. Den F2type-Verweisfehler gezielt behandeln, ohne deshalb das gesamte Decoderprofil zu wechseln.
- Die Anzeige als **angeforderte Stufe** kennzeichnen, solange kein Nachweis für die tatsächlich geschaltete Leistung vorliegt. Globale Boosterbits bleiben getrennte Diagnosewerte. Unbekannte Codes als Rohcode/„unbekannt“ ausgeben; keine geratenen kW-Werte.

### Bis zur bestätigten Zuordnung

- Bis zum Nachweis die vorhandenen Rohwerte mit Quellfeld und Decoderlayout anzeigen. Keine zusätzlichen kW-Werte aus den Bits berechnen.
- Elektrische NE-Leistungsstufe und allgemeine Wärmeerzeugerstufe getrennt führen. Bei SOL ist Solar eine zusätzliche allgemeine Stufe; daraus folgt keine zusätzliche elektrische NE-Stufe.
- Heizungs-/Warmwasserzuordnung zeitgleich anhand von Prozesszustand, Pumpen und Geräteanzeige prüfen. Pumpennachlauf, Abtauen und widersprüchliche Zustände als unklar behandeln; Pumpenbits allein beweisen keine Wärmeerzeugung.
- Für FW 2.16 frische Rohantworten FB/F2/F3/F4 sowie Geräteanzeige bei Verdichterbetrieb und jeder tatsächlich auftretenden NE-Stufe vergleichen, getrennt für Heizung und Warmwasser. Freigabeparameter und aktive Anforderung ebenfalls festhalten. Zur Erfassung keine Schutzfunktionen überbrücken oder Leistungsstufen automatisch einschalten.
- Eine spätere beschriftete Auswertung braucht eine je Gerätemodell und geprüftem Layout belegte Wertetabelle. v0.4 ergänzt F2-Rohcodes unabhängig vom fehlerhaften F2type-Verweis; eine feste Bit-zu-kW-Zuordnung bleibt nicht freigegeben.
