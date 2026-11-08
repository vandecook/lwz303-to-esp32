# Heizkurvenprofile in ESPHome – v0.5

Stand 08.11.2026. Profile werden im ESP32 gespeichert und ausschließlich manuell angewendet. Sie enthalten immer **P13 Gradient, P14 Fußpunkt und P15 Raumeinfluss**. Die getrennten Anlagen-Slider bleiben bedienbar. Speichern oder Auswählen eines Profils schreibt nicht in die Wärmepumpe.

## Speicherplätze und Startvorschläge

| Speicherplatz | Anfangsname | P13 | P14 | P15 | Zustand |
|---|---|---:|---:|---:|---|
| Profil 1 | Uebergangszeit | 0,2 | 2,0 K | 0 | Hinterlegter Versuchsvorschlag |
| Profil 2 | Winter | 0,4 | 1,0 K | 0 | Hinterlegter Versuchsvorschlag für zu kühle Räume bei Kälte |
| Profil 3–5 | Eigenes Profil 1–3 | 0 | 0 K | 0 | Leer, erst nach Speichern anwendbar |

Die Vorschläge sind **keine Herstellerprogramme oder bestätigte optimale Einstellungen**. Herstellerstandard 0,6/0/0 und zuletzt vom Nutzer gemeldete Einstellung 0,3/1,0 K sind in der [HK1-Dokumentation](HEIZKURVE-HK1.md) beschrieben. Keiner dieser Werte wird beim Start automatisch auf die Anlage übertragen.

Alle fünf Plätze können bearbeitet und benannt werden. „Profil 1“ bis „Profil 5“ bleiben feste Platzbezeichnungen im Auswahlfeld; der gespeicherte Name erscheint daneben unter „Profil gespeichert / Editorstatus“ und im Namen-Editor. Namen müssen eindeutig sein, 1–31 UTF-8-Bytes lang sein und dürfen keine Steuerzeichen enthalten. Umlaute benötigen mehr als ein Byte.

## Bedienung

1. Unter **Heizkurvenprofile** den Speicherplatz auswählen.
2. Name, Gradient und Fußpunkt im **Entwurf** einstellen. Der Raumeinfluss-Entwurf steht unter **Expert**. Grenzen: P13 0–1, P14 0–5 K, P15 0–10, jeweils Schritt 0,1.
3. Optional „Profil Anlagenwerte jetzt lesen“ und anschließend „Profil Anlagenwerte in Entwurf“ verwenden. Das übernimmt nur vollständige, frische Werte innerhalb der Profilbereiche. Es schreibt nichts in die LWZ.
4. „Profil Entwurf speichern“ speichert den Entwurf dauerhaft im ESP32. „Profil Entwurf verwerfen“ stellt die im gewählten Platz gespeicherten Werte wieder her. Ein ungespeicherter Entwurf blockiert Platzwechsel und Anwendung, damit Änderungen nicht stillschweigend verloren gehen.
5. Erst mit eingeschalteter **LWZ Schreibfreigabe** und der Taste **Profil jetzt anwenden** wird das gespeicherte Profil auf die Anlage übertragen. Eine laufende Schreibtransaktion blockiert parallele Änderungen. Aktionsstatus und Schreibstatus zeigen das Ergebnis.

Die Auswahl, die gespeicherten Profile und die letzte bestätigte Anwendung bleiben über ESP-Neustarts erhalten. Ungespeicherte Entwürfe werden nach Neustart verworfen. Die Schreibfreigabe startet weiterhin ausgeschaltet. Es gibt **keine automatische Anwendung** bei Neustart, Wiederverbindung, Temperaturänderung, Jahreszeitenwechsel oder manueller Abweichung. Es wird auch nicht automatisch zu einem früheren Profil zurückgeschrieben.

## Erkennung und Anzeigen

| Anzeige | Bedeutung |
|---|---|
| Profil gespeichert / Editorstatus | Gewählter Speicherplatz, gespeicherter Name, leer oder ungespeicherter Entwurf |
| Erkanntes Heizkurvenprofil | Namen aller gespeicherten Profile, deren P13/P14/P15 genau zu den tatsächlich ausgelesenen Rohwerten passen |
| Benutzerdefiniert | Vollständig ausgelesene Kombination passt zu keinem Profil; gilt auch nach manueller Änderung von P15 |
| Mehrere Profilnamen | Mehrere gespeicherte Profile enthalten dieselbe Kombination; es wird keine eindeutige Zuordnung vorgetäuscht |
| Unbekannt | Seit Start/Decoderwechsel fehlen vollständige P13/P14/P15-Werte |
| Veraltet: … | Nur die letzte gelesene Kombination ist noch bekannt; die bisherige Zuordnung wird ausdrücklich als veraltet angezeigt |
| Profil zuletzt angewendet | Zuletzt durch unabhängiges Rücklesen bestätigte und gespeicherte Anwendung; **kein Nachweis**, dass dieses Profil jetzt noch aktiv ist |
| Profil Datenstatus | Alter und zuletzt gelesene P13/P14/P15-Werte |

