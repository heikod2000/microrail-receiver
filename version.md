# Version-History

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
