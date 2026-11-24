// Static, locally embedded help. No requests to the LWZ or changes to entity names.
(() => {
  const showHelp = () => {
    if (document.getElementById("lwz-data-block-help")) return;
    const help = document.createElement("details");
    help.id = "lwz-data-block-help";
    help.lang = "de";
    help.innerHTML = `
      <style>
        #lwz-data-block-help {
          box-sizing: border-box; max-width: 1120px; margin: 12px auto;
          padding: 14px 18px; border: 1px solid #8888; border-radius: 8px;
          font: 15px/1.5 system-ui, sans-serif; color: CanvasText; background: Canvas;
        }
        #lwz-data-block-help summary { cursor: pointer; font-weight: 600; }
        #lwz-data-block-help p { margin: 12px 0; }
        #lwz-data-block-help table { width: 100%; border-collapse: collapse; }
        #lwz-data-block-help th, #lwz-data-block-help td {
          padding: 8px; border-bottom: 1px solid #8885; text-align: left; vertical-align: top;
        }
        #lwz-data-block-help th:first-child { width: 3em; }
        #lwz-data-block-help code { font-size: inherit; }
      </style>
      <summary>Datenblöcke erklärt – was bedeutet „frisch“?</summary>
      <p>Die LWZ liefert ihre Werte in einzelnen Datengruppen, den Datenblöcken.
      Die Kennungen sind keine Fehlercodes.</p>
      <p><strong>„Datenblock XX frisch“: Ein</strong> bedeutet, dass die Datengruppe
      innerhalb der vorgesehenen Zeit gültig empfangen wurde.
      <strong>Aus</strong> bedeutet, dass noch keine gültigen Daten vorliegen oder
      die letzte Antwort zu lange zurückliegt. Die Frist hängt vom Abfrageintervall ab.</p>
      <p>Bei seriellen Kommunikationsproblemen bleiben die letzten gültigen Werte sichtbar.
      Sie sind dann nicht als aktuell bestätigt. Das allein bedeutet keinen Fehler der Wärmepumpe.
      „Frisch“ bestätigt den Empfang der Datengruppe, nicht die Unterstützung jedes einzelnen
      Feldes durch die gewählte Gerätefirmware.</p>
      <table>
        <thead><tr><th scope="col">Block</th><th scope="col">Inhalt</th></tr></thead>
        <tbody>
          <tr><th scope="row"><code>FB</code></th><td><strong>Allgemeiner Status:</strong> Temperaturen, Verdichter, Pumpen, Ventile, Zusatzheizungszustände und relative Lüfterdrehzahlen.</td></tr>
          <tr><th scope="row"><code>F3</code></th><td><strong>Warmwasser:</strong> aktueller Sollwert, Betriebszustand und weitere Warmwasserdaten.</td></tr>
          <tr><th scope="row"><code>F4</code></th><td><strong>Heizkreis 1:</strong> Heizkreiszustand, Raum-Sollwert und weitere Heizungswerte, abhängig vom Decoderprofil.</td></tr>
          <tr><th scope="row"><code>E8</code></th><td><strong>Lüfterdaten:</strong> Ist- und Sollwerte sowie Speed200-Analysewerte. Für Firmware 2.16 sind nicht alle Felder verfügbar.</td></tr>
          <tr><th scope="row"><code>09</code></th><td><strong>Betriebsstunden:</strong> Zähler 1 und 2 sowie Heizungs- und Warmwasserstunden. Kühlungsstunden werden bei der LWZ303 nicht angezeigt.</td></tr>
          <tr><th scope="row"><code>0B</code></th><td><strong>Wochenprogramm Heizung:</strong> gespeicherte Schaltzeiten, Wochentage und Aktivierung für Heizkreis 1 und optional Heizkreis 2.</td></tr>
          <tr><th scope="row"><code>0C</code></th><td><strong>Wochenprogramm Warmwasser:</strong> gespeicherte Schaltzeiten, Wochentage und Aktivierung für Warmwasser.</td></tr>
          <tr><th scope="row"><code>0D</code></th><td><strong>Wochenprogramm Lüftung:</strong> gespeicherte Schaltzeiten, Wochentage und Aktivierung für zwei Zeitfenster.</td></tr>
          <tr><th scope="row"><code>EE</code></th><td><strong>Programmzustand:</strong> aktueller Betriebsmodus und Programmzustände für Heizung, Warmwasser und Lüftung.</td></tr>
          <tr><th scope="row"><code>FC</code></th><td><strong>Geräteuhr:</strong> Uhrzeit, Datum und Wochentag der LWZ.</td></tr>
          <tr><th scope="row"><code>17</code></th><td><strong>Grundeinstellungen P01–P12:</strong> unter anderem Raum- und Warmwasser-Solltemperaturen sowie Lüfterstufen.</td></tr>
          <tr><th scope="row"><code>05</code></th><td><strong>Heizkurveneinstellungen:</strong> unter anderem Gradient P13, Fußpunkt P14 und Raumeinfluss P15.</td></tr>
          <tr><th scope="row"><code>F2</code></th><td><strong>Betriebsanforderungen:</strong> Heizung, Warmwasser, Verdichter-Betriebszweck und direkter Verdampfer-Abtauzustand. Abtau-Rohbyte 0x10 (dezimal 16) = aus, 0x30 (dezimal 48) = Abtauen; Zuordnung für FW 2.16 vorläufig. Anforderungen sind kein Nachweis tatsächlicher Wärmelieferung.</td></tr>
          <tr><th scope="row"><code>F6</code></th><td><strong>Zeitweise Lüftungsübersteuerung:</strong> vorübergehende Lüfterstufe und verbleibende Dauer.</td></tr>
          <tr><th scope="row"><code>01</code></th><td><strong>Lüftereinstellungen:</strong> konfigurierte Lüfterwerte, abhängig vom Decoderprofil.</td></tr>
          <tr><th scope="row"><code>FD</code></th><td><strong>Firmwareabfrage:</strong> Versionsnummer der LWZ. Sie ändert nicht automatisch das Decoderprofil.</td></tr>
          <tr><th scope="row"><code>03</code></th><td><strong>Abtau-Einstellungen:</strong> unter anderem Verdichter-Wiederanlaufzeit P47 und Hauptlüfter P48. Nur lesen.</td></tr>
          <tr><th scope="row"><code>06</code></th><td><strong>Heizungsparameter:</strong> Hysteresen, maximale Zusatzheizstufen P31, Sommerbetrieb P49 und Zusatzheizungs-Wartezeit P79.</td></tr>
          <tr><th scope="row"><code>07</code></th><td><strong>Warmwasserparameter:</strong> unter anderem Warmwasser-Hysterese P32 und weitere gespeicherte Einstellungen.</td></tr>
          <tr><th scope="row"><code>08</code></th><td><strong>Solarparameter:</strong> nur beim gewählten SOL-Modell abfragen und anzeigen.</td></tr>
          <tr><th scope="row"><code>0A</code></th><td><strong>Umwälzpumpe:</strong> Pumpenzyklen und Temperaturgrenzen P54-P58.</td></tr>
          <tr><th scope="row"><code>0E</code></th><td><strong>Wiederanlauf:</strong> gespeicherter Parameter P59. Nur lesen.</td></tr>
          <tr><th scope="row"><code>10</code></th><td><strong>Trockenheizprogramm:</strong> Parameter P70-P74. Nur lesen, kein Start durch diese Integration.</td></tr>
        </tbody>
      </table>
      <p>Die einzelnen Frische-Anzeigen gibt es für FB, F3, F4, E8, 09, EE, FC, 17,
      05, F2 und F6. Die Wochenprogramme haben eigene Frische-Anzeigen im gleichnamigen Bereich. 01 und FD sind ergänzende Abfragen.</p>
      <p><strong>Programmierte Parameter:</strong> Der eigene Bereich zeigt gespeicherte Einstellungen.
      Bestehende Entitätsnamen bleiben erhalten. Einstellfelder schreiben erst bei manueller Bedienung
      mit eingeschalteter Schreibfreigabe. Erfolg wird durch erneutes Lesen geprüft.
      Vorhandene reine Sensoren bleiben für bestehende Dashboards erhalten; neue Eingabefelder sind zusätzlich vorhanden.
      Die Anzeige „Programmierte Parameter gueltig“ bewertet die Datenfrische der unterstützten Parameter.
      Bei Kommunikationslücken bleiben die letzten Werte sichtbar. P86 ist für das 2.x-Protokoll noch nicht belegt und nicht einstellbar.</p>
    `;
    const app = document.querySelector("esp-app");
    document.body.insertBefore(help, app);
  };
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", showHelp, { once: true });
  } else {
    showHelp();
  }
})();
