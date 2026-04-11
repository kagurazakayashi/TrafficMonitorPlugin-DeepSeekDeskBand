# Assistente DeepSeek (DeepSeekDeskBand)

![screenshot](screenshot.png)

Plugin per la barra delle applicazioni di [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor). Mostra il saldo DeepSeek in tempo reale nella finestra della barra delle applicazioni, con supporto per lo storico delle variazioni.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | Italiano | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## Funzionalità

- Interroga periodicamente l'API ufficiale DeepSeek per il saldo
- Mostra il saldo e la valuta nella barra delle applicazioni e nella finestra principale di TrafficMonitor
- Storico delle variazioni del saldo (archiviazione incrementale automatica, limite configurabile)
- Supporto multilingua (rilevamento automatico della lingua di sistema o selezione manuale)
- File di configurazione (inclusa la API Key) e storico crittografati con Windows DPAPI, accessibili solo all'utente Windows corrente

## Installazione

1. Scarica la DLL corrispondente dalla pagina [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases):
   - TrafficMonitor x86 → `DeepSeekDeskBand.dll`
   - TrafficMonitor x64 → `DeepSeekDeskBand64.dll`
2. Copia la DLL nella cartella `plugins` all'interno della directory di TrafficMonitor
3. Riavvia TrafficMonitor
4. Fai clic destro su un'area vuota della barra delle applicazioni, seleziona "Impostazioni di visualizzazione" e spunta "Assistente DeepSeek" per visualizzarlo

> **Nota**: La DLL deve essere posizionata nella sottocartella `plugins`; non verrà caricata se collocata in altre directory.

## Descrizione dei contenuti visualizzati

Il contenuto mostrato dal plugin è composto da due parti:

- **Etichetta sinistra**: modificabile in TrafficMonitor tramite "Opzioni" → "Impostazioni finestra barra applicazioni" → "Impostazioni testo visualizzato".
- **Valore destro**: mostra `Saldo Valuta`, ad esempio `100.00 CNY`. Il saldo viene aggiornato automaticamente a intervalli regolari.

Nel sottomenu "Elementi visualizzati" del menu contestuale di TrafficMonitor, il nome visualizzato del plugin cambia in base alla lingua (ad esempio "Assistente DeepSeek" in italiano).

## Aprire la finestra di configurazione del plugin

Fai clic destro sulla finestra della barra delle applicazioni o sulla finestra principale di TrafficMonitor, seleziona "Gestione plugin" dal menu contestuale, quindi fai clic destro su questo plugin e scegli "Opzioni" per aprire la finestra di configurazione. Qui puoi impostare la API Key e altre opzioni, oltre a visualizzare lo storico.

## Descrizione delle opzioni di configurazione

### Chiave API DeepSeek

