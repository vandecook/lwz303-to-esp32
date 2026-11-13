# Validierung v0.6.1 - Webserver-Hilfe

Stand: 13.11.2026. Ergänzung zur Ausgabe 0.6.0; LWZ-Abfrage-, Schreib-, Diagnose- und Speicherlogik unverändert. Lediglich die Komponenten-Startmeldung nennt die neue Version. Die frühere Ausgabe 0.6.0 und ihre Prüfsummen bleiben unverändert erhalten.

## Prüfung der Weboberfläche

- Beide Hauptkonfigurationen mit ESPHome 2026.10.2, lokaler Komponentenquelle, synthetischen Zugangsdaten und Decoderprofil 2.16 erfolgreich validiert und generiert. Auswahl-/Konfigurationstests erfolgreich.
- Syntaxprüfung von `web/lwz-help.js` und der zusammengeführten Weboberfläche erfolgreich.
- Die ursprüngliche v3-Weboberfläche aus dem installierten ESPHome wurde aus dessen komprimierter HTML-Ressource übernommen und mit der statischen Hilfe verbunden. Quelle und Prüfsummen stehen in `web/SOURCES.json`; die MIT-Lizenz des ursprünglichen Webservers liegt bei. Keine CDN-Abfrage für die Oberfläche erforderlich.
- Lokale Browser-Vorschau mit dem tatsächlichen v3-Frontend und simuliertem ESPHome-Ping: Hilfe zunächst geschlossen; Aufklappen zeigt Erklärung von Ein/Aus, Datenalter und alle 13 Datenblock-Beschreibungen. Die elf Frischeanzeigen sind von den ergänzenden Abfragen 01/FD abgegrenzt. Darstellung visuell geprüft; keine Verbindung zur realen LWZ.
- Einbindung über den unterstützten `js_include`-Mechanismus, leeres `js_url`. `local: false` wählt den HTML-Rahmen mit dieser lokal eingebetteten Ressource. Die Kombination `local: true` plus `js_include` wurde nicht verwendet, weil die vorgefertigte lokale HTML-Seite den zusätzlichen Script-Verweis nicht enthält.
- Entitätsnamen, interne IDs, Sensorwerte und zusätzliche LWZ-Abfragen werden durch die statische Hilfe nicht verändert.
- Abgleich mit dem archivierten Paket 0.6.0: alle Komponentenquellen unverändert bis auf die Start-Versionsnummer; Konfigurationsbereiche für LWZ, Sensoren, Aktionen und UART strukturell identisch. Die generierte HTML-Seite verweist ausschließlich auf die lokale Ressource `/0.js`; deren entpackte Bytes stimmen mit der vorgesehenen kombinierten Weboberfläche überein.

## ESP32-C3-Build

Vollständiger ESP32-C3-Build mit ESPHome 2026.10.2 und ESP-IDF 5.5.6 erfolgreich, ohne Compilerwarnungen oder Fehler. Alle sechs Komponentenquellen stimmen bytegenau mit dem geprüften Quellstand überein. Die tatsächlich eingebettete, komprimierte Webressource wurde entpackt und bytegenau mit der vorgesehenen Oberfläche einschließlich Hilfe verglichen.

- RAM: **112205 / 321296 Byte**.
- Programm-Flash: **1081887 / 1835008 Byte**.
- Build: `config_hash=0xa0f98bc0 build_time_str=2026-11-13 12:35:28 +0100`.

Die SHA-256-Prüfsumme des bereits ausgelieferten Pakets 0.6.0 wurde erneut geprüft und ist unverändert. Prüfbinaries mit synthetischen Zugangsdaten werden nicht ausgeliefert oder geflasht.

## Auslieferung

Eigenes Quellpaket 0.6.1 mit lokalen Konfigurationen, Webressourcen, Herkunft/Lizenz, Änderungsverlauf und SHA-256-Prüfsummen. Keine Zugangsdaten oder Prüfbinaries im Paket. Keine Übertragung auf den ESP32 oder GitHub-Veröffentlichung ausgeführt. Die Hilfe erscheint am realen Gerät erst nach Kompilieren und Übertragen der neuen Konfiguration/Firmware.

Die in [v0.6.0](VALIDATION-v0.6.md) dokumentierten Prüfungen und Grenzen gelten für die unveränderte Betriebslogik weiter. Die vorläufige F2-Betriebszweckzuordnung und zusätzliche, noch nicht definierte Plausibilitätsgrenzen werden durch diese Oberflächenänderung nicht bestätigt.

## Nachbesserung 0.6.1 - Ressourcenpfad bei GitHub-Komponenten

Anlass: `js_include: web/...` verweist auf das lokale Konfigurationsverzeichnis, auch wenn die Datei im GitHub-Repository liegt. Nach ausdrücklichem Nutzerwunsch erfolgt die Korrektur ohne Versionserhöhung.

- Ressource, Lizenz und Herkunft direkt in `components/lwz2`; neue Option `web_help: true`. Der abschließende Konfigurationsschritt verwendet den tatsächlichen Komponentenpfad und bindet das Asset automatisch ein. Beide Hauptkonfigurationen enthalten keinen manuellen `js_include` mehr.
- Vier Regressionstests erfolgreich: Komponentenpfad, deaktivierte Hilfe ohne Webserver, fehlender/falscher Webserver und Konflikt mit eigener JavaScript-Datei.
- Isolierte Prüfung mit einer Kopie ausschließlich von `components/lwz2` in einem simulierten externen Checkout und getrenntem Konfigurationsordner: weder dort noch am Repository-Hauptpfad ist ein `web`-Ordner vorhanden. ESPHome-Codegenerierung erfolgreich; eingebettete JavaScript-Bytes stimmen vollständig mit der vorgesehenen Hilfe überein.
- Beide regulären Konfigurationen erneut erfolgreich generiert; Auswahl-/Konfigurationstests erfolgreich. Native C++-Quellen und die Weboberfläche bleiben gegenüber dem bereits vollständig gebauten 0.6.1-Stand unverändert. Für diese reine Änderung der Python-Pfadauflösung ist kein neuer vollständiger Firmware-Build erfolgt; der oben genannte Build dokumentiert die ursprüngliche 0.6.1-Oberfläche.
- Vorheriges ZIP unverändert unter `revisions/lwz303-v0.6.1-vor-github-webfix.zip` aufbewahrt. Das aktuelle Paket wird unter der bisherigen Versionsnummer neu gepackt und erhält eine neue SHA-256-Prüfsumme.
