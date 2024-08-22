/**
 * Hilsfunktionen für Debug-Zwecke
 *
 * (c) 8/2024 heiko deserno
 */

#include <ArduinoJson.h>

/**
 * Funktion druckt den Inhalt der Konfiguration auf der Konsole.
 */
void debugConfig(JsonDocument config);

/**
 * Initialisierung des Filesystems und Auflistung aller Dateien
 */
void initFs();

/**
 * Liest die Empfänger-Konfiguration aus dem Filesystem.
 */
void readConfigurationFromFs();

/**
 * erstellt eine Welcomenachricht für die Konsole mit wichtigen
 * Konfigurationsdaten.
 */
String buildConnectMessage();
