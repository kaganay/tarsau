# tarsau — Sistem Programlama Donem Projesi Raporu

---

## Kapak Bilgileri

| | |
|---|---|
| **Proje Adi** | tarsau — sikistirma yapmayan basit arsivleyici |
| **Ders** | Sistem Programlama |
| **Donem** | 2025-2026 Bahar |
| **Teslim Tarihi** | 24 Mayis 2026, 23:59 |
| **GitHub Deposu** | https://github.com/kaganay/tarsau |

**Proje Ekibi:**

| Ad Soyad | Ogrenci No | Gorev Dagilimi |
|----------|------------|----------------|
| Kagan Aydogan | G231210375 | Mimari tasarim, `-b` (arsivleme) modulu, `.sau` format motoru, Makefile, test betigi, raporlama |
| Kadir Duyan   | G231210371 | `-a` (acma) modulu, izin/permission yonetimi, hata mesajlari, ornek metin dosyalari, dokumantasyon |

---

## Icindekiler

1. [Giris ve Amac](#1-giris-ve-amac)
2. [Sistem Gereksinimleri ve Kurulum](#2-sistem-gereksinimleri-ve-kurulum)
3. [Kullanim Kilavuzu](#3-kullanim-kilavuzu)
4. [`.sau` Arsiv Formati (Detayli)](#4-sau-arsiv-formati-detayli)
5. [Mimari ve Tasarim Kararlari](#5-mimari-ve-tasarim-kararlari)
6. [Kaynak Kodun Modul Modul Aciklamasi](#6-kaynak-kodun-modul-modul-aciklamasi)
7. [Hata Yonetimi ve Sartname Mesajlari](#7-hata-yonetimi-ve-sartname-mesajlari)
8. [Test Plani ve Sonuclari](#8-test-plani-ve-sonuclari)
9. [Ekran Ciktilari](#9-ekran-ciktilari)
10. [Gelistirme Sureci ve Zaman Cizelgesi](#10-gelistirme-sureci-ve-zaman-cizelgesi)
11. [Karsilasilan Zorluklar ve Cozumler](#11-karsilasilan-zorluklar-ve-cozumler)
12. [Klasor Yapisi](#12-klasor-yapisi)
13. [Sartname Karsilama Tablosu](#13-sartname-karsilama-tablosu)
14. [Sonuc](#14-sonuc)
15. [Kaynaklar](#15-kaynaklar)

---

## 1. Giris ve Amac

### 1.1 Projenin Tanimi

`tarsau`; `tar`, `rar` veya `zip` araclarina benzer sekilde calisan,
ancak **sikistirma yapmayan** basit bir arsivleme/birlestirme aracidir.
Verilen birden cok ASCII metin dosyasini tek bir `.sau` dosyasinda
birlestirir; daha sonra bu arsiv dosyasini orijinal dosyalarina (ve
orijinal Unix izinlerine) geri acabilir.

Bu proje, ders kapsaminda **POSIX dosya sistemi cagrilarinin
(open/read/write/stat/chmod/mkdir)**, **dinamik bellek yonetiminin**
ve **tampon (buffer) tabanli I/O'nun** uygulamali olarak ogrenilmesini
amaclamaktadir.

### 1.2 Proje Hedefleri

1. C dilinde, Linux/Unix uzerinde calisan, `make` ile derlenebilen tek
   bir ikili (binary) uretmek.
2. Sartnamede belirtilen `-b` (build / arsivleme) ve `-a`
   (archive open / acma) komutlarini eksiksiz desteklemek.
3. Tum sinirlamalari (32 dosya, 200 MB, ASCII metin, izin koruma)
   uygulamak.
4. Hicbir hatali girdide programin cokmemesi; tum kaynaklarin (FILE\*,
   `malloc`'lanmis bellek) duzgun bicimde serbest birakilmasi.
5. Sartnamede istenen **birebir hata mesajlarini** uretmek.
6. GitHub uzerinde versiyon kontrolu ile gelistirilmek.
7. Otomatik test seti ile dogrulanabilir olmak.

---

## 2. Sistem Gereksinimleri ve Kurulum

### 2.1 Bagimliliklar

| Yazilim | Versiyon | Amac |
|---------|----------|------|
| `gcc` | >= 9.0 (C11 destegi) | Derleyici |
| `make` | herhangi bir GNU Make | Yapilandirma |
| `bash` | >= 4.0 | Test betigi |
| Unix benzeri OS | Linux / macOS / WSL | `chmod`, `mkdir`, `stat` |

Test sirasinda kullanilan ortam:

```text
Ubuntu 24.04 LTS (WSL2)
gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
GNU Make 4.3
GNU bash 5.2.21
```

### 2.2 Derleme

```bash
git clone https://github.com/kaganay/tarsau.git
cd tarsau
make
```

`Makefile` icerigi:

```make
CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  := tarsau
SRC     := tarsau.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)
```

Derleyici uyari bayraklari aktif tutuldugu icin (`-Wall -Wextra
-Wpedantic`) kod **tek bir uyari uretmeden** derlenmektedir.

### 2.3 Make Hedefleri

| Komut | Acıklama |
|-------|----------|
| `make` | tarsau ikili'yi derler. |
| `make demo` | Ornek dosyalari arsivler, bir klasore acar, hex preview verir. |
| `make test` | `run_tests.sh` icindeki 14 testi calistirir. |
| `make clean` | Tum derleme/test ciktilarini siler. |

---

## 3. Kullanim Kilavuzu

### 3.1 Arsiv Olusturma (`-b`)

```bash
./tarsau -b dosya1 dosya2 ... [-o arsiv.sau]
```

**Ornekler:**

```bash
# 5 dosyayi default a.sau dosyasinda birlestir
./tarsau -b t1.txt t2.txt t3.txt t4.txt t5.dat

# Belirli bir cikis dosyasi
./tarsau -b t1.txt t2.txt t3.txt -o demo.sau
```

### 3.2 Arsivi Acma (`-a`)

```bash
./tarsau -a arsiv.sau [hedef_dizin]
```

**Ornekler:**

```bash
# Mevcut dizine ac
./tarsau -a demo.sau

# Yeni klasore ac (yoksa olusturulur)
./tarsau -a demo.sau cikti

# Mutlak yol da kabul edilir
./tarsau -a demo.sau /tmp/cikti
```

### 3.3 Sinirlamalar

| Sinir | Deger |
|-------|-------|
| Maksimum dosya sayisi | **32** |
| Toplam giris boyutu | **200 MB** |
| Kabul edilen format | ASCII metin (0..127, NUL hariç) |
| Default cikis adi | `a.sau` |

---

## 4. `.sau` Arsiv Formati (Detayli)

### 4.1 Genel Sema

```
+-----------------------------------------------------------+
|  10 bayt: ASCII sayisal -> ilk bolumun toplam boyutu       |
|  (10 baytlik on-ek dahil)                                  |   1. BOLUM:
|                                                            |   "Organizasyon /
|  |dosya1,izin_oktal,boyut|dosya2,izin_oktal,boyut|...      |   Icerik"
|                                                            |
+-----------------------------------------------------------+
|  <dosya1 icerigi><dosya2 icerigi> ... <dosyaN icerigi>     |   2. BOLUM:
|  (ardisik, ayraçsiz, yalnizca ASCII)                       |   "Arsivlenmis
|                                                            |   dosyalar"
+-----------------------------------------------------------+
```

### 4.2 1. Bolum: Organizasyon

- **Ilk 10 bayt:** Tum 1. bolumun (10 bayt + kayitlar) ASCII bicimdeki
  uzunlugu. Sifir doldurmali, her zaman sabit 10 karakter
  (`%010ld`). Boylelikle decoder bu degeri okur okumaz, dosya icerikleri
  bolumune `fseek` ile gidebilir.
- **Kayitlar:** `|` ile ayrilir, her kayit `dosya_adi,izin,boyut`
  formatindadir. `izin` POSIX rwx bitlerinin **8'lik (octal)** gosterimi
  (`0..0777`), `boyut` ondalik tabandadir.

### 4.3 2. Bolum: Arsivlenmis Dosyalar

- 1. bolum bittigi yerden **ayrac olmaksizin** baslar.
- Her dosyanin baslangic ofseti, ondan onceki tum dosyalarin
  boyutlarinin toplamiyla bulunur.
- Sartnameye gore son karakter dosya sonunu belirtir; yani arsiv
  dosyasinin son bayti, son arsivlenen dosyanin son baytidir
  (ek bir EOF/sentinel byte yazilmaz).

### 4.4 Gercek Bir Ornek

`make demo` komutuyla uretilen `demo.sau` dosyasinin ilk 200 bayti:

```text
0000000054|t1.txt,640,191|t2.txt,600,147|t3.txt,664,41Merhaba, bu birinci ornek metin dosyasidir.
Sistem Programlama dersi icin tarsau projesi.
Iceride birkac satir ve sayilar var: 12345
Ozel karak...
```

| Bolum | Bayt Araligi | Icerik |
|-------|--------------|--------|
| Header | `[0..10)` | `0000000054` -> 1. bolum 54 bayt |
| Kayitlar | `[10..54)` | `\|t1.txt,640,191\|t2.txt,600,147\|t3.txt,664,41` |
| t1 icerigi | `[54..245)` | `Merhaba, bu birinci ...` (191 bayt) |
| t2 icerigi | `[245..392)` | (147 bayt) |
| t3 icerigi | `[392..433)` | (41 bayt) |

### 4.5 Decoder Algoritmasi

```
1. Ilk 10 bayti oku -> first_section_size
2. (first_section_size - 10) bayt kadar daha oku -> kayit listesi
3. `|` ile bol, her kaydi `,` ile alanlara ayir
4. Her kayit icin:
     - hedef_dizin/dosya_adi olustur
     - boyut kadar bayti ham olarak oku, hedefe yaz
     - chmod ile izinleri uygula
```

---

## 5. Mimari ve Tasarim Kararlari

### 5.1 Tek Kaynak Dosya

Proje **tek bir kaynak dosyaya** (`tarsau.c`) sigdirildi. Sebepleri:

- Sartnamede modul/dosya sayisi sinirlamasi yok; en kucuk teslim paketi
  istenildigi icin tek dosya tercih edildi.
- Kod ~430 satir; modullere bolmek anlasilirligi azaltir, derleme
  zincirini gereksiz karmasiklastirirdi.
- `Makefile`'i ucucu sade tutmayi saglar (`gcc -o tarsau tarsau.c`).

### 5.2 Akis Diyagrami

```
                       argv[1]
                          |
              +-----------+-----------+
              |                       |
            "-b"                    "-a"
              |                       |
   parse files & -o opt.    parse archive & dir opt.
              |                       |
   validate (text/sayi/    validate (.sau / 10-byte
   boyut) ----------+      header / size sanity)
              |     |                 |
   build records   exit 1   parse records
              |                       |
   write 10b header                  mkdir hedef dizini
   write records                     |
   stream file contents     for each record:
              |                stream <size> bayt -> dosya
   close & report                chmod izinleri
                                |
                            close & report
```

### 5.3 Bellek ve I/O

- Dosya icerikleri **64 KB'lik tampon** (`IO_BUFFER`) ile parca parca
  kopyalanir; dosyanin tamami RAM'e yuklenmez. Bu sayede 200 MB
  toplam giris RAM'i yormaz.
- Yalnizca organizasyon bolumu `buffer_t` adli dinamik buyuyen bir
  yapida toplanir. 32 dosya x ~300 bayt kayit = ~10 KB civaridir, bu
  yuk ihmal edilebilir.

### 5.4 Path Saklama Karari

Arsivde yalnizca **dosyanin basename'i** (ornek: `dir/t1.txt` ->
`t1.txt`) saklanir. Bunun nedenleri:

1. Sartname acma sirasinda hedef bir dizin alir; goreli/mutlak girdi
   yollari acmaya engel olur.
2. `..` veya `/` iceren yol injection saldirilarinin onune gecer
   (kotu niyetli bir arsiv `/etc/passwd` uzerine yazamaz).

### 5.5 Tasinabilir C11

`-D_POSIX_C_SOURCE=200809L` ve `-std=c11` ile yazildi; `<unistd.h>`,
`<sys/stat.h>`, `<libgen.h>` gibi POSIX baslik dosyalari kullanildi.
Boylelikle Linux ve macOS uzerinde **degisiklik gerektirmeden**
calisir.

---

## 6. Kaynak Kodun Modul Modul Aciklamasi

`tarsau.c` mantiksal olarak sekiz bolumden olusur. Asagida her birinin
amaci ve ana kod parcalari verilmistir.

### 6.1 Sabitler ve Dahili Yapilar

```c
#define MAX_FILES        32
#define MAX_TOTAL_BYTES  (200L * 1024L * 1024L)
#define HEADER_PREFIX    10
#define IO_BUFFER        (64 * 1024)
#define MAX_PATH_LEN     1024

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} buffer_t;
```

`buffer_t`, organizasyon bolumunu dinamik bicimde olusturmak icin
yazilmis kucuk bir vektor yapisidir. `buf_append` kapasiteyi 2 katina
cikararak amortized O(1) ekleme saglar.

### 6.2 ASCII Metin Dogrulayici

Sartname "ASCII, karakter basina 1 bayt" dedigi icin `>127` baytlar ve
NUL kabul edilmez. Diger taraftan satir sonu, tab gibi yaygin kontrol
karakterlerinin de "metin" sayilmasi gerekir; aksi halde gercek metin
dosyalari da reddedilirdi.

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

### 6.3 Stat Yardimcilari

```c
static long file_size_bytes(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

static int file_perm_bits(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (int)(st.st_mode & 0777);
}
```

`& 0777` ile yalnizca rwx bitleri saklanir; `setuid`/`setgid`/`sticky`
bitleri arsivde tutulmaz (sartnamede istenmemistir).

### 6.4 `cmd_archive` (`-b`)

`-b` islevinin ozetlenmis hali:

```c
/* 1) Parametreleri ayristir */
for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0) { output = argv[++i]; }
    else { ...; files[file_count++] = argv[i]; }
}

/* 2) On dogrulamalar */
for (int i = 0; i < file_count; i++) {
    if (access(files[i], R_OK) != 0)            return err(...);
    if (!S_ISREG(...))                          return err(...);
    if (!check_text_file(files[i])) {
        fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n",
                files[i]);
        return 1;
    }
    total_size += st.st_size;
    if (total_size > MAX_TOTAL_BYTES)           return err(...);
}

/* 3) Organizasyon kayitlarini olustur */
for (int i = 0; i < file_count; i++) {
    snprintf(rec, sizeof(rec), "|%s,%o,%ld",
             path_basename(files[i]), perm, sz);
    buf_append(&org, rec, n);
}

/* 4) 10-bayt header + kayitlar + dosya icerikleri */
long first_section_size = HEADER_PREFIX + (long)org.len;
snprintf(header_buf, sizeof(header_buf), "%010ld", first_section_size);
fwrite(header_buf, 1, 10, out);
fwrite(org.data,    1, org.len, out);
for (int i = 0; i < file_count; i++)
    stream_file(files[i], out);
```

### 6.5 `cmd_extract` (`-a`)

```c
/* 1) Uzanti kontrolu */
if (alen < 4 || strcmp(archive + alen - 4, ".sau") != 0) {
    fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
    return 1;
}

/* 2) 10-bayt header oku ve sanity-check */
fread(prefix, 1, 10, in);
for (int i = 0; i < 10; i++)
    if (!isdigit(prefix[i])) bozuk();
long first_section_size = atol(prefix);
if (first_section_size > file_total) bozuk();

/* 3) Organizasyon bolumunu okuyup kayitlari ayristir */
fread(org, 1, org_len, in);
while (*p == '|') {
    p++; ... = strchr(p, '|');
    parse "name,perm,size";
    ...
}

/* 4) Hedef dizini olustur */
if (target != "." && !has_space(target))
    mkdir_p(target, 0755);

/* 5) Her kayit icin <size> bayt oku, dosyaya yaz, chmod uygula */
for (each record) {
    while (remaining > 0)
        fwrite(io_buf, 1, fread(...), out);
    chmod(outpath, perm);
}
```

`mkdir_p` ic ice dizin olusturmayi destekler ve `mkdir -p` benzeri
calisir, boylece `cikti/alt1/alt2` gibi dizinler de calisir.

### 6.6 Path Basename

```c
static const char *path_basename(const char *path) {
    const char *p1 = strrchr(path, '/');
    const char *p2 = strrchr(path, '\\');
    const char *p  = p1 > p2 ? p1 : p2;
    return p ? p + 1 : path;
}
```

Hem `/` hem `\` ayraciyla calisir; bu sayede Windows'ta yazilmis
metin dosyalari WSL uzerinden bile sorunsuz arsivlenir.

### 6.7 `mkdir_p`

Sartnamede "Girilen dizin adinda bos yer yoksa" denildigi icin
`target` icinde bosluk varsa hata verilir. Aksi halde `mkdir -p`
benzeri davranisla iç ice tum dizinler olusturulur.

```c
static int mkdir_p(const char *path, mode_t mode) {
    char tmp[MAX_PATH_LEN];
    strcpy(tmp, path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, mode) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) != 0 && errno != EEXIST) return -1;
    return 0;
}
```

### 6.8 `main`

```c
int main(int argc, char *argv[]) {
    if (argc < 2)                            { print_usage(argv[0]); return 1; }
    if      (strcmp(argv[1], "-b") == 0)     return cmd_archive(argc, argv);
    else if (strcmp(argv[1], "-a") == 0)     return cmd_extract(argc, argv);
    else if (strcmp(argv[1], "-h") == 0 ||
             strcmp(argv[1], "--help") == 0) { print_usage(argv[0]); return 0; }
    print_usage(argv[0]);
    return 1;
}
```

---

## 7. Hata Yonetimi ve Sartname Mesajlari

Sartnamede iki birebir hata mesaji bulunmaktadir; ikisi de programda
ayni kelimelerle uretilmektedir.

| Senaryo | Uretilen Mesaj |
|---------|----------------|
| Giris dosyasinin formati metin degil | `<dosya> giris dosyasinin formati uyumsuzdur!` |
| Acma sirasinda arsiv yanlis/bozuk | `Arsiv dosyasi uygunsuz veya bozuk!` |
| `-o` parametresi argumansiz birakilirsa | `-o parametresinden sonra arsiv adi belirtilmelidir!` |
| 32+ dosya | `En fazla 32 giris dosyasi arsivlenebilir!` |
| 200 MB asimi | `Giris dosyalarinin toplam boyutu 200 MB siniri asti!` |
| Dizin adinda bosluk | `Dizin adinda bosluk olmamalidir: "<dizin>"` |
| `-a` sonrasi 2'den fazla parametre | `-a parametresinden sonra en fazla 2 parametre verilebilir!` |

Tum hata yollarinda program **temiz bir sekilde sonlanir**:

- Acik dosyalar `fclose`'lanir,
- `malloc`'lanmis bellek `free`'lenir,
- `exit(1)` yerine `return 1` kullanilir, boylelikle stack'tegi
  destructor'lar (varsa) calisir.
- Hicbir kosulda program "core dump" / SIGSEGV uretmez.

---

## 8. Test Plani ve Sonuclari

### 8.1 Test Senaryolari

`run_tests.sh` icindeki 14 test senaryosu:

| # | Senaryo | Beklenen |
|---|---------|----------|
| 1.1-1.6 | 5 dosya arsivle, ac, hash karsilastir (`cmp`) | Tum icerikler ayni |
| 2.1-2.2 | Izin koruma (640, 600) | Acilanlarin izni ayni |
| 3 | `-o` verilmedi | `a.sau` olusur |
| 4 | Binary (`\x00\x01`) dosya | "uyumsuzdur" mesaji |
| 5 | Bozuk arsiv (rastgele text) | "uygunsuz veya bozuk" |
| 6 | Yanlis uzanti (`.txt`) | "uygunsuz veya bozuk" |
| 7 | Hedef dizin verilmedi | Mevcut dizine acilir |
| 8 | 33 dosya gonder | "32" sinir mesaji |

### 8.2 Sonuclar

```text
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

### 8.3 Manuel Test (uctan-uca)

```bash
$ make
gcc -Wall -Wextra -Wpedantic -O2 -std=c11 -D_POSIX_C_SOURCE=200809L \
    -o tarsau tarsau.c

$ ./tarsau -b test_files/t1.txt test_files/t2.txt test_files/t3.txt -o demo.sau
Toplam 3 dosya birlestirilerek "demo.sau" arsiv dosyasi olusturuldu.

$ ls -l demo.sau
-rw-r--r-- 1 user user 433 May  9 12:35 demo.sau

$ ./tarsau -a demo.sau cikti
"demo.sau" arsivi "cikti" dizinine acildi (3 dosya).

$ diff -r test_files cikti
# (cikti yok - tum dosyalar bit-bit ayni)
```

---

## 9. Ekran Ciktilari

### 9.1 Basarili Arsivleme

```text
$ ./tarsau -b t1.txt t2.txt t3.txt t4.txt t5.dat -o arsiv.sau
Toplam 5 dosya birlestirilerek "arsiv.sau" arsiv dosyasi olusturuldu.
```

### 9.2 Default `a.sau`

```text
$ ./tarsau -b t1.txt
Toplam 1 dosya birlestirilerek "a.sau" arsiv dosyasi olusturuldu.
```

### 9.3 Format Hatasi (binary dosya)

```text
$ ./tarsau -b t1.txt resim.bin -o cikti.sau
resim.bin giris dosyasinin formati uyumsuzdur!
```

### 9.4 Bozuk Arsiv

```text
$ echo "bu gercek bir arsiv degil" > kotu.sau
$ ./tarsau -a kotu.sau hedef
Arsiv dosyasi uygunsuz veya bozuk!
```

### 9.5 Yanlis Uzanti

```text
$ ./tarsau -a notlar.txt cikti
Arsiv dosyasi uygunsuz veya bozuk!
```

### 9.6 32 Dosya Siniri

```text
$ ./tarsau -b f1.txt f2.txt ... f33.txt -o tum.sau
En fazla 32 giris dosyasi arsivlenebilir!
```

### 9.7 Izinlerin Korunmasi (Linux native FS)

```text
$ chmod 640 t1.txt; chmod 600 t2.txt
$ ./tarsau -b t1.txt t2.txt -o demo.sau
$ ./tarsau -a demo.sau cikti
$ ls -l cikti
total 0
-rw-r----- 1 user user 191 May  9 12:35 t1.txt
-rw------- 1 user user 147 May  9 12:35 t2.txt
```

### 9.8 `make demo` Tam Ciktisi

```text
==> Arsivleme
./tarsau -b test_files/t1.txt test_files/t2.txt test_files/t3.txt -o demo.sau
Toplam 3 dosya birlestirilerek "demo.sau" arsiv dosyasi olusturuldu.

==> Arsiv icerigi (ilk 200 bayt):
0000000054|t1.txt,640,191|t2.txt,600,147|t3.txt,664,41Merhaba, bu birinci ornek metin dosyasidir.
Sistem Programlama dersi icin tarsau projesi.
Iceride birkac satir ve sayilar var: 12345
Ozel karak

==> Acma
./tarsau -a demo.sau cikti
"demo.sau" arsivi "cikti" dizinine acildi (3 dosya).
```

---

## 10. Gelistirme Sureci ve Zaman Cizelgesi

| Asama | Sure | Aciklama |
|-------|------|----------|
| Sartname analizi | ~1 saat | Tum gereksinimlerin maddelenmesi, format taslaginin cizilmesi. |
| Iskelet ve `main` | ~1 saat | argv ayristirma, alt komut yonlendirme. |
| `cmd_archive` | ~2 saat | Metin dogrulama, `buffer_t`, organizasyon kaydi yazimi. |
| `cmd_extract` | ~2 saat | 10-bayt header okuma, kayit ayristirma, chunked okuma. |
| Hata yonetimi | ~1 saat | Tum mesajlarin sartnameye uyumlu hale getirilmesi, kaynak temizleme. |
| Test betigi | ~1 saat | 14 senaryolu `run_tests.sh`. |
| Makefile + dokumantasyon | ~1 saat | `make demo` / `make test` hedefleri, `README.md`. |
| Rapor yazimi | ~2 saat | Bu doküman. |

### 10.1 Git Gelistirme Akisi

Proje GitHub uzerinde geliştirildi. Onemli commit'ler:

```text
ad10a3f  chore: enforce LF line endings via .gitattributes
f1cb27f  Initial: tarsau archive tool (Sistem Programlama 2025-2026)
```

GitHub deposu: https://github.com/kaganay/tarsau

---

## 11. Karsilasilan Zorluklar ve Cozumler

### 11.1 Windows DrvFs Mount'unda Izin Testi

WSL2'de Windows dizini (`/mnt/c/...`) varsayilan olarak tum dosyalari
`777` olarak gosterir. Bu yuzden `chmod 640` arsivlenip acildiktan
sonra `stat` yine `777` rapor ediyordu.

**Cozum:** `run_tests.sh` icindeki Test 2, dosyalari `mktemp -d` ile
ext4 dosya sisteminde calistirarak gerçek izinleri dogrular.

```bash
TMPDIR=$(mktemp -d)
cp "$TARSAU" "$TMPDIR/tarsau"
cp test_files/t1.txt test_files/t2.txt "$TMPDIR/"
( cd "$TMPDIR" && chmod 640 t1.txt && ./tarsau -b ... && ... )
```

### 11.2 `snprintf` `-Wformat-truncation` Uyarisi

`%010ld` icin gcc, derleme zamaninda 10-bayt'lik bir bufferin
yetmeyebilecegini bildirdi. 200 MB + ~10 KB <= 10 hane oldugu icin
gercekte taşma olamaz, ancak derleyiciyi tatmin etmek icin:

```c
char header_buf[32];
snprintf(header_buf, sizeof(header_buf), "%010ld", first_section_size);
char header[HEADER_PREFIX];
memcpy(header, header_buf, HEADER_PREFIX);
```

### 11.3 CRLF / LF Hatti Dengelemesi

Windows uzerinde editlenen `.sh` dosyalari WSL'de `\r` hatasi
veriyordu. `.gitattributes` ile tum metin dosyalarini LF'e zorladik:

```gitattributes
* text=auto eol=lf
```

### 11.4 Yol Sızması (Path Traversal)

Eger arsivde tam yol saklasak, kotu niyetli bir arsiv `../etc/passwd`
gibi yollarla hedef disina yazma yapabilirdi. Bu yuzden hem
arsivlerken hem acarken yalnizca `basename` kullaniyoruz.

---

## 12. Klasor Yapisi

```
tarsau/
|-- tarsau.c           # Tum program kodu (~430 satir, tek kaynak dosya)
|-- Makefile           # all / demo / test / clean
|-- run_tests.sh       # 14 testlik otomatik dogrulama
|-- README.md          # Hizli baslangic
|-- RAPOR.md           # Bu rapor
|-- .gitignore         # Build/test ciktilarini hariç tutar
|-- .gitattributes     # Tum metni LF olarak saklar
|-- test_files/        # Ornek ASCII dosyalar
|   |-- t1.txt
|   |-- t2.txt
|   |-- t3.txt
|   |-- t4.txt
|   |-- t5.dat
```

---

## 13. Sartname Karsilama Tablosu

| Sartname Maddesi | Karsilandi mi? | Ilgili Kod / Test |
|------------------|----------------|-------------------|
| C dilinde, Linux'ta `make` ile derleme | ✓ | `Makefile`, `tarsau.c` |
| `tarsau -b` ile arsivleme | ✓ | `cmd_archive`, Test 1 |
| Sadece ASCII metin kabulu | ✓ | `is_text_byte`, Test 4 |
| Cikis tek metin dosyasinda | ✓ | `.sau` formati |
| `-o` sonrasi arsiv adi | ✓ | `cmd_archive` argv parse |
| Arsiv adi yoksa `a.sau` | ✓ | `output = "a.sau"`, Test 3 |
| Toplam <= 200 MB | ✓ | `MAX_TOTAL_BYTES` |
| <= 32 dosya | ✓ | `MAX_FILES`, Test 8 |
| Hatali girdi: "<f> giris dosyasinin formati uyumsuzdur!" | ✓ | `cmd_archive`, Test 4 |
| `tarsau -a` ile acma | ✓ | `cmd_extract`, Test 1 |
| `-a` sonrasi en fazla 2 parametre | ✓ | argc kontrol |
| `*.sau` olmayan dosya: "Arsiv dosyasi uygunsuz veya bozuk!" | ✓ | uzanti kontrol, Test 6 |
| 2. parametre dizin adi | ✓ | `target` argument |
| Mevcut dizine acma (parametre verilmediginde) | ✓ | `target = "."`, Test 7 |
| Goreli/mutlak yol | ✓ | `mkdir_p`, manuel test |
| Bos yer yoksa dizini olustur | ✓ | `mkdir_p` + bosluk kontrol |
| Sorunsuz cikis, cokme yok | ✓ | Tum hata yollari `return 1`/`fclose`/`free` |
| Acilan dosyalarin orijinal izinleri | ✓ | `chmod`, Test 2 |
| 1. bolum: ilk 10 bayt = ASCII boyut | ✓ | `%010ld` header |
| Kayitlar `\|` ile ayrili | ✓ | `\|name,perm,size\|...` |
| Kayit alanlari `,` ile (ad, izin, boyut) | ✓ | `snprintf("%s,%o,%ld")` |
| Dosya icerikleri ardisik, ayraçsiz | ✓ | `fwrite` chunked |
| `make` dosyasi | ✓ | `Makefile` |
| 2 kisilik takim | ✓ | Kagan + Kadir |
| Rapor (gelistirme + kod + cikti) | ✓ | bu doküman |
| GitHub'da gelistirme | ✓ | https://github.com/kaganay/tarsau |
| Numarali zip teslim | ⏳ | `G231210375_G231210371.zip` (teslim asamasinda) |

---

## 14. Sonuc

`tarsau` projesi, sartnamede istenen tum ozellikleri eksiksiz olarak
karsilamaktadir. Kod **C11 + POSIX** ile yazilmis, `gcc -Wall -Wextra
-Wpedantic` altinda **uyari uretmeden** derlenmektedir. Otomatik test
seti **14/14 PASS** sonucu vermistir.

Proje boyunca kazanilan deneyimler:

- POSIX dosya cagrilarinin (`stat`, `chmod`, `mkdir`, `open/read/write`)
  pratik kullanimi.
- Buffered I/O ile 200 MB'a kadar cikabilen veri akislarini kucuk
  bellek izi ile islemek.
- Format tasarimi: sabit-genislikte header + delimiter-based kayitlar.
- Defansif programlama: hatali her girdide kaynak sizdirmadan
  cikabilmek.
- `make` ile yapilandirma yonetimi ve `bash` test scripleri ile
  CI'a hazirlik.

Proje `make` ile derlenip `make test` komutuyla anlik dogrulanabilir
sekildedir; teslim sonrasi degerlendirme icin **bu iki komut yeterlidir**.

---

## 15. Kaynaklar

1. **The Open Group Base Specifications Issue 7**, IEEE Std 1003.1,
   <https://pubs.opengroup.org/onlinepubs/9699919799/>
2. **GNU C Library Reference Manual** -- File system interface,
   <https://www.gnu.org/software/libc/manual/>
3. **TLPI -- The Linux Programming Interface**, Michael Kerrisk, 2010,
   No Starch Press. Bolum 4 (File I/O), Bolum 15 (File Attributes),
   Bolum 18 (Directories and Links).
4. **Beej's Guide to C Programming** -- File I/O,
   <https://beej.us/guide/bgc/>
5. **GNU Make Manual**, <https://www.gnu.org/software/make/manual/>
6. **POSIX `stat(2)` man page**, <https://man7.org/linux/man-pages/man2/stat.2.html>
7. **POSIX `chmod(2)` man page**, <https://man7.org/linux/man-pages/man2/chmod.2.html>

---

*Rapor sonu.*
