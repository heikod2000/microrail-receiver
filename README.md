# Microrail

Heiko Deserno, 2024, [FB22](https://feldbahn22.de)

*Microrail*-Dokumentation: [microrail.hdecloud.de](https://microrail.hdecloud.de)

`Microrail` ist eine Software zur drahtlosen Steuerung von Modellfahrzeugen. Zum System gehören neben dem hier beschriebenen Empfänger ein Sender (Handregler). Der Sender als Bediengerät mit Drehregler und Display ist ein separates Softwareprojekt (derzeit noch nicht vorhanden). Die Software läuft auf den beschriebenen Hardware-Konfiguration, die aus einem Microcontroller und weiteren Komponenten bestehen. Die Hardware muss nach Dokumentation beschafft und zusammengebaut werden.

## Warum gibt es Microrail?

Da die wunderbaren [Deltang](http://www.deltang.co.uk/)-Empfänger *Rx6* nicht mehr hergestellt werden und kaum noch lieferbar sind, habe ich nach einer geeigneten Alternative zur drahtlosen Steuerung von Modellbahn-Fahrzeugen (insbesondere Feldbahnen im Maßstab 1:13) gesucht. [Locoremote](http://www.locoremote.co.uk/) ist auf den ersten Blick eine brauchbare Alternative. Den für mich größten Makel wollte ich jedoch nicht hinnehmen: Eine Modellfahrzeug sollte nicht mit einem Handy gesteuert werden. Hier gehört ein richtiger Handregler mit Tasten und einem Drehregler her. Da *Locoremote* diese Funktion nicht unterstützt, kam die Idee einer eigenen Software. Die Idee zur dieser Software stammt urspünglich von [Olle Sköld](http://depronized.com/), der eine solche Steuerung für seine [Hectorrail](https://www.thingiverse.com/thing:2575667)-Lokomotive vorstellte. `Microrail` nutzt eine andere Hardware-Plattform sowie ein modifiziertes Bedienkonzept. Auch die Idee des Handsenders stammt von ihm. Auf [Thingiverse](https://www.thingiverse.com/thing:3252986) stellte er einen Prototypen vor, der eher als Idee statt einer fertigen Lösung zu verstehen ist.

## Microrail-Receiver

Der `Microrail`-Receiver bzw. Empfänger ist die Komponente zur Motorensteuerung. Ein  Microcontroller und eine Motoren-Steuerplatine genügen für diese Funktion. Der Microcontroller erstellt einen eigenen WLAN-Accesspoint. WLAN-fähige Clients wie Mobiltelefone oder Tablets können sich mit dem WLAN des Microcontrollers verbinden und die UI über die Adresse http://192.168.1.4 aufrufen. Die UI enthält Buttons zur Steuerung von Geschwindigkeit und Fahrtrichtung.

### Hardware

Als Microcontroller wird ein [Lolin D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) verwendet. Als Motorentreiber unterstützt die Software folgende Platinen:

- [Lolin Motorshield HR8833](https://www.wemos.cc/en/latest/d1_mini_shield/hr8833_motor.html), HR8833 Chipsatz
- [Lolin Motorshield AT8870](https://www.wemos.cc/en/latest/d1_mini_shield/at8870_motor.html), AT8870 Chipsatz

Alle diese Platinen werden über den *I2C*-Bus mit dem *D1 mini* verbunden. Die Software-Steuerung basiert auf der [Lolin-Treiberlib](https://github.com/wemos/LOLIN_I2C_MOTOR_Library).

Die Platinen mit den *HR8833* und *AT8870* Chipsätzen sind seit 2022 neu und haben verschiedene Einsatzzwecke:

- *HR8833*: 3 bis 10 V, 1,5 A Motorstrom
- *AT8870*: 6,5 bis 38 V, 2 A Motorstrom

## Weitere Dokumentationen

> [!TIP]
> Mehr Informationen zur Hard- und Software sowie zur Entwicklungsumgebung sind auf der Seite [microrail.hdecloud.de](https://microrail.hdecloud.de) vorhanden.
