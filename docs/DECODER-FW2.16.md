# Decoderprofil FW 2.16 – vorläufig

Stand 07.11.2026. Eigenes auswählbares Profil `2.16`, auf Basis des vorhandenen FHEM-Decoderprofils `2.06`. Grundlage der Kompatibilitätsbewertung ist die Rückmeldung des Nutzers zur Geräte-FW 2.16; keine vollständige unabhängige Geräteverifikation. Keine neuen Offsets oder Skalierungen werden geraten. Die bisherigen Profile bleiben erhalten.

## Verwendung

Im Geräteweb „Firmwareprofil“ auf `2.16` stellen. Alternativ `lwz2.firmware: "2.16"` und die initiale Auswahl `lwz_firmware.initial_option: "2.16"` setzen. Eine bereits gespeicherte Auswahl hat beim Start Vorrang. Die bestehenden Startwerte wurden bei dieser Vorbereitung nicht automatisch umgestellt. `hc1_layout` bleibt separat, zunächst `2.06`; `2.16` ist kein zusätzliches HC1-Layout. Eine FD-Antwort `02.16` schaltet kein Profil automatisch um.

## Übernahmen und Ausnahmen

| Bereich/Feld | Behandlung im Profil 2.16 | Verifikationsstatus |
|---|---|---|
| Übrige vorhandene Lesewerte | 2.06-Regeln übernehmen; HC1 weiter mit separat gewähltem Layout | Nutzer meldet Kompatibilität; kein zusätzlicher Gerätebeweis |
| Innentemperatur sHC1, `s_h_c1_inside_temp` | Nicht verfügbar, auch bei gewähltem 2.14j-HC1-Layout | Vom Nutzer als Ausnahme benannt; Ersatzoffset offen |
| Ist-Drehzahl Zuluft, `supply_fan_speed` | Nicht verfügbar | sFan / `supplyFanSpeed`; Ersatzoffset/Skalierung offen |
| Ist-Drehzahl Abluft, `exhaust_fan_speed` | Nicht verfügbar | sFan / `exhaustFanSpeed`; Ersatzoffset/Skalierung offen |
| Zuluft-Drehzahl-Sollwert, `supply_fan_speed_target` | Nicht verfügbar | sFan / `supplyFanSpeedTarget`; Ersatzoffset/Skalierung offen |
| Abluft-Drehzahl-Sollwert, `exhaust_fan_speed_target` | Nicht verfügbar | sFan / `exhaustFanSpeedTarget`; Ersatzoffset/Skalierung offen |
| Relative Ist-Drehzahlen Zuluft/Abluft aus sGlobal | Weiter verfügbar, Standard für F5/F6 | Nutzer bestätigt Abschalttest >0 auf 0 |
| Speed200 Zuluft/Abluft | Weiter optional, unveränderte 2.06-Regeln, keine erfundene physikalische Einheit | Getrennter Analysewert, keine F5-/F6-Quelle |

Nicht verfügbar bedeutet `NaN` statt eines Zahlenwerts; beim Profilwechsel werden auch alte Werte verworfen. Die sHC1-Ausnahme betrifft nicht die separate Innentemperatur aus sGlobal und nicht den Raum-Sollwert.

Die letzte Nutzerangabe „zuluft-Drehzahl-Sollwert, % …“ wird dem vorhandenen sFan-Sollwert zugeordnet. Die relativen sGlobal-Istwerte sind davon getrennt. Sollte eine andere Entität gemeint sein, ist diese Zuordnung zu korrigieren.

## Diagnose und Schreiben

F5/F6 laufen mit frischen sGlobal-Istwerten weiter. Bei Auswahl `sFan` liefern die ausgeschlossenen Istwerte im Profil 2.16 keine gültige Diagnosegrundlage: Anzeige „Unbekannt“, keine neue Null-Drehzahl-Störung, keine automatische Ersatzquelle. Bereits gespeicherte Fehler bleiben erhalten.

Die vorhandenen Schreibwege und ihre Rückleseprüfung bleiben unverändert. Die Lesekompatibilitätsmeldung belegt keine zusätzliche Schreibfreigabe oder Hardwareprüfung. Statusbits, Lüfter-Konfigurationsblock 01 und dessen Mindestlänge verwenden dieselbe 2.06-Basis; dort nicht belegte Zustände bleiben nicht verfügbar.

## Quellen und offene Prüfung

- Unveränderte [FHEM-Referenz im Paket](references/fhem/00_THZ.pm): `E8fan206`, `F4hc1`, `F4hc1214j`, `FBglob206`, Zuordnung des Profils 2.06. [Gepinnte Originalquelle](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm). Die Regeln stammen aus dieser Quelle; die 2.16-Kompatibilitätsaussage und Ausnahmen stammen vom Nutzer.
- Lokale Umsetzung: `lwz2.cpp`, `v03.cpp`, `v04.cpp`, Python-Konfigurationsschema und beide YAML-Profilselektionen.
- Synthetische Tests vergleichen die Basiswerte, FD-Abfrage, Block-01-Länge und Skalierung, sperren alle fünf Ausnahmen und prüfen F5/F6 mit beiden Quellen einschließlich der 120-Sekunden-Grenze. Kein Ersatz für einen Gerätetest.
- Für eine spätere Korrektur der fünf Ausnahmen fehlen zeitgleiche Rohantworten F4/E8 und verlässliche Vergleichswerte bei Lüfterstufe 0 und laufenden Lüftern. Bis dahin bleiben diese Felder ausdrücklich gesperrt.
