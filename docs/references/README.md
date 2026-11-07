# Referenzunterlagen im Paket

Stand: 07.11.2026. Alle beigefügten Originaldateien sind unverändert kopiert. Herkunft, Revision, Dateigröße und SHA-256-Prüfsumme stehen in [SOURCES.json](SOURCES.json).

**Urheberrecht:** Quelltexte und Dokumentation unterliegen dem Urheberrecht ihrer jeweiligen Ersteller bzw. Rechteinhaber. Die beigefügte FHEM-Datei und die STIEBEL-ELTRON-Handbücher bleiben den ursprünglichen Rechteinhabern zugeordnet. Ihre eigenen Lizenz- und Nutzungshinweise gelten unverändert; die MIT-Lizenz des Projekts wird nicht auf diese Fremdunterlagen übertragen. Originale Autoren-, Urheberrechts- und Lizenzhinweise sind beizubehalten.

**Lokale Herstellerunterlagen:** Die STIEBEL-ELTRON-Dateien liegen ausschließlich im eigenen Unterverzeichnis `docs/references/stiebel-eltron/`. Dieses Verzeichnis einschließlich etwaiger Seitenauszüge ist über `.gitignore` von Git ausgeschlossen. Die Original-PDFs werden weiterhin dem lokalen ZIP-Paket beigelegt. Nach einem Git-Checkout können die PDFs über die Herstellerlinks in [SOURCES.json](SOURCES.json) beschafft und dort unter den angegebenen Dateinamen abgelegt werden; die Paketierung prüft ihre SHA-256-Werte. FHEM-Referenz und Quellenverzeichnis bleiben außerhalb des ausgeschlossenen Verzeichnisses.

| Datei | Herkunft und Zweck |
|---|---|
| [FHEM 00_THZ.pm](fhem/00_THZ.pm) | Tatsächlich ausgewerteter Perl-Decoder, Version 0.210; Mirror-Commit `0178ba5567d8b203db51215d29406454f1c7d001`. Referenz, keine für die Integration benötigte Laufzeitkomponente. |
| [GNU GPL v2](fhem/GPL-2.0.txt) | Unveränderter vollständiger Lizenztext von GNU. Der Lizenzhinweis GPL v2 oder später im Kopf von 00_THZ.pm bleibt erhalten. |
| [Bedienung und Installation](stiebel-eltron/LWZ303-403-Bedienung-DM0000017729-nsm.pdf) | STIEBEL ELTRON A 166459-37161-8835, 76 Seiten. Heizstufentabelle S. 70/71; Warmwasser S. 47; F5/F6 S. 55. |
| [Gebrauchs- und Montageanweisung](stiebel-eltron/LWZ303-403-Montage-166459.pdf) | STIEBEL ELTRON 166459-34227-8160, 28 PDF-Seiten. Schaltpläne i und SOL auf S. 17/18. |

Die FHEM-Datei enthält auch den noch nicht korrigierten `F2type`-Verweis bei 2.14/j. Sie ist bewusst eine unveränderte Quellenkopie. Die zugehörigen Originalhinweise bleiben maßgeblich; aus dem Beilegen folgt keine Änderung der Projektlizenz für diese Fremdunterlagen.

Die PDFs sind vollständige Herstellerdokumente, keine neu erstellten Anleitungen und kein Beleg für identische Register jeder Firmware. Die vom Nutzer angeführte ältere „Bedienungsanweisung für den Fachmann“, Seite 20 unten, wurde nicht als konkrete Ausgabe identifiziert und ist deshalb nicht als eigene dritte Anleitung enthalten.

Projektbewertung: [Booster-Zuordnung](../BOOSTER-ZUORDNUNG.md). Geplante ESPHome-Fehlererkennung und Uhrbedienung: [Lüfterdiagnose v0.4](../LUEFTERDIAGNOSE-v0.4.md). Der aktuelle Paketstand enthält die v0.4-Software; Quellenbeleg und bestandene Softwareprüfung sind weiterhin von einer Hardwareprüfung zu unterscheiden.
