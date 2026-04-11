# DeepSeek Asistanı (DeepSeekDeskBand)

![screenshot](screenshot.png)

[TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) için bir görev çubuğu eklentisidir. DeepSeek resmi web sitesi bakiye bilgilerini görev çubuğu penceresinde gerçek zamanlı olarak görüntüler ve bakiye değişim geçmişini destekler.

[English](README.md) | [简体中文](README.zh-Hans.md) | [繁體中文](README.zh-Hant.md) | [日本語](README.ja.md) | [Deutsch](README.de.md) | [עברית](README.he.md) | [Magyar](README.hu.md) | [Italiano](README.it.md) | [Polski](README.pl.md) | [Português (Brasil)](README.pt-BR.md) | [Русский](README.ru.md) | Türkçe

## Özellikler

- DeepSeek resmi API'sinden düzenli aralıklarla bakiye sorgulama
- TrafficMonitor'ün görev çubuğu ve ana penceresinde bakiye değerini ve para birimini görüntüleme
- Bakiye değişim geçmişi (otomatik artımlı depolama, ayarlanabilir üst sınır)
- Çoklu dil desteği (sistem dilini otomatik algılama veya manuel seçim)
- Yapılandırma dosyası (API Anahtarı dahil) ve geçmiş kayıtları Windows DPAPI ile şifrelenir, yalnızca mevcut Windows kullanıcısı tarafından çözülebilir

## Kurulum

