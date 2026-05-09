# tarsau

Sistem Programlama 2025-2026 Bahar Donemi Projesi.

`tarsau`, **sikistirma yapmadan** birden cok metin dosyasini tek bir
arsiv (`.sau`) dosyasinda toplayan ve daha sonra geri acan basit bir
arsivleme aracidir. `tar`, `zip` veya `rar` araclarina benzer ancak
sadece ASCII metin dosyalari uzerinde calisir.

## Derleme

```bash
make
```

`make` komutu `gcc -Wall -Wextra -Wpedantic -O2 -std=c11` ile
`tarsau` adli bir cikti uretir.

Temizlik icin:

```bash
make clean
```

## Kullanim

### 1) Arsiv olusturma (`-b`)

```bash
./tarsau -b dosya1 dosya2 ... [-o arsiv.sau]
```

Ornek:

```bash
./tarsau -b test_files/t1.txt test_files/t2.txt test_files/t3.txt -o demo.sau
```

Kurallar:

- Giris dosyalari **sadece** metin (ASCII, karakter basina 1 bayt) olabilir.
- En fazla **32** giris dosyasi kabul edilir.
- Toplam giris boyutu **200 MB**'i gecemez.
- `-o` verilmezse cikis dosyasi adi varsayilan olarak **`a.sau`** olur.
- Hatali bir giris dosyasi verildiginde
  `t7 giris dosyasinin formati uyumsuzdur!` mesaji yazilir ve programdan
  duzgun bicimde cikilir.

### 2) Arsivi acma (`-a`)

```bash
./tarsau -a arsiv.sau [hedef_dizin]
```

Ornek:

```bash
./tarsau -a demo.sau cikti
```

Kurallar:

- `-a`'dan sonra en fazla 2 parametre verilebilir.
- Birinci parametre `*.sau` uzantili olmalidir; aksi halde
  `Arsiv dosyasi uygunsuz veya bozuk!` mesaji yazilir.
- Ikinci parametre verilmezse arsiv mevcut dizine acilir.
- Verilen dizin yoksa otomatik olarak olusturulur.
- Acilan dosyalar arsivlenirken sahip olduklari ayni izinlere
  (rwx) sahip olur.

### 3) Hizli demo

```bash
make demo
```

Bu komut ornek dosyalari arsivler, `cikti/` klasorune acar ve sonuclari
ekrana basar.

### 4) Otomatik testler

```bash
make test
# veya
bash run_tests.sh
```

`run_tests.sh` 14 farkli testi sirayla calistirir
(arsivleme/acma, izin koruma, varsayilan dosya adi, ikili dosya reddi,
bozuk arsiv reddi, dizin olmadiginda olusturma, 32 dosya siniri, ...).

## `.sau` Arsiv Format

```
+-----------------------------------------------+
| 10 bayt: ASCII sayisal -> 1. bolumun toplam   |
|         boyutu (10 baytlik on-ek dahil)       |
+-----------------------------------------------+
| |dosya1,izin_oktal,boyut|dosya2,izin_oktal,...|   <-- "|" ile
+-----------------------------------------------+        ayrilmis kayitlar
| <dosya1 icerigi><dosya2 icerigi>...           |   <-- ASCII icerikler
|                                               |       ardisik
+-----------------------------------------------+
```

Ornek (`demo.sau`'nun ilk 50 bayti):

```
0000000054|t1.txt,640,191|t2.txt,600,147|t3.txt,664,41Merhaba, bu...
```

## Dosyalar

| Dosya | Aciklama |
|-------|----------|
| `tarsau.c` | Tum program kodu (tek kaynak dosya) |
| `Makefile` | `make`, `make demo`, `make test`, `make clean` |
| `run_tests.sh` | 14 testten olusan otomatik test betigi |
| `test_files/` | Ornek metin dosyalari (`t1..t4.txt`, `t5.dat`) |
| `RAPOR.md` | Proje raporu (gelistirme sureci, kod parcalari) |

## Notlar

- Program Linux/Unix uzerinde calisacak sekilde yazilmistir
  (`<sys/stat.h>`, `<unistd.h>`, `chmod`, `mkdir` vb.).
- Kod **tek kaynak dosyada** tutulmustur (`tarsau.c`); `make` ile
  dogrudan derlenir.
- Kod tabani **C11** ve `-Wall -Wextra -Wpedantic` ile uyaridan aridir.
