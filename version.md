# Version-History

## Version 1.0.1

- Textänderung Setup: alt `Motor-Trägheit` neu `Regler Verzögerung`
- Setup: Eingabeprüfung der nummerischen Parameter. Bei erfolgloser Prüfung werden die Default-Werte verwendet.

## Version 1.0.0

- **neue Hardwareversion!!!*
- Der Widerstand am Analogen Eingang `A0` erhöht sich auf 1M. Damit wird eine Spannungsmessung bis 13,2V möglich.

## Version 0.9.2

- Websocket-Commands vom Client zum Server verkürzt (`#SL` statt `#SLOWER` usw.)
- Die Berechnung und Anzeige der Batteriekapazität wurde entfernt
- neues Websocket-Command #INFO, Übermittlung Name, SSID und Version an einen Client
- neue Einstellung: Trägheit/Inertia
- neue Einstellung: Tausch Motorpolung

## Version 0.9.1

- Umstellung von *Server-Sent Events* auf *Websockets*
- Code-Optimierungen zur Reduzierung Speicherverbrauch
- Code strukturiert, Logging verbessert

## Version 0.9.0

- Empfänger-Einstellungen können editiert werden:
  - Seite `/setup`
- Aktualisierung Libs
- JQuery Aufrufe durch plain JS ersetzt, JQUery-Lib entfernt
- Design umgestellt