1. [Release](https://github.com/KagurazakaYashi/TrafficMonitorPlugin-DeepSeekDeskBand/releases) sayfasından uygun DLL sürümünü indirin:
   - x86 TrafficMonitor → `DeepSeekDeskBand.dll`
   - x64 TrafficMonitor → `DeepSeekDeskBand64.dll`
2. DLL dosyasını TrafficMonitor program dizini altındaki `plugins` klasörüne kopyalayın
3. TrafficMonitor'ü yeniden başlatın
4. Görev çubuğundaki boş bir alana sağ tıklayın, "Görüntü Ayarları"nı seçin ve "DeepSeek Asistanı"nı işaretleyin

> **Not**: DLL dosyası `plugins` alt klasörüne yerleştirilmelidir, başka bir dizinde yüklenemez.

## Görüntülenen İçerik Açıklaması

Eklentinin görüntülediği içerik iki bölümden oluşur:

- **Sol etiket metni**: TrafficMonitor'ün "Seçenekler" → "Görev Çubuğu Penceresi Ayarları" → "Görüntü Metni Ayarları" bölümünden değiştirilebilir.
- **Sağ değer metni**: `Bakiye Değeri Para Birimi` biçiminde görüntülenir, örneğin `100.00 CNY`. Bakiye verileri düzenli aralıklarla otomatik olarak güncellenir.

TrafficMonitor sağ tıklama menüsünün "Görüntü Öğeleri" alt menüsünde, eklentinin görünen adı dil ayarına göre değişir (örneğin Türkçe ortamda "DeepSeek Asistanı").

## Eklenti Yapılandırma Penceresini Açma

TrafficMonitor'ün görev çubuğu penceresine veya ana penceresine sağ tıklayın, sağ tıklama menüsünde "Eklenti Yönetimi"ni seçin, bu eklentiye sağ tıklayın ve "Seçenekler"i seçerek yapılandırma iletişim kutusunu açın. Yapılandırma iletişim kutusunda API Anahtarı gibi seçenekleri ayarlayabilir ve geçmiş kayıtlarını görüntüleyebilirsiniz.

## Yapılandırma Seçenekleri Açıklaması

### DeepSeek API Anahtarı

- [DeepSeek API Anahtarınızı](https://platform.deepseek.com/api_keys) giriş kutusuna yazın
- Anahtar şifre maskesi modunda görüntülenir (düz metin gösterilmez)
- Giriş kutusunun altında biçim gerçek zamanlı olarak doğrulanır: `sk-` ile başlamalı ve ardından 32 haneli harf ve rakam gelmelidir
- Biçim doğru olduğunda anahtarın geçerliliğini doğrulamak için "API'yi Test Et" düğmesine tıklayın
- Test başarılı olduğunda bakiye detaylarını gösteren bir bilgi kutusu açılır ve onay düğmesi etkinleşir

> **API testi başarılı olmalıdır**, ancak onay düğmesine tıklanarak kaydedilebilir. Anahtar boşsa (görüntülenmesi gerekmiyorsa), doğrudan kaydedilebilir.

### Güncelleme Aralığı (saniye)

- Varsayılan değer: **60** saniye
- Aralık: 1 ~ 31536000
- Ayarlanan saniyede bir DeepSeek API bakiyesi otomatik olarak sorgulanır
- Güncelleme aralığı istek zaman aşımı süresinden büyük olmalıdır, aksi takdirde kaydedilemez

### İstek Zaman Aşımı (saniye)

- Varsayılan değer: **10** saniye
- Aralık: 3 ~ 60 saniye
- API isteği için maksimum bekleme süresi, aşıldığında istek başarısız kabul edilir
- Ağ durumuna göre ayarlanması önerilir, ağ koşulları kötü olduğunda uygun şekilde artırılabilir

### Geçmiş Kayıt Sayısı

- Varsayılan değer: **1000** kayıt
- Aralık: 0 ~ 10000 (0 olarak ayarlanırsa geçmiş kaydedilmez)
- Yalnızca bakiye gerçekten değiştiğinde (fark > 0.001) yeni kayıt eklenir, gereksiz kayıtlar önlenir
- Üst sınırı aşan eski kayıtlar otomatik olarak temizlenir

### Otomatik Yenileme

- Varsayılan olarak işaretlidir
- Etkinleştirildiğinde geçmiş kayıt listesi her saniye otomatik olarak yenilenir
- Liste üzerinde herhangi bir yere tıklamak otomatik yenilemeyi iptal eder

### Görüntüleme Dili

- Varsayılan değer "Otomatik"tir (sistem diline göre otomatik olarak algılanır)
- 12 dili destekler: 简体中文, 繁體中文, 日本語, English, Deutsch, עברית, Magyar, Italiano, Polski, Português (Brasil), Русский, Türkçe
- Dil değiştikten sonra iletişim kutusu hemen etkilenir, ancak kalıcı olarak kaydetmek için "Onay" düğmesine tıklanmalıdır

### Geçmişi Temizle

- "Geçmişi Temizle" düğmesine tıklayıp onayladıktan sonra tüm geçmiş kayıtları silinir
- Temizleme işlemi geri alınamaz

## Derleme

### Gereksinimler

- Visual Studio 2026 (v145 araç seti)
- Windows 10 SDK veya üstü
- `/utf-8` derleme seçeneği etkinleştirilmelidir (Çince kaynak kodu için gereklidir)

### Hızlı Derleme (Önerilen)

```batch
build.bat
```

Geçerli sistem mimarisine göre Release yapılandırmasını otomatik olarak derler ve oluşturulan DLL'yi `C:\TrafficMonitor\plugins\` dizinine kopyalar.

> **Not**: Lütfen TrafficMonitor'ün gerçek kurulum yoluna göre `build.bat` dosyasındaki `PLUGINDIR` değişkenini değiştirin.

### Manuel Derleme

```batch
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86
MSBuild DeepSeekDeskBand.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Çıktı Dosyaları

| Platform | Yapılandırma | Çıktı Dosyası                        |
|----------|--------------|--------------------------------------|
| x86      | Debug        | `Debug\DeepSeekDeskBand.dll`         |
| x86      | Release      | `Release\DeepSeekDeskBand.dll`       |
| x64      | Debug        | `x64\Debug\DeepSeekDeskBand64.dll`   |
| x64      | Release      | `x64\Release\DeepSeekDeskBand64.dll` |

## Gizlilik ve Veri Güvenliği

Bu eklenti kullanıcı gizliliğine ve veri güvenliğine büyük önem verir:

- **API Anahtarı**: Yerel yapılandırma dosyası `DeepSeekDeskBand.dat` içinde Windows DPAPI ile şifrelenerek saklanır, yalnızca mevcut Windows kullanıcısı tarafından erişilebilir ve çözülebilir. Anahtar hiçbir zaman üçüncü taraflara yüklenmez.
- **Bakiye Verileri**: Yalnızca HTTPS üzerinden DeepSeek resmi API'sinden (`api.deepseek.com`) alınır, hiçbir ara sunucudan geçmez.
- **Geçmiş Kayıtları**: Bakiye değişiklik kayıtları yerel dosya `DeepSeekDeskBand_History.dat` içinde DPAPI ile şifrelenerek saklanır, yalnızca mevcut Windows kullanıcısı tarafından erişilebilir.
- **Ağ İstekleri**: Yalnızca `api.deepseek.com` adresine HTTPS istekleri (Authorization: Bearer Token) gönderilir, başka hiçbir alan adına istek yapılmaz.
- **Veri Toplama Yok**: Bu eklenti hiçbir kullanıcı verisi veya kullanım istatistiği toplamaz veya yüklemez.
- **Simge Önbelleği**: Yalnızca yapılandırma iletişim kutusu ilk kez açıldığında `www.deepseek.com` adresinden bir kez simge (favicon.ico) indirilir ve eklenti yapılandırma dizininde önbelleğe alınır.

## Teknik Notlar

- Eklenti arayüzü: TrafficMonitor API v7
- MFC bağımlılığı yok, saf Win32 C++ uygulaması
- Eklenti örneğini yönetmek için tekil (singleton) desen
- WinHTTP HTTPS istemcisi
- Bakiye değişiklik algılama eşiği > 0.001, gereksiz kayıtları azaltır

## Lisans

Copyright (c) 2026 KagurazakaYashi (KagurazakaMiyabi)

Bu proje Mulan PSL v2 lisansı altında açık kaynaklıdır. Bu yazılımı Mulan PSL v2 şartlarına uygun olarak kullanabilirsiniz. Lisans metnine şu adresten ulaşabilirsiniz: http://license.coscl.org.cn/MulanPSL2

Bu yazılım "olduğu gibi" sunulmaktadır, açık veya örtülü hiçbir garanti verilmez; satılabilirlik, belirli bir amaca uygunluk ve ihlal etmeme garantileri dahil ancak bunlarla sınırlı olmamak üzere.
