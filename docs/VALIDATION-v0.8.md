# Wochenprogramme in v0.8.0

Im ESPHome-Webinterface unter **Wochenprogramme** und als Textsensoren in Home Assistant erscheinen Heizkreis 1, Warmwasser und die beiden Lüftungszeitfenster. Der Sensorwert `Aktiviert | 06:50–22:10 | Mo, Mi, Fr` zeigt ein Zeitfenster für Montag, Mittwoch und Freitag.

Seit v0.8.1 ist der zweite Eintrag aus `0B` in den mitgelieferten Konfigurationen als „Wochenprogramm Heizprogramm 2“ (`heating_weekly_program_hc2`) aktiviert. Die Herstelleranleitung beschreibt HP1 und HP2 als zwei Absenkprogramme derselben Heizung. FHEM nennt die Einträge `progHC1`/`progHC2`. Die bestehende Entität „Wochenprogramm Heizkreis 1“ zeigt HP1 und behält Namen und ID.

| LWZ-Block | Inhalt |
| --- | --- |
| `0B` | Je ein Zeitfenster für Heizprogramm 1 (HP1) und Heizprogramm 2 (HP2) |
| `0C` | Ein Zeitfenster für Warmwasser |
| `0D` | Zwei Zeitfenster für Lüftung |

Die Abfragen starten gestaffelt nach dem Einschalten und liegen danach etwa fünf Minuten auseinander. Seit v0.9.0 gilt für die Wochenprogramme das Parameterintervall (Startwert 60 Minuten), die Frist das Dreifache davon. Die Frische-Anzeigen beziehen sich auf die letzte empfangene Antwort des jeweiligen Blocks. Nach mehr als 15 Minuten ohne gültige Antwort erlischt die Frische-Anzeige. Die zuletzt empfangenen Zeiten bleiben sichtbar. Vor der ersten gültigen Antwort zeigt der Textsensor „Nicht verfügbar“ an. Der ESP sendet für die Wochenprogramme ausschließlich Leseanfragen.

Die Zeitwerte sind 16-Bit-Zahlen in Big-Endian-Reihenfolge. Der dezimale Wert 650 steht für 06:50. Die Wochentage belegen die Bits 0 bis 6 von Montag bis Sonntag. Das Aktivierungsbyte hat die Werte 0 oder 1. [Mitgelieferte FHEM-Referenz](references/fhem/00_THZ.pm#L113-L139)

## Prüfstand

- Automatisierte C++-Tests prüfen die drei Blockformate, alle Wochentage, Zeiten über Mitternacht, deaktivierte Einträge, unplausible Felder, abgeschnittene Antworten, Datenalter und Wiederverbindung. Ein simulierter UART-Austausch prüft die Warmwasserabfrage.
- Der Vergleich mit den am Gerät gespeicherten Wochenprogrammen auf Firmware 2.16 steht aus.
