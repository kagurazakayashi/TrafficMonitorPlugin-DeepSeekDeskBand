# DeepSeek Egyenleg Asszisztens (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) asztali kiegészítő modul. Valós időben jeleníti meg a DeepSeek hivatalos webhelyének egyenlegadatait a tálca ablakában, támogatja az egyenlegváltozási előzményeket.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | Magyar | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | [Türkçe](README.tr.md)

## Funkciók

- DeepSeek hivatalos API egyenlegének időzített lekérdezése
- Az egyenleg értékének és pénznemének megjelenítése a TrafficMonitor tálcáján és főablakában
- Egyenlegváltozási előzmények (automatikus növekményes tárolás, állítható korláttal)
- Többnyelvű támogatás (rendszernyelv automatikus észlelése vagy kézi választás)
- A konfigurációs fájl (API kulccsal együtt) és az előzmények Windows DPAPI titkosítással vannak ellátva, így azok csak az aktuális Windows felhasználó által fejthetők vissza

## Telepítés

1. Töltsd le a megfelelő DLL-t a [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) oldalról:
   - x86 TrafficMonitor → `DeepSeekDeskBand.dll`
   - x64 TrafficMonitor → `DeepSeekDeskBand64.dll`
2. Másold a DLL-t a TrafficMonitor programkönyvtárában található `plugins` mappába
3. Indítsd újra a TrafficMonitor alkalmazást
4. Kattints a jobb egérgombbal a tálca üres területére, válaszd a „Megjelenítési beállítások” menüpontot, és jelöld be a „DeepSeek Egyenleg Asszisztens” elemet a megjelenítéshez

> **Figyelem**: A DLL fájlnak a `plugins` almappában kell lennie, más könyvtárban nem tölthető be.

## A megjelenített tartalom leírása

A modul által megjelenített tartalom két részből áll:

- **Bal oldali címkeszöveg**: Módosítható a TrafficMonitor „Beállítások” → „Tálca ablak beállításai” → „Megjelenített szöveg beállításai” menüpontjában.
- **Jobb oldali érték szövege**: Az `egyenleg értéke pénznem` formátumban jelenik meg, például `100.00 CNY`. Az egyenlegadatok időzítetten automatikusan frissülnek.

A TrafficMonitor jobb egérgombos menüjének „Megjelenítendő elemek” almenüjében a modul megjelenített neve a nyelvi beállítás szerint változik (például magyar környezetben „DeepSeek Egyenleg Asszisztens”).

## A modul konfigurációs ablakának megnyitása

Kattints a jobb egérgombbal a TrafficMonitor tálca ablakára vagy főablakára, válaszd a „Bővítménykezelés” menüpontot a jobb egérgombos menüben, majd kattints a jobb egérgombbal erre a modulra, és válaszd a „Beállítások” lehetőséget a konfigurációs párbeszédpanel megnyitásához. A konfigurációs párbeszédpanelen beállíthatod az API kulcsot és egyéb opciókat, valamint megtekintheted az előzményeket.

## Beállítási lehetőségek leírása

### DeepSeek API kulcs

