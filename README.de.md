# DeepSeek-Guthabenassistent (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) DeskBand-Plugin. Zeigt den DeepSeek-Kontostand in Echtzeit im Taskleistenfenster an, mit Verlaufsprotokoll der Guthabenänderungen.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | Deutsch | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## Funktionen

- Regelmäßige Abfrage des DeepSeek-API-Kontostands
- Anzeige von Guthaben und Währung im Taskleisten- und Hauptfenster von TrafficMonitor
- Verlaufsprotokoll der Guthabenänderungen (automatische inkrementelle Speicherung, einstellbare Obergrenze)
- Mehrsprachig (automatische Erkennung der Systemsprache oder manuelle Auswahl)
- Konfigurationsdatei (inkl. API-Key) und Verlauf werden mit Windows DPAPI verschlüsselt – nur der aktuelle Windows-Benutzer kann sie entschlüsseln

## Installation

1. Laden Sie die passende DLL von [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) herunter:
   - TrafficMonitor x86 → `DeepSeekDeskBand.dll`
   - TrafficMonitor x64 → `DeepSeekDeskBand64.dll`
2. Kopieren Sie die DLL in den Ordner `plugins` im TrafficMonitor-Programmverzeichnis
3. Starten Sie TrafficMonitor neu
4. Klicken Sie mit der rechten Maustaste auf einen leeren Bereich der Taskleiste, wählen Sie „Anzeigeeinstellungen" und aktivieren Sie „DeepSeek-Guthabenassistent"

> **Hinweis**: Die DLL muss sich im Unterordner `plugins` befinden. In anderen Verzeichnissen kann sie nicht geladen werden.

## Angezeigte Inhalte

Die Plugin-Anzeige besteht aus zwei Teilen:

- **Linker Beschriftungstext**: Kann unter TrafficMonitor → „Optionen" → „Taskleistenfenster-Einstellungen" → „Anzeigetext-Einstellungen" geändert werden.
- **Rechter Wertetext**: Zeigt `Guthaben Währung` an, z. B. `100.00 CNY`. Der Kontostand wird automatisch in festen Intervallen aktualisiert.

Im Untermenü „Anzeigeelemente" des TrafficMonitor-Kontextmenüs ändert sich der Anzeigename des Plugins je nach Spracheinstellung (z. B. „DeepSeek-Guthabenassistent" in deutscher Umgebung).

## Plugin-Konfigurationsfenster öffnen

Klicken Sie mit der rechten Maustaste auf das Taskleisten- oder Hauptfenster von TrafficMonitor, wählen Sie im Kontextmenü „Plugin-Verwaltung", klicken Sie mit der rechten Maustaste auf dieses Plugin und wählen Sie „Optionen", um den Konfigurationsdialog zu öffnen. Dort können Sie den API-Key und weitere Einstellungen vornehmen sowie den Verlauf einsehen.

## Erläuterung der Einstellungen

### DeepSeek API-Key

