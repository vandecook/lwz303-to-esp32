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
          <tr><th scope="row"><code>EE</code></th><td><strong>Programmzustand:</strong> aktueller Betriebsmodus und Programmzustände für Heizung, Warmwasser und Lüftung. Keine vollständigen Wochenzeitprogramme.</td></tr>
          <tr><th scope="row"><code>FC</code></th><td><strong>Geräteuhr:</strong> Uhrzeit, Datum und Wochentag der LWZ.</td></tr>
          <tr><th scope="row"><code>17</code></th><td><strong>Grundeinstellungen P01–P12:</strong> unter anderem Raum- und Warmwasser-Solltemperaturen sowie Lüfterstufen.</td></tr>
          <tr><th scope="row"><code>05</code></th><td><strong>Heizkurveneinstellungen:</strong> unter anderem Gradient P13, Fußpunkt P14 und Raumeinfluss P15.</td></tr>
          <tr><th scope="row"><code>F2</code></th><td><strong>Betriebsanforderungen:</strong> Heizung, Warmwasser und Hinweise zum Verdichter-Betriebszweck. Anforderungen sind kein Nachweis tatsächlicher Wärmelieferung.</td></tr>
          <tr><th scope="row"><code>F6</code></th><td><strong>Zeitweise Lüftungsübersteuerung:</strong> vorübergehende Lüfterstufe und verbleibende Dauer.</td></tr>
          <tr><th scope="row"><code>01</code></th><td><strong>Lüftereinstellungen:</strong> konfigurierte Lüfterwerte, abhängig vom Decoderprofil.</td></tr>
          <tr><th scope="row"><code>FD</code></th><td><strong>Firmwareabfrage:</strong> Versionsnummer der LWZ. Sie ändert nicht automatisch das Decoderprofil.</td></tr>
        </tbody>
      </table>
      <p>Die einzelnen Frische-Anzeigen gibt es für FB, F3, F4, E8, 09, EE, FC, 17,
      05, F2 und F6. 01 und FD sind ergänzende Abfragen.</p>
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
