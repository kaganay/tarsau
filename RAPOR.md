# tarsau - Proje Raporu

**Ders:** Sistem Programlama
**Donem:** 2025-2026 Bahar
**Teslim Tarihi:** 24 Mayis 2026

**Proje Ekibi:**

| Ad Soyad | Ogrenci No |
|----------|------------|
| Kagan Aydogan | G231210375 |
| Kadir Duyan   | G231210371 |

**GitHub Deposu:** `https://github.com/kaganay/tarsau`

---

## 1. Projenin Tanimi

`tarsau`, `tar`, `rar` veya `zip` araclarina benzer sekilde calisan ancak
**sikistirma yapmayan** basit bir arsivleme programidir. Verilen birden
cok ASCII metin dosyasini tek bir `.sau` arsiv dosyasi haline getirir
ve daha sonra bu arsivi orijinal dosyalarina (orijinal izinleri ile
birlikte) geri acar.

Program Linux uzerinde C dilinde gelistirilmistir ve C11 standardi ile
`gcc -Wall -Wextra -Wpedantic` derleyici uyari bayraklari altinda
uyarisiz bicimde derlenmektedir.

### Kullanim Bicimi

| Komut | Ne yapar? |
|-------|-----------|
| `./tarsau -b f1 f2 ... [-o ad.sau]` | Verilen metin dosyalarini tek bir arsiv olarak birlestirir. |
| `./tarsau -a ad.sau [dizin]` | `.sau` arsivinin icindeki dosyalari belirtilen dizine acar. |

### Kisitlamalar (sartnamede istenildigi sekilde)

- Sadece **ASCII metin** dosyalari arsivlenebilir (karakter basina 1 bayt).
- En fazla **32** giris dosyasi.
- Toplam giris boyutu **200 MB**'i gecemez.
- Cikis dosyasi adi `-o` ile verilmezse varsayilan olarak `a.sau`.
- `-a`'dan sonra en fazla 2 parametre.
- Acilan dosyalar arsivlendiklerindeki izinleri korur.
- Tum hata durumlarinda program duzgun bicimde sonlanir, ani sekilde
  cokmez.

---

## 2. `.sau` Arsiv Formati

Arsiv dosyasi iki bolumden olusur.

### 2.1 Organizasyon (icerik) bolumu

```
[10 bayt: 1. bolumun ASCII boyutu (sifir doldurmali)]
|dosya1,izin_oktal,boyut|dosya2,izin_oktal,boyut|...|dosyaN,izin_oktal,boyut
```

- Ilk **10 bayt** ASCII formatinda 1. bolumun toplam boyutudur (bu 10
  bayt dahil).
- Devaminda her kayit `|` ile ayrilir.
- Bir kaydin ic alanlari `,` ile ayrilir: `dosya_adi,izin,boyut`.

### 2.2 Arsivlenmis dosyalar bolumu

- 1. bolumun bittigi yerden itibaren tum dosya icerikleri **ardisik**
  olarak ve ayraclasiz sekilde yerlestirilir. Ortaya cikan tek bir buyuk
  metin elde edilir; her dosyanin baslangic ofseti, ondan onceki tum
  dosyalarin boyutlarinin toplamiyla bulunur.

### 2.3 Gercek bir cikti

Asagida `make demo` ciktisinin ilk 200 baytindan kesilmis bir ornek var:

```
0000000054|t1.txt,640,191|t2.txt,600,147|t3.txt,664,41Merhaba, bu birinci ornek metin dosyasidir.
Sistem Programlama dersi icin tarsau projesi.
Iceride birkac satir ve sayilar var: 12345
Ozel karak...
```

Burada:

- `0000000054` -> 1. bolum 54 bayt uzunluktadir.
- Sonraki 44 bayt 3 kayit icerir.
- Pozisyon 54'ten itibaren `t1.txt` dosyasinin icerigi baslar.

---

## 3. Gelistirme Sureci

Proje tek bir kaynak dosyada (`tarsau.c`) tutuldu ve asagidaki adimlar
sirayla gelistirildi:

1. **Iskelet ve `main` fonksiyonu** -- `argv[1]`'i kontrol ederek
   `-b`/`-a` alt komutlarina yonlendiren bir disagiri.
2. **Yardimci fonksiyonlar:**
   - Metin dosyasi dogrulayici (`check_text_file`).
   - Dosya boyutu / izin okuma (`stat` ile).
   - `path_basename` -- dosya yolu yerine sadece dosya adini saklamak
     icin.
   - Dinamik buffer (`buffer_t`) -- organizasyon bolumunu RAM'de
     birlestirmek icin.