- Geben Sie Ihren [DeepSeek API Key](https://platform.deepseek.com/api_keys) in das Eingabefeld ein
- Der Key wird maskiert dargestellt (kein Klartext)
- Unter dem Eingabefeld wird das Format in Echtzeit geprüft: Muss mit `sk-` beginnen, gefolgt von 32 alphanumerischen Zeichen
- Nach korrekter Formateingabe klicken Sie auf „API testen", um den Key zu validieren
- Bei erfolgreichem Test erscheint ein Dialog mit den Guthabendetails; die Schaltfläche „OK" wird aktiviert

> **Der API-Test muss bestanden werden**, bevor Sie mit „OK" speichern können. Wenn der Key leer ist (keine Anzeige gewünscht), können Sie direkt speichern.

### Aktualisierungsintervall (Sekunden)

- Standardwert: **60** Sekunden
- Bereich: 1 ~ 31536000
- Der DeepSeek-API-Kontostand wird alle eingestellten Sekunden automatisch abgefragt
- Das Aktualisierungsintervall muss größer als die Anforderungs-Timeout sein, sonst kann nicht gespeichert werden

### Anforderungs-Timeout (Sekunden)

- Standardwert: **10** Sekunden
- Bereich: 3 ~ 60 Sekunden
- Maximale Wartezeit für API-Anfragen; wird diese überschritten, gilt die Anfrage als fehlgeschlagen
- Passen Sie den Wert je nach Netzwerkbedingungen an – bei schlechter Verbindung kann er erhöht werden

### Anzahl der Verlaufseinträge

- Standardwert: **1000** Einträge
- Bereich: 0 ~ 10000 (0 bedeutet: keine Aufzeichnung)
- Ein neuer Eintrag wird nur bei tatsächlicher Guthabenänderung (Differenz > 0,001) angelegt, um Redundanz zu vermeiden
- Ältere Einträge, die die Obergrenze überschreiten, werden automatisch gelöscht

### Automatische Aktualisierung

- Standardmäßig aktiviert
- Bei Aktivierung wird die Verlaufsliste jede Sekunde automatisch aktualisiert
- Ein Klick auf eine beliebige Stelle der Liste deaktiviert die automatische Aktualisierung

### Anzeigesprache

- Standardwert: „Automatisch" (erkennt die Systemsprache automatisch)
- Unterstützt 12 Sprachen: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- Sprachwechsel werden im Dialog sofort wirksam, aber erst mit Klick auf „OK" dauerhaft gespeichert

### Verlauf löschen

- Klicken Sie auf „Verlauf löschen" und bestätigen Sie, um alle Verlaufseinträge zu entfernen
- Dieser Vorgang kann nicht rückgängig gemacht werden

## Kompilieren

### Voraussetzungen

- Visual Studio 2026 (v145-Toolset)
- Windows 10 SDK oder höher
- Die Compileroption `/utf-8` muss aktiviert sein (erforderlich für chinesischen Quellcode)

### Schnellkompilierung (empfohlen)

```batch
build.bat
```

Kompiliert automatisch die Release-Konfiguration für die aktuelle Systemarchitektur und kopiert die erzeugte DLL nach `C:\TrafficMonitor\plugins\`.

> **Hinweis**: Passen Sie die Variable `PLUGINDIR` in `build.bat` an das tatsächliche Installationsverzeichnis von TrafficMonitor an.

### Manuelle Kompilierung

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Ausgabedateien

| Plattform | Konfiguration | Ausgabedatei                         |
|-----------|---------------|--------------------------------------|
| x86       | Debug         | `Debug\DeepSeekDeskBand.dll`         |
| x86       | Release       | `Release\DeepSeekDeskBand.dll`       |
| x64       | Debug         | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64       | Release       | `x64\Release\DeepSeekDeskBand64.dll` |

## Datenschutz und Datensicherheit

Dieses Plugin legt großen Wert auf Datenschutz und Datensicherheit:

- **API-Key**: Wird mit Windows DPAPI verschlüsselt in der lokalen Konfigurationsdatei `DeepSeekDeskBand.dat` gespeichert und ist nur für den aktuellen Windows-Benutzer zugänglich und entschlüsselbar. Der Key wird niemals an Dritte übermittelt.
- **Kontostanddaten**: Werden ausschließlich per HTTPS von der offiziellen DeepSeek-API (`api.deepseek.com`) abgerufen und durchlaufen keine Zwischenserver.
- **Verlauf**: Guthabenänderungen werden mit DPAPI verschlüsselt in der lokalen Datei `DeepSeekDeskBand_History.dat` gespeichert und sind nur für den aktuellen Windows-Benutzer zugänglich.
- **Netzwerkanfragen**: Es werden ausschließlich HTTPS-Anfragen an `api.deepseek.com` gesendet (Authorization: Bearer Token). Keine Anfragen an andere Domains.
- **Keine Datenerfassung**: Dieses Plugin sammelt oder übermittelt keinerlei Nutzerdaten oder Nutzungsstatistiken.
- **Icon-Cache**: Das Icon (favicon.ico) wird nur einmalig beim ersten Öffnen des Einstellungsdialogs von `www.deepseek.com` heruntergeladen und im Plugin-Konfigurationsverzeichnis zwischengespeichert.

## Technische Hinweise

- Plugin-Schnittstelle: TrafficMonitor API v7
- Keine MFC-Abhängigkeit, reine Win32-C++-Implementierung
- Singleton-Pattern zur Verwaltung der Plugin-Instanz
- WinHTTP HTTPS-Client
- Schwellenwert für Guthabenänderungen > 0,001, um redundante Einträge zu vermeiden

## Lizenz

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Dieses Projekt ist unter der Mulan PSL v2 lizenziert. Sie dürfen die Software unter Einhaltung der Bedingungen der Mulan PSL v2 nutzen. Eine Kopie der Lizenz finden Sie unter: http://license.coscl.org.cn/MulanPSL2

Die Software wird ohne Mängelgewähr und ohne jegliche ausdrückliche oder stillschweigende Garantie bereitgestellt, einschließlich, aber nicht beschränkt auf die Garantie der Marktgängigkeit, der Eignung für einen bestimmten Zweck und der Nichtverletzung von Rechten Dritter.