- Inserisci la tua [Chiave API DeepSeek](https://platform.deepseek.com/api_keys) nel campo di testo
- La chiave viene visualizzata in modalità mascherata (password)
- Sotto il campo di input viene convalidato il formato in tempo reale: deve iniziare con `sk-`, seguito da 32 caratteri alfanumerici
- Dopo la convalida del formato, clicca su "Testa API" per verificare la validità della chiave
- Se il test ha successo, apparirà una finestra con i dettagli del saldo e il pulsante OK sarà abilitato

> **Il test API deve essere superato** prima di poter salvare con il pulsante OK. Se la chiave è vuota (nessuna visualizzazione desiderata), è possibile salvare direttamente.

### Intervallo di aggiornamento (secondi)

- Valore predefinito: **60** secondi
- Intervallo: 1 ~ 31536000
- Interroga l'API DeepSeek automaticamente ogni numero di secondi impostato
- L'intervallo di aggiornamento deve essere maggiore del timeout della richiesta, altrimenti non sarà possibile salvare

### Timeout richiesta (secondi)

- Valore predefinito: **10** secondi
- Intervallo: 3 ~ 60 secondi
- Tempo massimo di attesa per una richiesta API; dopo il timeout la richiesta è considerata fallita
- Si consiglia di regolarlo in base alle condizioni di rete; aumentarlo se la rete è instabile

### Numero di record storici

- Valore predefinito: **1000** record
- Intervallo: 0 ~ 10000 (0 significa nessuna registrazione storica)
- Un nuovo record viene aggiunto solo quando il saldo cambia effettivamente (differenza > 0,001), per evitare ridondanza
- I record più vecchi oltre il limite vengono eliminati automaticamente

### Aggiornamento automatico

- Attivo per impostazione predefinita
- Quando attivo, l'elenco dello storico si aggiorna automaticamente ogni secondo
- Cliccando in qualsiasi punto dell'elenco si disattiva l'aggiornamento automatico

### Lingua di visualizzazione

- Valore predefinito: "Auto" (rilevamento automatico in base alla lingua di sistema)
- Supporta 12 lingue: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- Il cambio di lingua ha effetto immediato nella finestra di dialogo, ma viene salvato in modo permanente solo cliccando su "OK"

### Cancella storico

- Cliccando su "Cancella storico" e confermando, tutti i record storici vengono eliminati
- L'operazione di cancellazione è irreversibile

## Compilazione

### Requisiti

- Visual Studio 2026 (set di strumenti v145)
- Windows 10 SDK o successivo
- Opzione di compilazione `/utf-8` obbligatoria (necessaria per il codice sorgente in cinese)

### Compilazione rapida (consigliata)

```batch
build.bat
```

Compila automaticamente la configurazione Release in base all'architettura di sistema corrente e copia la DLL generata in `C:\TrafficMonitor\plugins\`.

> **Nota**: Modifica la variabile `PLUGINDIR` in `build.bat` in base al percorso di installazione effettivo di TrafficMonitor.

### Compilazione manuale

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### File di output

| Piattaforma | Config. | File di output                        |
|-------------|---------|---------------------------------------|
| x86         | Debug   | `Debug\DeepSeekDeskBand.dll`          |
| x86         | Release | `Release\DeepSeekDeskBand.dll`        |
| x64         | Debug   | `x64\Debug\DeepSeekDeskBand64.dll`    |
| x64         | Release | `x64\Release\DeepSeekDeskBand64.dll`  |

## Privacy e sicurezza dei dati

Questo plugin attribuisce grande importanza alla privacy e alla sicurezza dei dati dell'utente:

- **Chiave API**: crittografata con Windows DPAPI e archiviata nel file di configurazione locale `DeepSeekDeskBand.dat`, accessibile e decifrabile solo dall'utente Windows corrente. La chiave non viene mai caricata su server di terze parti.
- **Dati del saldo**: ottenuti esclusivamente tramite HTTPS dall'API ufficiale DeepSeek (`api.deepseek.com`), senza passare attraverso server intermedi.
- **Storico**: i record delle variazioni del saldo sono crittografati con DPAPI e archiviati nel file locale `DeepSeekDeskBand_History.dat`, accessibile solo dall'utente Windows corrente.
- **Richieste di rete**: vengono inviate solo richieste HTTPS a `api.deepseek.com` (Authorization: Bearer Token), senza richieste ad altri domini.
- **Nessuna raccolta dati**: questo plugin non raccoglie né carica alcun dato utente o statistica di utilizzo.
- **Cache icone**: l'icona (favicon.ico) viene scaricata da `www.deepseek.com` solo al primo accesso alla finestra di configurazione e memorizzata nella cache della directory di configurazione del plugin.

## Note tecniche

- Interfaccia plugin: TrafficMonitor API v7
- Nessuna dipendenza da MFC, implementazione pura Win32 C++
- Gestione dell'istanza del plugin tramite pattern Singleton
- Client HTTPS basato su WinHTTP
- Soglia di rilevamento variazione saldo > 0,001, per ridurre i record ridondanti

## Licenza

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Questo progetto è distribuito sotto licenza Mulan PSL v2. Puoi utilizzare questo software nel rispetto dei termini di Mulan PSL v2. Una copia della licenza è disponibile all'indirizzo: http://license.coscl.org.cn/MulanPSL2

Questo software viene fornito "così com'è", senza alcuna garanzia esplicita o implicita, incluse, a titolo esemplificativo, le garanzie di commerciabilità, idoneità per uno scopo particolare e non violazione.
