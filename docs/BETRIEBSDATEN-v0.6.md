# Betriebsdaten und Datenqualität - v0.6.1

Stand: 13.11.2026. Lokales Quellpaket; keine Änderung der Wärmepumpenparameter und keine automatische Regelung.

## Betriebsstunden

Die vier vorhandenen Werte aus `sHistory` (09) verwenden unverändert das FHEM-Layout `09his206`: `operatingHours1`, `operatingHours2`, `heatingHours`, `DHWhours`. Jeweils unsigned 16 Bit Big Endian, Divisor 1, ganze Stunden. ESPHome-Web: Expert; HA: standardmäßig deaktiviert, bei Bedarf dort aktivieren. Zähler 1/2 bleiben neutral benannt. `coolingHours` wird in den mitgelieferten Konfigurationen nicht angelegt. Die übrigen Bytes werden nicht verschoben oder anders zugeordnet.

## Anzeige und Frische

Bei ausbleibenden seriellen Antworten bleiben die letzten gültigen Messwerte, Statusbits, F2-Anforderungen und gelesenen Einstellwerte sichtbar. Es werden keine Ersatznullen veröffentlicht. Werte bleiben nur im laufenden ESP im Speicher; ein ESP-Neustart hat bis zum ersten gültigen Lesen unbekannte Werte. Ein Decoderwechsel verwirft die alte Interpretation; nicht unterstützte Felder bleiben unbekannt. Eine Wiederinitialisierung der seriellen Kommunikation erhält die Anzeige, verwirft aber die interne Gültigkeit.

`data_quality` zeigt die Quellen FB, F3, F4, E8, 09, EE, FC, 17, 05, F2 und F6 jeweils als fehlt/frisch/veraltet, mit Alter seit der letzten gelesenen Antwort in Sekunden. Die zugehörigen `block_FB_fresh` usw. sind einzeln als Binärsensoren verfügbar: EIN bedeutet frische gültige Quelle. Die Grenzen folgen den bisherigen Abfrageintervallen: maximal `stale_after` oder dreifaches geplantes Intervall; F2/F6 verwenden `diagnostic_max_age`. Die Diagnose aktualisiert sich sekündlich und beim Lesen. Das tatsächliche Alter kann sich durch serielle Übertragung und Abfragewarteschlange erhöhen.

Für HA-Auswertungen reicht ein gehaltener Verdichter-Ein/Aus-Wert nicht aus: Zusätzlich `block_FB_fresh` beachten und Übergänge über Datenlücken nicht als neue Starts zählen. Für Zweckzuordnung zusätzlich `compressor_purpose_valid` und `compressor_purpose_data_status` verwenden. Lücken nicht nachträglich als bekannte Laufzeit auffüllen. ESPHome-API-/WLAN-Ausfälle können das gesamte Gerät in HA weiterhin als nicht verfügbar markieren; dies ist von seriellen LWZ-Ausfällen zu unterscheiden.

F5/F6 prüft weiterhin ausschließlich frische Drehzahlen und Quellen für die wirksame Lüfterstufe. Eine Datenlücke unterbricht den 120-Sekunden-Nachweis, löscht aber keinen gespeicherten Fehler. Schreibaktionen lesen weiterhin den Block frisch und bestätigen erst nach unabhängigem Rücklesen; gehaltene Anzeigen bestätigen keinen Schreibauftrag.

## Vorläufiger Verdichter-Betriebszweck

FHEM `F2ctrl.heatRequest` und `heatRequest2` (erste zwei Nutzdatenbytes) werden als `heat_request` und `heat_request2` unverändert angezeigt. Die FHEM-Kommentare nennen 0=Warmwasser, 2=Heizung, 5=aus, 6=Verdampfer-Abtauen. Diese Zuordnung ist für die reale FW 2.16 noch am Gerät zu prüfen.

- Bei frischem Verdichterstatus AUS: Zweck Aus, gültig auch ohne F2.
- Bei frischem Verdichterstatus EIN und frischem F2: Nur übereinstimmende Anforderungscodes 0/2/6 ergeben Warmwasser/Heizung/Abtauen, ausdrücklich als vorläufig markiert.
- Widersprüchliche/unbekannte Codes oder Anforderung AUS bei tatsächlich eingeschaltetem Verdichter: nicht eindeutig zuordenbar, Zweck-Gültigkeit AUS.
- Bei veralteten Quellen bleibt die letzte Anzeige erhalten, Zweck-Gültigkeit ist AUS und der Datenstatus nennt die fehlende/veraltete Quelle.

Gleichzeitiger Heizungs-/Warmwasserbedarf wird für SOL aus den bekannten F2-Stufen getrennt angezeigt. Er ist eine Anforderung, kein Nachweis gleichzeitiger Wärmelieferung. Für i/unbekannte Modelle und unbekannte Codes wird keine SOL-Zuordnung erfunden. Ein Zweckwechsel bei weiterlaufendem Verdichter ist kein neuer Verdichterstart.

Quelle: [unveränderte FHEM-Referenz](references/fhem/00_THZ.pm), Definitionen `09his206` und `F2ctrl`.

## Umfang und Migration

Neue Gültigkeitsanzeigen und vorläufige Zweckanzeige ergänzen die vorhandenen Entitäten. Projektversion und Komponenten-Startmeldung sind 0.6.0. Der Profil-Editorstatus heißt jetzt „Gespeichertes Profil und Editorstatus“; bei vorhandenen HA-Entitäten die Zuordnung nach dem Update prüfen. Dauerhafte Heizkurvenprofile und Lüfterfehlerhistorie verwenden unveränderte Speicherschlüssel.

Vorhandene Protokoll-, Datenalterungs- und Lüfterprüfungen bleiben aktiv. Keine neuen, nicht vereinbarten Temperatur-/Sprung-/Stillstandsgrenzen und keine automatische Korrektur von Vorlauf/Rücklauf. Verdichterstartzähler und zusätzliche Heizstabstunden werden ohne belegte Quelle nicht ergänzt. Diese Punkte bleiben für spätere 0.6.x offen. HA-Statistik und Wochenprogramme gehören zu ihren getrennten Roadmap-Versionen.

Quellpaket enthält die regulären GitHub-Konfigurationen sowie `lwz303-local.yaml` (Nutzerkonfiguration) und `lwz303-sol-local.yaml` (Beispiel) mit lokaler Komponentenquelle. Bis zur Veröffentlichung für den neuen lokalen Stand diese lokale Konfiguration mit eigenen Zugangsdaten verwenden. Keine Prüfung am echten Gerät, kein Flashen oder GitHub-Publish durch diese Erstellung. [Prüfbericht](VALIDATION-v0.6.1.md).

## Webserver-Hilfe ab v0.6.1

Oben im Geräteweb steht der aufklappbare Bereich **„Datenblöcke erklärt – was bedeutet frisch?“**. Er erklärt Ein/Aus der Frische-Anzeigen, die elf überwachten Datenblöcke sowie 01 und FD. Die Hilfe ist statisch und erzeugt keine zusätzlichen LWZ-Abfragen. Entitätsnamen und Zuordnungen bleiben unverändert. Die ursprüngliche ESPHome-v3-Oberfläche aus ESPHome 2026.10.2 und die Hilfe werden gemeinsam über `js_include` lokal eingebettet; `js_url` bleibt leer, daher keine CDN-Abfrage. `local: false` wählt hier den ESPHome-HTML-Rahmen mit der eingebetteten JavaScript-Datei und bedeutet nicht, dass externe Dateien geladen werden. Quelldateien, Herkunft und MIT-Lizenz stehen unter `web/`.
