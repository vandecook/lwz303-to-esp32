# Heizkurve HK1 – P13, P14 und P15

Stand 08.11.2026. Drei nichtoptimistische Slider sind in der lokalen v0.4 ergänzt. Sie lesen beim Start die tatsächlichen Anlagenwerte; kein Herstellerwert oder Vorschlag wird beim Start geschrieben. Die separate Schreibfreigabe gilt auch für diese Regler und startet ausgeschaltet.

## Werte und Zuordnung

| Parameter | Herstellerstandard | Nutzer meldet aktuell | Sliderbereich | Schritt | Gruppe |
|---|---:|---:|---:|---:|---|
| P13 Gradient HK1 | 0,6 | 0,3 | 0–1 | 0,1 | Basis |
| P14 Fußpunktverschiebung HK1 | 0 K | 1,0 K | 0–5 K | 0,1 K | Basis |
| P15 Raumeinfluss Heizen HK1 | 0 | 0 (vom Nutzer bestätigt) | 0–10 | 0,1 | Expert |

Die Herstelleranleitung bestätigt die Standards und die größeren Einstellbereiche P13 0–5, P14 0–20, P15 0–10 auf Seite 70. Die Slider für P13/P14 sind auf Wunsch enger begrenzt. Eine tatsächlich ausgelesene Einstellung außerhalb der Slidergrenzen wird unverändert angezeigt, nicht stillschweigend gekürzt oder zurückgeschrieben. Die Fußpunktverschiebung wird als Temperaturdifferenz in K geführt; die Parametertabelle des Herstellers verwendet hierfür °C.

Die 0,1-Auflösung ist durch die FHEM-Decoder-/Schreibregeln des 2.x-Profils belegt, einschließlich P15 als ein Byte mit Skalierung /10. Ob die konkrete FW 2.16 jeden Zwischenwert übernimmt, muss die unabhängige Rückleseprüfung beim Gerät bestätigen. P15 hat hier keine Prozent-Einheit; die abweichende 4.x-Definition wird nicht übernommen.

## Vorschläge zur Erprobung

| Einstellung | Gradient P13 | Fußpunkt P14 | Raumeinfluss P15 | Einordnung |
|---|---:|---:|---:|---|
| Herstellerstandard | 0,6 | 0 K | 0 | Referenz, kein Optimierungsvorschlag |
| Aktuell laut Nutzer | 0,3 | 1,0 K | 0 | Ausgangswert; nicht automatisch überschreiben |
| **Vorschlag Übergangszeit** | **0,2** | **2,0 K** | **0** | Flachere Kurve mit höherem Fußpunkt; Versuch für milde Heiztage, wenn die bisherige Kurve nicht gleichmäßig passt |
| **Vorschlag sehr kalte Tage** | **0,4** | **1,0 K** | **0** | Erster Versuch bei etwa −10 °C und darunter, falls es mit 0,3/1,0 im Raum zu kalt wird |

Die beiden Vorschläge sind eigene Startpunkte relativ zur gemeldeten Einstellung, keine Herstellerprogramme und keine nachgewiesene optimale Auslegung. Außentemperaturen sind Orientierung, keine automatischen Umschaltgrenzen. Wenn das Haus mit 0,3/1,0 auch bei Kälte ausreichend warm wird, besteht kein Anlass, allein wegen Frost auf 0,4 zu erhöhen. Für Radiatoren oder einen anderen Wärmebedarf können andere Einstellungen nötig sein.

Die Anleitung erläutert auf Seiten 10–11: Gradient bestimmt die Reaktion auf Außentemperaturänderungen; Fußpunkt verschiebt die Kurve. Genau die derzeit gemeldete Kombination 0,3/1,0 wird dort auch als Beispiel für eine Fußbodenheizung beschrieben. Sie kann deshalb eine sinnvolle dauerhafte Einstellung sein. Eine HK1-Kurve ist abhängig vom Vorlaufanteil nicht automatisch identisch mit der Vorlauftemperaturkurve.

Zur Erprobung zunächst den bisherigen Zustand festhalten, einen Parameter ändern und bei vergleichbarem Wetter Raumtemperatur und Verlauf beobachten. Als eigener Praxisvorschlag bei träger Fußbodenheizung mindestens 24–48 Stunden je Änderung abwarten. P15 zunächst bei 0 belassen. Laut Hersteller setzt Raumeinfluss ein zusätzliches Bedienteil „FES Komfort“ im Wohnraum voraus; ein beliebiger HA-Raumsensor wird dadurch nicht automatisch als Regelungsfühler verwendet. Die aktuell unzuverlässige sHC1-Innentemperatur im Decoderprofil 2.16 ist kein Nachweis für einen funktionierenden Raumfühler.

