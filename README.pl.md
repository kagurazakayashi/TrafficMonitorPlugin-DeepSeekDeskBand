# Asystent salda DeepSeek (DeepSeekDeskBand)

![screenshot](screenshot.png)

Wtyczka paska narzędzi pulpitu dla [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor). Wyświetla w czasie rzeczywistym informacje o saldzie konta DeepSeek w oknie paska zadań, z obsługą historii zmian salda.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | Polski | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## Funkcje

- Okresowe sprawdzanie salda przez API DeepSeek
- Wyświetlanie wartości salda i waluty w pasku zadań i głównym oknie TrafficMonitor
- Historia zmian salda (automatyczny przyrostowy zapis z konfigurowalnym limitem)
- Obsługa wielu języków (automatyczne wykrywanie języka systemu lub wybór ręczny)
- Plik konfiguracyjny (zawierający klucz API) i historia szyfrowane za pomocą Windows DPAPI, dostępne tylko dla bieżącego użytkownika Windows

## Instalacja

1. Pobierz odpowiednią wersję DLL z [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases):
   - TrafficMonitor x86 → `DeepSeekDeskBand.dll`
   - TrafficMonitor x64 → `DeepSeekDeskBand64.dll`
2. Skopiuj plik DLL do folderu `plugins` w katalogu programu TrafficMonitor
3. Uruchom ponownie TrafficMonitor
4. Kliknij prawym przyciskiem myszy na pustym obszarze paska zadań, wybierz «Ustawienia wyświetlania» i zaznacz «Asystent salda DeepSeek», aby go wyświetlić

> **Uwaga**: Plik DLL musi znajdować się w podfolderze `plugins`. Umieszczenie go w innym katalogu uniemożliwi załadowanie.

## Opis wyświetlanych informacji

Wyświetlane informacje składają się z dwóch części:

- **Tekst etykiety po lewej stronie**: Można go zmienić w TrafficMonitor w «Opcje» → «Ustawienia okna paska zadań» → «Ustawienia wyświetlanego tekstu».
- **Tekst wartości po prawej stronie**: Wyświetla `wartość_salda waluta`, np. `100.00 CNY`. Dane salda są automatycznie aktualizowane okresowo.

W podmenu «Wyświetlane elementy» menu kontekstowego TrafficMonitor nazwa wyświetlana wtyczki zmienia się w zależności od ustawień językowych (np. w środowisku angielskim «DeepSeek Balance Assistant»).

## Otwieranie okna konfiguracji wtyczki

Kliknij prawym przyciskiem myszy okno paska zadań lub główne okno TrafficMonitor, wybierz «Zarządzanie wtyczkami» z menu kontekstowego, następnie kliknij prawym przyciskiem myszy tę wtyczkę i wybierz «Opcje», aby otworzyć okno konfiguracji. W tym oknie możesz ustawić klucz API i inne opcje oraz przeglądać historię.

## Opis opcji konfiguracji

### Klucz API DeepSeek

