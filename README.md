# CFGEDIT für Atari ST

CFGEDIT ist ein kleiner TOS-Editor für Druckertreiber im
GST-CFG-Format von 1st Word Plus.

Damit lassen sich Druckertreiberdateien wie `SCHN_DEU.CFG`,
`PAN_INT.CFG` oder `PAN_DEU.CFG` direkt auf dem Atari ST
betrachten und bearbeiten.

## Funktionen

- GST-CFG-Dateien laden und speichern
- Alle Records mit Offset, Code, Länge und Hex-Daten anzeigen
- Funktionscode ändern
- Hex-Daten ändern
- Records einfügen und löschen
- Originalheader bleibt erhalten

## Dateiformat

Eine GST-CFG-Datei enthält:

- Einen `GST-CFG:`-Header
- Funktions-Records: Länge, Code, Daten
- Einen Trenner `00`
- Eine Zeichentabelle mit Records für Atari-Zeichencodes
- Ein abschließendes `00`-Byte

## Beispiel

Initialisierung des deutschen Zeichensatzes:

```text
Code: 21
Daten: 1B 40 1B 52 02
```

Bedeutung:

- `1B 40` = `ESC @`, Drucker zurücksetzen
- `1B 52 02` = `ESC R 2`, deutschen Zeichensatz wählen

## Bedienung

Nach dem Start wird eine CFG-Datei abgefragt, zum Beispiel:

```text
PAN_DEU.CFG
```

Danach stehen diese Befehle zur Verfügung:

| Taste | Funktion |
|---|---|
| `L` | Alle Records auflisten |
| `E` | Record bearbeiten |
| `I` | Neuen Record einfügen |
| `D` | Record löschen |
| `S` | Datei speichern |
| `Q` | Programm beenden |

## Kompilierung

### Mit Pure C auf dem Atari ST

1. Neues Pure-C-Projekt anlegen.
2. `src/cfgedit.c` hinzufügen.
3. Als TOS-Programm übersetzen und linken.
4. Das entstandene `CFGEDIT.PRG` starten.

### Mit Cross-Compiler

```bash
make
```

Voraussetzung ist ein installierter `m68k-atari-mint-gcc`.

## GitHub Actions

Das Repository baut bei jedem Push automatisch ein
`CFGEDIT.PRG` und stellt es als Artifact bereit.