Eine Änderung über die Anlagen-Slider wird beim zugehörigen Lesen/Schreiben/Rücklesen erkannt. Änderungen direkt an der LWZ werden erst mit der nächsten Block-05-Abfrage erkannt. „Profil Anlagenwerte jetzt lesen“ fordert die Abfrage vorzeitig an. Die Auswertung vergleicht die ganzzahligen Rohwerte, nicht ungenaue Fließkommazahlen. Manuelle Eingriffe überschreiben keine gespeicherten Profile.

Der Standard für „Heizkurven-Abfrageintervall HK1“ ist ab v0.5 **1 Minute**, zuvor 60 Minuten. Bereits in ESPHome gespeicherte Einstellungen des bestehenden Intervallreglers bleiben erhalten; beim Umstieg gegebenenfalls auf 1 Minute stellen. Veraltet sind die Werte nach dem größeren Wert aus `stale_after` und dem dreifachen Konfigurations-Abfrageintervall. Die letzte Lüfterstufenzuordnung und ihre 30-Sekunden-Gültigkeitsbewertung bleiben davon unabhängig.

## Schreiben, Speicherung und Fehlerfälle

Die Anwendung liest Block **05** frisch, verlangt den vollständigen 2.x-Block mit mindestens 22 Nutzdatenbytes und ersetzt ausschließlich P13/P14/P15 an Offset 0–4. Alle übrigen Bytes einschließlich HK2 und Min-/Max-Grenzen bleiben erhalten. Es folgt genau ein SET und ein unabhängiger GET. Erst wenn der gesamte zurückgelesene Nutzdatenblock übereinstimmt, wird die Anwendung bestätigt und ihre Momentaufnahme im ESP32 gespeichert.

Bei fehlender Schreibfreigabe, leerem Profil, ungespeichertem Entwurf, Speicherfehler oder laufendem Schreibvorgang wird die Anwendung abgewiesen. Eine unsichere SET-Antwort führt zum Rücklesen, niemals zur Wiederholung des SET. Rückleseabweichungen bestätigen das Profil nicht als angewendet. Die Erkennungsanzeige kann dennoch zu einem Profil passen, wenn P13/P14/P15 stimmen, aber ein anderes erhaltenes Byte abweicht; der getrennte Aktionsstatus bleibt dann fehlerhaft.

Die Profilspeicherung verwendet einen eigenen Preferences-Schlüssel. Uhrzeitmodus und F5-/F6-Fehlerhistorie behalten ihren bisherigen Schlüssel und ihr Format. Ein Speicherfehler wird nicht als erfolgreich gespeichert ausgegeben. Bei erfolgreichem LWZ-Rücklesen, aber anschließend fehlgeschlagener Speicherung des Anwendungsvermerks, wird dieser Unterschied ausdrücklich angezeigt. Neustart löst keinen erneuten Schreibauftrag aus.

## Umstieg von v0.4

- `enable_v03` und `enable_v04` aus der YAML entfernen. Die Optionen und internen Aktivierungsflags sind vollständig entfernt; es gibt keinen Ersatz-Aktivierungsschalter. Die Funktionen werden automatisch integriert. Alte YAML-Dateien mit diesen Optionen müssen angepasst werden; sie werden nicht stillschweigend umgedeutet.
- Der vorhandene Gerätename (`device_name` / ESPHome-Name) bleibt für die Speicherzuordnung maßgeblich; beim Umstieg den bisherigen Namen beibehalten.
- Die separate Schreibfreigabe bleibt erhalten. Native Anlagen-Slider und F5/F6 funktionieren weiter; die Kommunikationspausen betragen weiterhin jeweils 1 Sekunde.
- Beide regulären YAML-Dateien enthalten die neuen Bedienelemente, Gerätenamen und `refresh: 0s` bei der GitHub-Quelle. Solange v0.5 nicht auf GitHub veröffentlicht ist, die lokale Paketkonfiguration mit `source: components` verwenden.
- Das Paket enthält Quelltext und Dokumentation. Etwaige Prüfbinaries mit synthetischen Zugangsdaten werden nicht ausgeliefert oder auf die Anlage übertragen.

Die Softwareprüfungen und ihre Grenzen stehen im [Validierungsbericht v0.5](VALIDATION-v0.5.md). Physische Profil-Schreibversuche und die Optimierung der Vorschlagswerte sind noch nicht erfolgt.