3. **`-b` arsivleme akisi:** parametre ayristirma -> on dogrulama (var
   mi, metin mi, toplam boyut < 200 MB mi) -> kayitlari uretme ->
   10-bayt on-ek + kayitlar + dosya icerikleri yazma.
4. **`-a` acma akisi:** `*.sau` uzanti kontrolu -> 10 bayt header okuma
   -> organizasyon bolumunu okuma ve ayristirma -> hedef dizini
   olusturma -> her kayit icin dosya yazma + `chmod` ile izinleri
   kurma.
5. **Hata yonetimi:** her kritik adimda `errno`/`fread`/`fwrite` donus
   degeri kontrol edilerek bellek/dosya sizintisi olmadan duzgun
   sonlandirma.
6. **Test betigi:** `run_tests.sh` ile 14 senaryo otomatik kosulur.
7. **Makefile:** `all`, `clean`, `demo`, `test` hedefleri.

Gelistirme sirasinda kullanilan komutlar:

```bash
make            # Derleme
make test       # 14 testlik otomatik dogrulama
make demo       # Tek komutluk ucu uca demo
make clean      # Tum cikti dosyalarini temizler
```

---

## 4. Onemli Kod Parcalari

### 4.1 Komut secimi (`main`)

```c
int main(int argc, char *argv[]) {
    if (argc < 2) { print_usage(argv[0]); return 1; }
    if (strcmp(argv[1], "-b") == 0)        return cmd_archive(argc, argv);
    else if (strcmp(argv[1], "-a") == 0)   return cmd_extract(argc, argv);
    else if (strcmp(argv[1], "-h") == 0 ||
             strcmp(argv[1], "--help") == 0) { print_usage(argv[0]); return 0; }
    print_usage(argv[0]);
    return 1;
}
```

### 4.2 Metin dosyasi kontrolu

`is_text_byte` yalnizca ASCII (0..127) icindeki yazilabilir karakterleri
ve dogal kontrol karakterlerini (`\n \r \t \b \f \v`) kabul eder; NUL
veya 127'den buyuk baytlar dosyayi metin disi yapar.

```c
static int is_text_byte(unsigned char c) {
    if (c == '\n' || c == '\r' || c == '\t' ||
        c == '\b' || c == '\f' || c == '\v') return 1;
    if (c >= 32 && c <= 126) return 1;
    return 0;
}

static int check_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    unsigned char buf[IO_BUFFER];
    size_t n; int ok = 1;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        for (size_t i = 0; i < n; i++)
            if (!is_text_byte(buf[i])) { ok = 0; break; }
        if (!ok) break;
    }
    fclose(fp);
    return ok;
}
```

Kontrol basarisiz olursa kullaniciya tam olarak sartnamede belirtilen
mesaj basilir:

```c
if (!check_text_file(fp)) {
    fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", fp);
    return 1;
}
```

### 4.3 Organizasyon bolumunun yazilmasi

```c
char rec[MAX_PATH_LEN + 64];
const char *bn = path_basename(files[i]);
int n = snprintf(rec, sizeof(rec), "|%s,%o,%ld", bn, perm, sz);
buf_append(&org, rec, (size_t)n);
...
long first_section_size = (long)HEADER_PREFIX + (long)org.len;
char header_buf[32];
snprintf(header_buf, sizeof(header_buf), "%010ld", first_section_size);
char header[HEADER_PREFIX];
memcpy(header, header_buf, HEADER_PREFIX);

fwrite(header, 1, HEADER_PREFIX, out);
fwrite(org.data, 1, org.len, out);
```

Boyut sifir doldurmali 10 haneli ASCII bicimde yazilir
(`%010ld`). Sonra dosya icerikleri 64 KB'lik bir tampon ile parca parca
kopyalanir.

### 4.4 Acma sirasinda izinlerin geri yazilmasi

```c
char *endp = NULL;
long perm = strtol(permstr, &endp, 8);   /* sekizlik tabanda */
...
fclose(out);
if (chmod(outpath, (mode_t)perm) != 0) {
    fprintf(stderr, "Uyari: %s icin izin atanamadi (%s)\n",
            outpath, strerror(errno));
}
```

### 4.5 Bozuk arsiv tespiti

Acma asamasinda asagidaki dort durumda kullaniciya
`Arsiv dosyasi uygunsuz veya bozuk!` mesaji verilir:

