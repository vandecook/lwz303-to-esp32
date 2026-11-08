# Mapping – Heizungs- und Warmwasseranforderung

Festlegung vom 08.11.2026: `sControl.hcStage` und `sControl.dhwStage` aus **F2ctrl** bilden die bevorzugte **Basisanzeige**. Das folgende Mapping gehört zum **Gerätemodell LWZ 303 / 403 SOL**, ausdrücklich auch bei gewähltem **Decoderprofil 2.16**. Modellwahl und Firmwareprofil bleiben getrennt.

| Feld | Quelle und Mapping | Bewertung |
|---|---|---|
| `sControl.hcStage` | F2ctrl, Kommentar: **0 aus, 1 Solar, 2 Wärmepumpe, 3 boost1, 4 boost2, 5 boost3** | Kommentar passt zur SOL-Heizstufenfolge; vom Nutzer als plausibel zur Verwendung freigegeben. Keine eigene i-Tabelle und kein Geräteversuch für FW 2.16. |
| `sControl.dhwStage` | F2ctrl, Kommentar: **0 aus, 1 Solar, 2 Wärmepumpe, 3 boostMax** | Code 3 bezeichnet laut Kommentar maximale Booster-Anforderung. Daraus folgt keine feste Leistung; die Bedeutung des Codes ist keine Aussage über die tatsächlich freigegebene oder geschaltete Leistung. |

Die Software zeigt diese Werte als **angeforderte** Stufen. `boost1` bis `boost3` heißen dort „Nacherwärmstufe 1“ bis „Nacherwärmstufe 3 angefordert“. `boostMax` heißt „Maximale freigegebene Nacherwärmung angefordert“. Es werden keine kW-Werte berechnet. Der Code 3 allein weist weder die Freigabe noch deren aktuelle Höhe nach.

## Anzeigezuordnung

| Bereich | Entitäten/Quellen | Darstellung |
|---|---|---|
| Basis | `hc_stage`, `dhw_stage`, `hc_stage_text`, `dhw_stage_text` | F2-Rohcode und zugehöriger Anforderungstext; im Geräteweb unter Basis, in HA ohne Diagnosekategorie |
| Expert | `sDHW.dhwBoosterStage`, `sHC1.hcBoosterStage`, `sHC1.hcStage` | Separate alternative Rohwerte, keine Gleichsetzung mit F2 |
| Expert | `sGlobal.boosterStage1/2/3` | Einzelne Statusbits; keine automatische Zuordnung zu Leistungsstufen |

Die Umgruppierung der anderen Quellen ändert deren bestehende Aktivierung nicht. Bereits vorhandene HA-Entitäten können ihre bisherige Kategorie bis zur Aktualisierung durch HA behalten. Die Gruppierung Basis/Expert gilt explizit für das Geräteweb; HA erhält die entsprechende Diagnosekategorie, aber keine automatisch erzeugten Dashboardbereiche.

Bei Modell **i** oder **Unbekannt** bleiben F2-Rohcodes sichtbar; es wird keine SOL-Tabelle übertragen. Nicht definierte SOL-Codes bleiben Rohcodes. Das Modell SOL allein beweist nicht den Anschluss einer Solaranlage. Die Zuordnung gilt im Code für alle auswählbaren Decoderprofile einschließlich 2.16; eine automatische Modellwahl findet nicht statt.

## Quelle und Verifikation

[Unveränderte FHEM-Referenz](references/fhem/00_THZ.pm), Definition `F2ctrl`; [gepinnte Originalquelle](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm). Die Tabelle ist **im Quellkommentar belegt und vom Nutzer für SOL akzeptiert**, jedoch **nicht unabhängig an FW 2.16 am Gerät verifiziert**. Die lokalen Tests prüfen die Softwarezuordnung einschließlich unbekannter Codes und der Abgrenzung zum Modell i. Weitere Grenzen und die P21–P25-Zuordnung: [Booster-Zuordnung](BOOSTER-ZUORDNUNG.md).
