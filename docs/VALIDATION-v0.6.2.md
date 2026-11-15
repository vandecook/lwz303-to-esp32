# Validierung v0.6.2

Stand: 15.11.2026. Zielhardware ESP32-C3; reale LWZ 303 SOL laut Nutzer Firmware 2.16. Keine Verbindung zum tatsächlichen Gerät, kein Flashen, kein GitHub-Publish und keine Änderungen an Home Assistant durch diese Erstellung.

## Ausgeführte Prüfungen

- Zwei Auswahl-/Konfigurationstests und vier Webhilfe-Regressionstests erfolgreich.
- Beide Hauptkonfigurationen mit ESPHome 2026.10.2, lokaler Komponentenquelle, synthetischen Zugangsdaten und Decoderprofil 2.16 erfolgreich validiert und generiert.
- Isolierter externer Komponenten-Checkout ohne `web`-Ordner im Konfigurations- oder Repository-Hauptverzeichnis erfolgreich generiert; eingebettete Webressource bytegenau mit der vorgesehenen Oberfläche verglichen.
- Neue synthetische C++-Tests prüfen das direkte F2-Feld am Offset 8, `0x10`/`0x30`, fehlende und ausdrücklich ausgeschaltete Verdichterquelle, unbekannte Rohwerte, Anforderungswidersprüche, fehlendes Byte, Datenalterung, Wiederverbindung, serielle Wiederinitialisierung, alle vier Decoderprofile und Millisekundenüberlauf. Es werden keine zusätzlichen Befehle gesendet.
- Die bestehenden C++-Tests für Transport, Prüfsummen, Wiederholungen, F5/F6, Heizkurvenprofile und unabhängige Schreib-Rückleseprüfung werden zusammen mit den neuen Tests ausgeführt. Alle Testgruppen erfolgreich. Der neue Test stellt seine simulierte Uhr nach Abschluss wieder her; die bestehenden zeitabhängigen Tests bleiben dadurch unabhängig.

## Vollständiger ESP32-C3-Build

Nachträgliche Anzeigevereinfachung innerhalb 0.6.2: Binärsensorname „Abtauen“, bekannter Textzustand „Ja“/„Nein“. Die C++-Tests einschließlich beider Textwerte und die Konfigurationsgenerierung wurden dafür erneut erfolgreich ausgeführt. Der unten dokumentierte vollständige Gerätebuild erfolgte vor dieser reinen Namens-/Textanpassung.

Vollständiger ESP32-C3-Build mit ESPHome 2026.10.2 und ESP-IDF 5.5.6 erfolgreich, ohne Compilerwarnungen oder Fehler im abschließenden Build. Die sechs kompilierten Komponentenquellen stimmen bytegenau mit dem ausgelieferten Quellstand überein. Alle sechs neuen Entitäten sind in der erzeugten Konfiguration registriert. Die eingebettete Weboberfläche wurde entpackt und bytegenau mit der vorgesehenen Komponentendatei abgeglichen.

- RAM: **112453 / 321296 Byte**.
- Programm-Flash: **1084387 / 1835008 Byte**.
- Build: `config_hash=0xf7895637 build_time_str=2026-11-15 22:54:07 +0100`.

Build mit synthetischen Zugangsdaten. Prüfbinaries werden nicht ausgeliefert, veröffentlicht oder geflasht. Das Quellpaket enthält lokale und GitHub-Konfigurationen, Komponenten-Webressourcen, Herkunft/Lizenz und Dateiprüfsummen. Die Bezeichnungen bereits vorhandener Entitäten in lwz303.yaml wurden mit dem vorigen 0.6.1-Paket verglichen und sind unverändert; hinzugefügt wurden ausschließlich sechs Abtau-Anzeigen.

## Grenzen

Die Tests bestätigen die Softwareauswertung synthetischer Telegramme, nicht die physische Bedeutung des Bytes an FW 2.16. Ein realer Abtauvorgang einschließlich Beginn, Ende und möglicher Übergangscodes muss noch beobachtet werden. Andere Codes bleiben unbekannt; aus einem unbekannten Code wird kein Aus-Zustand abgeleitet. Der Vergleich mit Anforderungen ist diagnostisch und löst keinen Eingriff aus.

Die Ursache serieller Kommunikationsprobleme wird durch diese Ergänzung nicht als behoben ausgewiesen. Gehaltene Werte sind keine frischen Beobachtungen. Für HA-Statistik ist die eigene Abtau-Gültigkeit mitzuführen. Keine automatische Wärmepumpensteuerung, keine neuen Schreibwege und keine Übernahme eines Display-Bits neuerer Firmwarefamilien.

[Bedienung und neue Entitäten](ABTAUEN-v0.6.2.md). Historischer Prüfstand: [0.6.1](VALIDATION-v0.6.1.md).