- Wprowadź swój [klucz API DeepSeek](https://platform.deepseek.com/api_keys) w polu tekstowym
- Klucz jest wyświetlany w trybie maskowania (nie jest widoczny jako zwykły tekst)
- Pod polem tekstowym na bieżąco sprawdzany jest format: musi zaczynać się od `sk-`, po którym następują 32 znaki alfanumeryczne
- Po poprawnym sformatowaniu kliknij «Testuj API», aby zweryfikować ważność klucza
- Po pomyślnym teście pojawi się okno ze szczegółami salda, a przycisk OK stanie się dostępny

> **Klucz musi przejść test API**, aby można było zapisać ustawienia przyciskiem OK. Jeśli klucz jest pusty (nie chcesz go wyświetlać), można zapisać bezpośrednio.

### Interwał aktualizacji (sekundy)

- Wartość domyślna: **60** sekund
- Zakres: 1 ~ 31536000
- Saldo DeepSeek API jest sprawdzane automatycznie co ustawioną liczbę sekund
- Interwał aktualizacji musi być większy niż limit czasu żądania, w przeciwnym razie zapis nie będzie możliwy

### Limit czasu żądania (sekundy)

- Wartość domyślna: **10** sekund
- Zakres: 3 ~ 60 sekund
- Maksymalny czas oczekiwania na żądanie API. Po przekroczeniu limitu żądanie uznawane jest za nieudane
- Zaleca się dostosowanie do warunków sieciowych. Przy słabszym połączeniu można odpowiednio zwiększyć tę wartość

### Liczba wpisów historii

- Wartość domyślna: **1000** wpisów
- Zakres: 0 ~ 10000 (ustawienie 0 wyłącza zapisywanie historii)
- Nowy wpis jest dodawany tylko wtedy, gdy saldo faktycznie się zmieni (różnica > 0,001), aby uniknąć nadmiarowości
- Najstarsze wpisy przekraczające limit są automatycznie usuwane

### Automatyczne odświeżanie

- Domyślnie zaznaczone
- Gdy zaznaczone, lista historii jest automatycznie odświeżana co sekundę
- Kliknięcie w dowolnym miejscu listy anuluje automatyczne odświeżanie

### Język wyświetlania

- Wartość domyślna: «Automatyczny» (automatycznie wykrywany na podstawie języka systemu)
- Obsługuje 12 języków: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- Zmiana języka jest natychmiast widoczna w oknie dialogowym, ale zostaje trwale zapisana dopiero po kliknięciu «OK»

### Czyszczenie historii

- Kliknij przycisk «Wyczyść historię» i potwierdź, aby usunąć wszystkie wpisy historii
- Tej operacji nie można cofnąć

## Kompilacja

### Wymagania środowiskowe

- Visual Studio 2026 (zestaw narzędzi v145)
- Windows 10 SDK lub nowszy
- Należy włączyć opcję kompilacji `/utf-8` (wymagane dla plików źródłowych w języku chińskim)

### Szybka kompilacja (zalecana)

```batch
build.bat
```

Automatycznie kompiluje konfigurację Release dla bieżącej architektury systemu i kopiuje wynikowy plik DLL do `C:\TrafficMonitor\plugins\`.

> **Uwaga**: Zmodyfikuj zmienną `PLUGINDIR` w pliku `build.bat` zgodnie z rzeczywistą ścieżką instalacji TrafficMonitor.

### Kompilacja ręczna

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Pliki wynikowe

| Platforma | Konfiguracja | Plik wynikowy                        |
|-----------|--------------|--------------------------------------|
| x86       | Debug        | `Debug\DeepSeekDeskBand.dll`         |
| x86       | Release      | `Release\DeepSeekDeskBand.dll`       |
| x64       | Debug        | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64       | Release      | `x64\Release\DeepSeekDeskBand64.dll` |

## Prywatność i bezpieczeństwo danych

Ta wtyczka przywiązuje dużą wagę do prywatności użytkownika i bezpieczeństwa danych:

- **Klucz API**: Szyfrowany za pomocą Windows DPAPI i przechowywany lokalnie w pliku konfiguracyjnym `DeepSeekDeskBand.dat`. Dostęp i odszyfrowanie możliwe tylko dla bieżącego użytkownika Windows. Klucz nigdy nie jest przesyłany do żadnych podmiotów trzecich.
- **Dane salda**: Pobierane wyłącznie przez HTTPS z oficjalnego API DeepSeek (`api.deepseek.com`), bez pośrednictwa jakichkolwiek serwerów pośredniczących.
- **Historia**: Zapis zmian salda szyfrowany za pomocą DPAPI i przechowywany lokalnie w pliku `DeepSeekDeskBand_History.dat`. Dostęp tylko dla bieżącego użytkownika Windows.
- **Żądania sieciowe**: Wysyłane tylko do `api.deepseek.com` przez HTTPS (Authorization: Bearer Token). Nie są wysyłane żądania do innych domen.
- **Brak zbierania danych**: Ta wtyczka nie zbiera ani nie przesyła żadnych danych użytkownika ani statystyk użytkowania.
- **Pamięć podręczna ikon**: Ikona (favicon.ico) jest pobierana z `www.deepseek.com` tylko raz, przy pierwszym otwarciu okna ustawień, i przechowywana w katalogu konfiguracyjnym wtyczki.

## Szczegóły techniczne

- Interfejs wtyczki: TrafficMonitor API v7
- Brak zależności od MFC, czysta implementacja Win32 C++
- Wzorzec singleton do zarządzania instancją wtyczki
- Klient HTTPS oparty na WinHTTP
- Próg wykrywania zmiany salda > 0,001 w celu redukcji nadmiarowych wpisów

## Licencja

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Ten projekt jest udostępniany na licencji open source Mulan PSL v2. Możesz korzystać z tego oprogramowania zgodnie z warunkami licencji Mulan PSL v2. Kopia licencji jest dostępna pod adresem: http://license.coscl.org.cn/MulanPSL2

To oprogramowanie jest dostarczane «tak jak jest» (as is), bez żadnych wyraźnych ani dorozumianych gwarancji, w tym między innymi gwarancji przydatności handlowej, przydatności do określonego celu oraz nienaruszania praw.