1. Dosya `.sau` ile bitmiyor.
2. Dosya 10 bayttan kisa veya ilk 10 bayt rakam disi.
3. Header'in icerdigi 1. bolum boyutu fiziksel dosya boyutundan buyuk.
4. Organizasyon bolumu `|` ile baslamiyor veya kayit alanlari (virgul
   sayisi vs.) tutmuyor.

---

## 5. Test Sonuclari

### 5.1 Otomatik Testler

`bash run_tests.sh` ciktisi (Ubuntu 24.04, gcc 13.3):

```
=== Test 1: temel arsivleme ve acma ===
  [PASS] demo.sau olusturuldu
  [PASS] t1.txt icerigi ayni
  [PASS] t2.txt icerigi ayni
  [PASS] t3.txt icerigi ayni
  [PASS] t4.txt icerigi ayni
  [PASS] t5.dat icerigi ayni

=== Test 2: izinler korunuyor mu? ===
  [PASS] t1.txt izinleri 640
  [PASS] t2.txt izinleri 600

=== Test 3: arsiv adi belirtilmediginde a.sau ===
  [PASS] a.sau olusturuldu

=== Test 4: ikili (binary) dosya reddi ===
  [PASS] binary dosya reddedildi

=== Test 5: bozuk arsiv reddi ===
  [PASS] bozuk arsiv reddedildi

=== Test 6: yanlis uzantili arsiv reddi ===
  [PASS] .txt arsiv reddedildi

=== Test 7: hedef dizin verilmedi (mevcut dizin) ===
  [PASS] mevcut dizine acildi

=== Test 8: 32 dosya siniri ===
  [PASS] 32+ dosya reddedildi

===============================
  PASS: 14    FAIL: 0
===============================
```

### 5.2 `make demo` Ciktisi

```
==> Arsivleme
./tarsau -b test_files/t1.txt test_files/t2.txt test_files/t3.txt -o demo.sau
Toplam 3 dosya birlestirilerek "demo.sau" arsiv dosyasi olusturuldu.

==> Arsiv icerigi (ilk 200 bayt):
0000000054|t1.txt,640,191|t2.txt,600,147|t3.txt,664,41Merhaba, bu birinci...

==> Acma
./tarsau -a demo.sau cikti
"demo.sau" arsivi "cikti" dizinine acildi (3 dosya).

==> cikti/ icerigi:
total 12
-rw-r----- 1 user user 191 May  9 02:35 t1.txt
-rw------- 1 user user 147 May  9 02:35 t2.txt
-rw-r--r-- 1 user user  41 May  9 02:35 t3.txt
```

### 5.3 Hatali Senaryolar

**Ikili dosya reddi:**

```
$ ./tarsau -b test_files/t1.txt test_files/binary.bin -o demo.sau
test_files/binary.bin giris dosyasinin formati uyumsuzdur!
```

**Bozuk arsiv reddi:**

```
$ echo "boyle bir arsiv olmaz" > bad.sau
$ ./tarsau -a bad.sau cikti
Arsiv dosyasi uygunsuz veya bozuk!
```

**32 dosya siniri:**

```
$ ./tarsau -b f1.txt f2.txt ... f33.txt -o demo.sau
En fazla 32 giris dosyasi arsivlenebilir!
```

---

## 6. Klasor Yapisi

```
tarsau/
|-- tarsau.c           # Tum program kodu (tek kaynak dosya)
|-- Makefile           # make / make test / make demo / make clean
|-- run_tests.sh       # 14 testlik otomatik dogrulama betigi
|-- test_files/        # Ornek ASCII dosyalar
|   |-- t1.txt
|   |-- t2.txt
|   |-- t3.txt
|   |-- t4.txt
|   |-- t5.dat
|-- README.md          # Hizli baslangic
|-- RAPOR.md           # Bu rapor
```

---

## 7. Sonuc

`tarsau`, sartnamede istenen tum gereksinimleri karsilamaktadir:

- [x] C dilinde, Linux uzerinde, `make` ile derleniyor.
- [x] `-b` ile en fazla 32, toplamda <= 200 MB metin dosyasi
      arsivleniyor.
- [x] `-o` opsiyonel; varsayilan arsiv adi `a.sau`.
- [x] `-a` ile arsiv aciliyor, isteyen dizine veya mevcut dizine
      cikariliyor.
- [x] Acilan dosyalar orijinal `chmod` izinlerine geri donuyor.
- [x] Sartnamede yer alan tum hata mesajlari ayni kelimelerle
      uretiliyor.
- [x] Hicbir senaryoda program ani bicimde cokmuyor; tum dosya/bellek
      kaynaklari serbest birakiliyor.
- [x] 14 farkli test senaryosu otomatik gecirildi.