## Schreibweg und Rückleseprüfung

| Parameter | FHEM-Feld | Datenblock | Payloadoffset / Format | Geschriebener Rohwert |
|---|---|---|---|---|
| P13 | `p13GradientHC1` | `pHeat1`, Befehl `05`, Decoder `05pxx206` | 0–1, uint16 Big Endian, /10 | Slider ×10 |
| P14 | `p14LowEndHC1` | gleich | 2–3, uint16 Big Endian, /10 | Slider ×10 |
| P15 | `p15RoomInfluenceHC1` | gleich | 4, uint8, /10 | Slider ×10 |

Jede Bedienung liest Block 05 frisch, ändert ausschließlich die betroffenen ein oder zwei Bytes und erhält alle übrigen Felder, insbesondere HK2 sowie Min-/Max-Grenzen. Schreibvoraussetzung ist ein mindestens 22 Byte vollständiger 2.x-Parameterblock. GET: Befehl `05`; SET: gleicher Befehl mit SET-Kennung `0x80` und erhaltenem, gezielt verändertem Nutzdatenblock. Kein Einsatz der 4.x-Befehle `0B010E`/`0B010F`.

Nach einem einzelnen SET folgt ein unabhängiger GET. Erst der tatsächliche Lesewert wird als Zustand veröffentlicht; der Schreibstatus ist nur dann erfolgreich, wenn Zielwert und sämtliche erhaltenen Bytes übereinstimmen. Bei unsicherer SET-Antwort keine SET-Wiederholung, stattdessen Rücklesen. Bei Abweichung Fehlerstatus; unvollständige Parameterblöcke und Werte außerhalb des Bereichs oder 0,1-Rasters werden verworfen. Eine laufende Schreibtransaktion blockiert weitere Schreibaufträge.

Die bisherigen Lesesensoren `hc1_gradient` und `hc1_low_end` werden durch Numbers mit denselben Konfigurationsschlüsseln und Namen ersetzt. Keine zusätzlichen Dubletten. In HA wechselt damit die Domäne von `sensor` zu `number`; bestehende Karten und Automationen können eine Anpassung benötigen. P15 kommt als Number `hc1_room_influence` hinzu.

Seit v0.5 sind benannte Heizkurvenprofile P13/P14/P15 mit dauerhafter Speicherung in ESPHome, ausschließlich manueller Anwendung und Profilerkennung umgesetzt. Fünf Speicherplätze, klare Trennung von Entwurf/Speichern/Anwenden, „Benutzerdefiniert“ bei abweichender tatsächlicher Kombination. Einzelheiten, Grenzen und Umstieg: [Heizkurvenprofile v0.5](HEIZKURVENPROFILE-v0.5.md).


## Quellen und Verifikation

- [STIEBEL-ELTRON-Anleitung LWZ 303/403, Seiten 10–11 und 70](https://www.stiebel-eltron.com/static/ste/docportal/manual/DM0000017729-nsm.pdf): Standardwerte, Herstellerbereiche, Heizkurvenwirkung, Raumfühler-Voraussetzung. Original im lokalen Dokumentationspaket enthalten.
- [FHEM 00_THZ.pm, gepinnte Revision](https://github.com/mhop/fhem-mirror/blob/0178ba5567d8b203db51215d29406454f1c7d001/fhem/FHEM/00_THZ.pm): `05pxx206`, `p13GradientHC1`, `p14LowEndHC1`, `p15RoomInfluenceHC1`, `pHeat1` und Lesen-Ändern-Schreiben in `THZ_Set`. [Lokale Referenz](references/fhem/00_THZ.pm).
- Aktuelle Werte P13/P14/P15 = 0,3/1,0 K/0: Nutzerangabe vom 08.11.2026, keine Live-Abfrage.
- Software: Hosttests für alle vier Profile, Grenzen/Raster einschließlich P15 10→100, Erhalt der übrigen Bytes, unsichere SET-Antwort, Rückleseabweichung, unvollständige Antwort und Zurücksetzen der Anzeige. ESPHome-Konfiguration und Codegenerierung werden mit synthetischen Zugangsdaten geprüft. Physische Schreibprüfung und Wirksamkeitsprüfung der Vorschläge offen; es wurde nichts an der Wärmepumpe geändert.
