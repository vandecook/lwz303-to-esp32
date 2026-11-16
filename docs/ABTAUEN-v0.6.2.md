# Verdampfer-Abtauzustand - v0.6.2

Stand: 16.11.2026. Neue read-only Anzeigen; keine Änderung an LWZ-Parametern, keine zusätzlichen Abfragen. Die Abtau-Integration wurde am 16.11.2026 vom Nutzer an seiner LWZ303 mit Firmware 2.16 als funktionierend bestätigt. Grundlage der Zuordnung ist der FHEM-Kommentar.

## Datenquelle

FHEM definiert `F2ctrl.defrostEvaporator` an Hex-Zeichenposition 20 der internen Antwortzeichenfolge als `raw` mit zwei Hex-Zeichen. Das entspricht dem neunten Nutzdatenbyte, Offset 8. Das gesamte Byte wird ausgewertet:

| Rohbyte | Anzeige als Zahl | Auswertung |
|---|---|---|
| `0x10` | 16 | Kein Verdampfer-Abtauen, vorläufig |
| `0x30` | 48 | Verdampfer taut ab, vorläufig |
| Andere Werte | Unveränderter Rohwert | Unbekannter Abtaucode |

Es wird keine Bitmaske aus dem Unterschied dieser Werte abgeleitet. Insbesondere sind `0x20`, `0x31` oder `0xFF` nicht als bekannte Zustände freigegeben. Das `sDisplay.defrost`-Bit neuerer Firmwareprofile wird nicht auf 2.x übertragen. [Unveränderte FHEM-Referenz](references/fhem/00_THZ.pm), Definition `F2ctrl`.

## Neue Anzeigen in Webserver und Home Assistant

| Schlüssel | Zweck |
|---|---|
| `defrost_evaporator_active` | Abtauen; reiner binärer Zustand |
| `defrost_evaporator_valid` | Aktueller Abtaucode ist frisch, vorhanden und in der vorläufigen Tabelle bekannt |
| `defrost_evaporator_raw` | Ganzzahliger Rohwert 0–255; Hex-Schreibweise zusätzlich im Datenstatus |
| `defrost_evaporator_status` | Verständlicher Zustand oder ausdrücklicher Hinweis auf einen unbekannten Code |
| `defrost_evaporator_data_status` | Datenfrische, Hex-Rohwert und Hinweis auf vorläufige Zuordnung |
| `defrost_request_comparison` | Diagnostischer Vergleich mit `heatRequest` und `heatRequest2` |

Die eigene Abtauanzeige ist unabhängig vom Verdichterstatus. Auch ohne gültigen FB-Block oder bei ausgeschaltetem Verdichter kann ein frischer bekannter F2-Abtaucode angezeigt werden. Die bisherige Anzeige des Verdichter-Betriebszwecks bleibt unverändert und wird nicht automatisch durch das neue Feld ersetzt.

## Widersprüche und Datenlücken

- Übereinstimmende bekannte Anforderungscodes werden mit der direkten Anzeige verglichen: `heatRequest = 6` deutet auf Abtauanforderung hin; 0, 2 und 5 sind bekannte andere Anforderungen.
- Ein direkter Abtauzustand bei anderer Anforderung oder eine Abtauanforderung bei direktem Zustand Aus wird ausdrücklich als Widerspruch angezeigt. Unterschiedliche Anforderungsbytes werden ebenfalls kenntlich gemacht. Die direkte Anzeige bleibt bei frischem bekanntem Code unabhängig davon gültig. Es wird kein automatischer Eingriff oder Alarm ausgelöst.
- Ein unbekannter Abtaucode wird als unbekannt angezeigt. Der binäre Abtausensor hat dann keinen bestätigten Ein/Aus-Zustand; dies ist keine automatische Aus-Meldung. Der Rohwert bleibt sichtbar, die eigene Gültigkeit ist Aus.
- Bei Kommunikationslücken oder einer F2-Antwort ohne dieses Byte bleiben letzte Anzeigewerte erhalten, die eigene Gültigkeit ist Aus und der Datenstatus erklärt den Grund. Eine frische FB-Antwort macht fehlende F2-Daten nicht aktuell.
- Die Gültigkeitsgrenze folgt `diagnostic_max_age`, standardmäßig 30 Sekunden. Nach serieller Wiederinitialisierung ist eine neue gültige F2-Beobachtung erforderlich. Decoderwechsel verwerfen alte Interpretationen; ein ESP-Neustart beginnt ohne gespeicherten Abtauzustand.

Für HA-Auswertungen niemals allein aus einem gehaltenen Ein/Aus-Wert Abtaulaufzeit oder neue Ereignisse ableiten. Zusätzlich `defrost_evaporator_valid` beachten. Zähler, Laufzeiten, Trends und Benachrichtigungen bleiben dem getrennten HA-Versionsplan zugeordnet.

## Übernahme und Geräteprüfung

Vollständigen Ordner `components/lwz2` und die passende YAML der Version 0.6.2 übernehmen. Die Hilfe verwendet weiterhin `web_help: true`; kein eigener `/root/config/web`-Ordner und kein manueller `js_include` nötig. Bis zur GitHub-Veröffentlichung die mitgelieferte lokale Paketkonfiguration mit `source: components` verwenden.

Am Zielgerät einen tatsächlich auftretenden Abtauvorgang beobachten: Rohcode, direkte Anzeige, Gültigkeit, Anforderungsbytes und LWZ-Anzeige zeitgleich vergleichen, einschließlich Beginn und Ende sowie Phasen mit ausgeschaltetem Verdichter. Eine unbekannte oder widersprüchliche Beobachtung zuerst dokumentieren; keine automatische Änderung der Tabelle. Es wurde keine Abtauung erzwungen oder ausgelöst.

[Software- und Buildprüfung](VALIDATION-v0.6.2.md). Vorheriger Stand: [0.6.1](VALIDATION-v0.6.1.md).
