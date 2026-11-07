# Home Assistant – Lüfterdiagnose v0.4

Die Fehlerentscheidung erfolgt lokal in ESPHome. Je Lüfter gibt es einen gespeicherten Problem-Binärsensor, einen Zustands- und Datenqualitätstext, einen Fehlerzähler sowie letzten Beginn und letztes Ende. Alle Entitätsnamen enthalten `device_name`.

## Benachrichtigung

Die Vorlage [lwz_fan_fault_notification.yaml](../ha/lwz_fan_fault_notification.yaml) unter `config/blueprints/automation/lwz/` im HA-System ablegen und daraus eine Automation anlegen. Die beiden lokalen F5-/F6-Problem-Entitäten der gewünschten Wärmepumpe auswählen. Die Vorlage erzeugt bei jedem neuen Übergang Aus → Ein eine dauerhafte HA-Mitteilung; sie startet die Wärmepumpe nicht neu.

Die Vorlage ist bewusst nicht an einen erfundenen Handy-Benachrichtigungsdienst gebunden. Eine Push-Aktion kann in der eigenen Automation ergänzt werden. Während HA nicht läuft, kann HA keine Meldung ausführen; gespeicherter Fehler und Fehleranzahl bleiben am ESP erhalten. Unbekannt → Ein bei Wiederverbindung löst keine doppelte Meldung aus. Ein vollständig während einer HA-Unterbrechung aufgetretener und zurückgesetzter Fehler bleibt über die Zählerhistorie erkennbar, wird mit dieser einfachen Vorlage aber nicht nachträglich gemeldet.

## Anschluss an die bestehende Neustartautomation

Nach dem tatsächlich erfolgten Wärmepumpenneustart die vom ESP bereitgestellte Taste **„Lüfterfehler nach LWZ-Neustart zurücksetzen“** über `button.press` aufrufen. Die konkrete Entität aus dem eigenen Gerät auswählen. Dadurch werden nur die gespeicherten lokalen F5-/F6-Fehler zurückgesetzt. Die Taste sendet keinen Befehl an die LWZ und schaltet weder Fingerbot noch Stromversorgung.

Das vorhandene Neustartverfahren bleibt in der eigenen HA-Automation. Ein Neustart des ESP, ein serieller Timeout oder ein bloßer Startversuch der HA-Automation gilt nicht als Nachweis eines Wärmepumpenneustarts. Die Komponente erfindet daraus keine automatische Neustarterkennung; ein belastbarer LWZ-Neustartzähler ist bisher nicht belegt. Deshalb ist die ausdrückliche Rückmeldung der eigenen HA-Automation der umgesetzte Weg.

Nach Rücksetzung sind neue frische Daten erforderlich. Besteht der Fehler weiter, wird nach einer neuen beobachteten 120-Sekunden-Nullphase erneut ein Fehler gespeichert und gemeldet. Zähler und bisherige Historie werden nicht gelöscht. „Letztes Fehlerende“ bezeichnet die Rücksetzung des gespeicherten Status, nicht den Beweis eines wieder laufenden Ventilators.

## Umstieg von v0.3

Entitäten sind jetzt explizit mit dem Gerätenamen benannt. Home Assistant verwaltet bestehende Entitätskennungen zusätzlich in seiner Registrierung; vorherige Automationen und Anzeigen vor dem Umstieg prüfen und nötigenfalls auf die neuen Entitäten umstellen. Die P07-/P08-Lesesensoren werden durch nichtoptimistische Number-Entitäten ersetzt und deshalb nicht doppelt angeboten. Die übrigen optionalen v0.3-Werte bleiben erhalten.

Die Konfiguration setzt den Geräte-Anzeigenamen auf `device_name`, passend zum expliziten Präfix. Eine tatsächliche HA-Registrierung und die Anzeige durch die jeweilige HA-Version müssen am System geprüft werden; die erzeugten ESPHome-Namen enthalten das Präfix genau einmal.