- Add meg a [DeepSeek API kulcsodat](https://platform.deepseek.com/api_keys) a beviteli mezőben
- A kulcs jelszavas maszkolással jelenik meg (nem látható nyílt szövegként)
- A beviteli mező alatt valós időben történik a formátum ellenőrzése: `sk-` előtaggal kell kezdődnie, amelyet 32 karakter hosszú betűkből és számjegyekből álló karakterlánc követ
- Helyes formátum esetén kattints az „API tesztelése” gombra a kulcs érvényességének ellenőrzéséhez
- Sikeres teszt után egy egyenleg részleteit tartalmazó párbeszédpanel jelenik meg, és az OK gomb elérhetővé válik

> **Az API teszt sikeres végrehajtása kötelező**, az OK gombra kattintva menthető. Ha a kulcs üres (nem szükséges megjeleníteni), közvetlenül menthető.

### Frissítési időköz (másodperc)

- Alapértelmezett érték: **60** másodperc
- Tartomány: 1 ~ 31536000
- A beállított másodpercenként automatikusan lekérdezi a DeepSeek API egyenleget
- A frissítési időköznek nagyobbnak kell lennie a kérés időtúllépési idejénél, különben nem menthető

### Kérés időtúllépése (másodperc)

- Alapértelmezett érték: **10** másodperc
- Tartomány: 3 ~ 60 másodperc
- Az API kérés maximális várakozási ideje, időtúllépés esetén a kérés sikertelennek minősül
- Javasolt a hálózati körülményekhez igazítani, rosszabb hálózat esetén megfelelően növelhető

### Előzmények száma

- Alapértelmezett érték: **1000** bejegyzés
- Tartomány: 0 ~ 10000 (0 érték esetén nem rögzít előzményeket)
- Csak akkor kerül új bejegyzés rögzítésre, ha tényleges egyenlegváltozás történt (különbség > 0,001), elkerülve a redundanciát
- A korlátot meghaladó régi bejegyzések automatikusan törlésre kerülnek

### Automatikus frissítés

- Alapértelmezetten be van jelölve
- Bejelölve az előzmények listája másodpercenként automatikusan frissül
- A lista bármely pontjára kattintva az automatikus frissítés megszakítható

### Megjelenítési nyelv

- Alapértelmezett érték: „Automatikus” (a rendszernyelv alapján automatikusan észleli)
- 12 nyelvet támogat: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- Nyelvváltás után a párbeszédpanel azonnal alkalmazza a változást, de a tartós mentéshez az „OK” gombra kell kattintani

### Előzmények törlése

- Az „Előzmények törlése” gombra kattintva és a művelet megerősítése után az összes előzmény törlődik
- A törlési művelet nem vonható vissza

## Fordítás (Build)

### Környezeti követelmények

- Visual Studio 2026 (v145 eszközkészlet)
- Windows 10 SDK vagy újabb verzió
- A `/utf-8` fordítási kapcsolónak engedélyezve kell lennie (a kínai nyelvű forráskódhoz szükséges)

### Gyors fordítás (ajánlott)

```batch
build.bat
```

Automatikusan lefordítja a Release konfigurációt az aktuális rendszerarchitektúrának megfelelően, és a generált DLL-t a `C:\TrafficMonitor\plugins\` mappába másolja.

> **Figyelem**: Módosítsd a `build.bat` fájlban található `PLUGINDIR` változót a TrafficMonitor tényleges telepítési útvonalának megfelelően.

### Kézi fordítás

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Kimeneti fájlok

| Platform | Konfiguráció | Kimeneti fájl                        |
|----------|-------------|--------------------------------------|
| x86      | Debug       | `Debug\DeepSeekDeskBand.dll`         |
| x86      | Release     | `Release\DeepSeekDeskBand.dll`       |
| x64      | Debug       | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64      | Release     | `x64\Release\DeepSeekDeskBand64.dll` |

## Adatvédelem és adatbiztonság

Ez a modul kiemelt figyelmet fordít a felhasználói adatvédelemre és adatbiztonságra:

- **API kulcs**: Windows DPAPI titkosítással kerül tárolásra a `DeepSeekDeskBand.dat` helyi konfigurációs fájlban, és csak az aktuális Windows felhasználó férhet hozzá és fejtheti vissza. A kulcs soha nem kerül feltöltésre harmadik félhez.
- **Egyenlegadatok**: Kizárólag HTTPS-en keresztül kerülnek lekérésre a DeepSeek hivatalos API-jából (`api.deepseek.com`), semmilyen köztes szerveren nem haladnak át.
- **Előzmények**: Az egyenlegváltozási rekordok DPAPI titkosítással kerülnek tárolásra a `DeepSeekDeskBand_History.dat` helyi fájlban, és csak az aktuális Windows felhasználó férhet hozzájuk.
- **Hálózati kérések**: Kizárólag HTTPS kérések kerülnek küldésre az `api.deepseek.com` címre (Authorization: Bearer Token), más domainekhez nem történik kérés.
- **Nincs adatgyűjtés**: Ez a modul semmilyen felhasználói adatot vagy használati statisztikát nem gyűjt és nem tölt fel.
- **Ikon gyorsítótár**: Csak a beállítási párbeszédpanel első megnyitásakor tölt le egyszer egy ikont (favicon.ico) a `www.deepseek.com` webhelyről, amelyet a modul konfigurációs könyvtárában tárol gyorsítótárban.

## Műszaki megjegyzések

- Modul felület: TrafficMonitor API v7
- Nincs MFC függőség, tisztán Win32 C++ megvalósítás
- Singleton minta a modulpéldány kezeléséhez
- WinHTTP HTTPS kliens
- Egyenlegváltozás észlelési küszöbérték > 0,001, a redundáns bejegyzések csökkentése érdekében

## Licenc

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Ez a projekt a Mulan PSL v2 licenc alapján kerül nyílt forráskódú közzétételre. Használhatod ezt a szoftvert a Mulan PSL v2 feltételeinek betartásával. A licenc másolata itt található: http://license.coscl.org.cn/MulanPSL2

Ez a szoftver „jelenlegi állapotában” kerül biztosításra, bármilyen kifejezett vagy hallgatólagos garancia nélkül, beleértve, de nem kizárólagosan a forgalomképességre, az adott célra való alkalmasságra és a jogsértésmentességre vonatkozó garanciákat.
