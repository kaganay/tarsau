/*
 * tarsau - sikistirma yapmayan basit arsivleyici
 *
 * Sistem Programlama 2025-2026 Bahar Donemi Projesi
 *
 * Kullanim:
 *   tarsau -b dosya1 dosya2 ... [-o arsiv.sau]
 *   tarsau -a arsiv.sau [hedef_dizin]
 *
 * Arsiv (.sau) dosya formati:
 *   [10 bayt] : ASCII sayisal -> ilk bolumun (organizasyon) toplam boyutu
 *               (bu 10 baytlik on-ek dahil edilir)
 *   |dosya1,izin_oktal,boyut|dosya2,izin_oktal,boyut|...|dosyaN,izin_oktal,boyut
 *   <dosya1 icerigi><dosya2 icerigi>...<dosyaN icerigi>
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#define MAX_FILES        32
#define MAX_TOTAL_BYTES  (200L * 1024L * 1024L)   /* 200 MB */
#define HEADER_PREFIX    10                        /* ilk 10 bayt */
#define IO_BUFFER        (64 * 1024)
#define MAX_PATH_LEN     1024

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Kullanim:\n"
        "  %s -b dosya1 dosya2 ... [-o arsiv.sau]\n"
        "  %s -a arsiv.sau [hedef_dizin]\n",
        prog, prog);
}

/* ---------------------------------------------------------------------------
 * Yardimci fonksiyonlar
 * ------------------------------------------------------------------------ */

/* Verilen bir baytin "metin" karakteri olup olmadigini kontrol eder.
 * ASCII (0..127) ve printable + bilinen kontrol karakterleri (\n, \r, \t, \b, \f, \v) kabul edilir.
 * NUL bayti (0) ve diger ikili kontrol karakterleri reddedilir. */
static int is_text_byte(unsigned char c) {
    if (c == '\n' || c == '\r' || c == '\t' ||
        c == '\b' || c == '\f' || c == '\v') {
        return 1;
    }
    if (c >= 32 && c <= 126) return 1;   /* printable ASCII */
    return 0;                            /* >127 veya NUL/diger kontrol */
}

/* Dosyanin metin dosyasi olup olmadigini denetler.
 * Bos dosya da metin sayilir.
 * Dosya acilamaz veya metin disi bayt iceriyorsa 0 doner. */
static int check_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;

    unsigned char buf[IO_BUFFER];
    size_t n;
    int ok = 1;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        for (size_t i = 0; i < n; i++) {
            if (!is_text_byte(buf[i])) { ok = 0; break; }
        }
        if (!ok) break;
    }
    fclose(fp);
    return ok;
}

/* Dosya boyutunu doner; hata halinde -1. */
static long file_size_bytes(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (long)st.st_size;
}

/* Dosyanin POSIX izin bitlerini (rwx) doner; hata halinde -1. */
static int file_perm_bits(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return -1;
    return (int)(st.st_mode & 0777);
}

/* Yola sahip bir dosyayi sadece basename olarak doner (girdi degistirilmez). */
static const char *path_basename(const char *path) {
    const char *p1 = strrchr(path, '/');
    const char *p2 = strrchr(path, '\\');
    const char *p  = p1 > p2 ? p1 : p2;
    return p ? p + 1 : path;
}

/* Esnek bir bayt buffer yapisi (organizasyon bolumunu olusturmak icin). */
typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} buffer_t;

static int buf_append(buffer_t *b, const char *s, size_t n) {
    if (b->len + n + 1 > b->cap) {
        size_t newcap = b->cap ? b->cap : 256;
        while (newcap < b->len + n + 1) newcap *= 2;
        char *p = (char *)realloc(b->data, newcap);
        if (!p) return -1;
        b->data = p;
        b->cap = newcap;
    }
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return 0;
}

/* ---------------------------------------------------------------------------
 * -b : ARSIVLEME
 * ------------------------------------------------------------------------ */

static int cmd_archive(int argc, char *argv[]) {
    const char *files[MAX_FILES];
    int file_count = 0;
    const char *output = "a.sau";

    /* Parametreleri ayristir: argv[1] == "-b" oldugu zaten dogrulandi */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "-o parametresinden sonra arsiv adi belirtilmelidir!\n");
                return 1;
            }
            output = argv[i + 1];
            i++;
        } else {
            if (file_count >= MAX_FILES) {
                fprintf(stderr, "En fazla %d giris dosyasi arsivlenebilir!\n", MAX_FILES);
                return 1;
            }
            files[file_count++] = argv[i];
        }
    }

    if (file_count == 0) {
        fprintf(stderr, "En az bir giris dosyasi belirtmelisiniz!\n");
        print_usage(argv[0]);
        return 1;
    }

    /* On dogrulamalar: erisim, metin kontrolu, toplam boyut */
    long total_size = 0;
    for (int i = 0; i < file_count; i++) {
        const char *fp = files[i];

        if (access(fp, R_OK) != 0) {
            fprintf(stderr, "%s dosyasi acilamiyor: %s\n", fp, strerror(errno));
            return 1;
        }

        struct stat st;
        if (stat(fp, &st) != 0 || !S_ISREG(st.st_mode)) {
            fprintf(stderr, "%s gecerli bir dosya degil!\n", fp);
            return 1;
        }

        if (!check_text_file(fp)) {
            fprintf(stderr, "%s giris dosyasinin formati uyumsuzdur!\n", fp);
            return 1;
        }

        total_size += (long)st.st_size;
        if (total_size > MAX_TOTAL_BYTES) {
            fprintf(stderr,
                "Giris dosyalarinin toplam boyutu 200 MB siniri asti!\n");
            return 1;
        }
    }

    /* Organizasyon (icerik) bolumunu RAM'de hazirla */
    buffer_t org = {0};
    for (int i = 0; i < file_count; i++) {
        long sz = file_size_bytes(files[i]);
        int perm = file_perm_bits(files[i]);
        if (sz < 0 || perm < 0) {
            fprintf(stderr, "%s dosya bilgileri okunamadi!\n", files[i]);
            free(org.data);
            return 1;
        }
        char rec[MAX_PATH_LEN + 64];
        /* Sadece basename'i sakliyoruz; acma sirasinda hedef dizine yazilacak. */
        const char *bn = path_basename(files[i]);
        int n = snprintf(rec, sizeof(rec), "|%s,%o,%ld", bn, perm, sz);
        if (n < 0 || (size_t)n >= sizeof(rec)) {
            fprintf(stderr, "Dosya adi cok uzun: %s\n", files[i]);
            free(org.data);
            return 1;
        }
        if (buf_append(&org, rec, (size_t)n) != 0) {
            fprintf(stderr, "Bellek yetersiz!\n");
            free(org.data);
            return 1;
        }
    }

    /* Ilk 10 bayt: 10 baytlik on-ek dahil ilk bolum boyutu (sifir doldurmali) */
    long first_section_size = (long)HEADER_PREFIX + (long)org.len;
    if (first_section_size < 0 || first_section_size > 9999999999L) {
        fprintf(stderr, "Organizasyon bolumu cok buyuk!\n");
        free(org.data);
        return 1;
    }
    char header_buf[32];
    snprintf(header_buf, sizeof(header_buf), "%010ld", first_section_size);
    char header[HEADER_PREFIX];
    memcpy(header, header_buf, HEADER_PREFIX);

    /* Cikisi yaz */
    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "%s arsiv dosyasi olusturulamadi: %s\n",
                output, strerror(errno));
        free(org.data);
        return 1;
    }

    if (fwrite(header, 1, HEADER_PREFIX, out) != HEADER_PREFIX ||
        fwrite(org.data, 1, org.len, out) != org.len) {
        fprintf(stderr, "%s arsiv dosyasina yazilamadi!\n", output);
        fclose(out);
        free(org.data);
        return 1;
    }

    /* Dosya iceriklerini ardisik olarak ekle */
    char io_buf[IO_BUFFER];
    for (int i = 0; i < file_count; i++) {
        FILE *in = fopen(files[i], "rb");
        if (!in) {
            fprintf(stderr, "%s acilamadi: %s\n", files[i], strerror(errno));
            fclose(out);
            free(org.data);
            return 1;
        }
        size_t n;
        while ((n = fread(io_buf, 1, sizeof(io_buf), in)) > 0) {
            if (fwrite(io_buf, 1, n, out) != n) {
                fprintf(stderr, "%s yazma hatasi!\n", output);
                fclose(in);
                fclose(out);
                free(org.data);
                return 1;
            }
        }
        fclose(in);
    }

    fclose(out);
    free(org.data);

    printf("Toplam %d dosya birlestirilerek \"%s\" arsiv dosyasi olusturuldu.\n",
           file_count, output);
    return 0;
}

/* ---------------------------------------------------------------------------
 * -a : ACMA
 * ------------------------------------------------------------------------ */

/* Yolu bilesen bilesen olusturur (mkdir -p benzeri). */
static int mkdir_p(const char *path, mode_t mode) {
    if (!path || !*path) return -1;

    /* Yolu kopyala */
    char tmp[MAX_PATH_LEN];
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) return -1;
    memcpy(tmp, path, len + 1);

    /* Bittiyse / kaldir */
    if (len > 1 && tmp[len - 1] == '/') tmp[len - 1] = '\0';

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

static int cmd_extract(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return 1;
    }
    if (argc > 4) {
        fprintf(stderr,
            "-a parametresinden sonra en fazla 2 parametre verilebilir!\n");
        return 1;
    }

    const char *archive = argv[2];
    const char *target  = (argc == 4) ? argv[3] : ".";

    /* .sau uzantisi kontrolu */
    size_t alen = strlen(archive);
    if (alen < 4 || strcmp(archive + alen - 4, ".sau") != 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return 1;
    }

    FILE *in = fopen(archive, "rb");
    if (!in) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        return 1;
    }

    /* Toplam dosya boyutu (bozukluk kontrolu icin) */
    if (fseek(in, 0, SEEK_END) != 0) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }
    long file_total = ftell(in);
    rewind(in);

    if (file_total < HEADER_PREFIX) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }

    /* Ilk 10 bayt -> first section size */
    char prefix[HEADER_PREFIX + 1];
    if (fread(prefix, 1, HEADER_PREFIX, in) != HEADER_PREFIX) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }
    prefix[HEADER_PREFIX] = '\0';
    for (int i = 0; i < HEADER_PREFIX; i++) {
        if (!isdigit((unsigned char)prefix[i])) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            fclose(in);
            return 1;
        }
    }
    long first_section_size = atol(prefix);
    long org_len = first_section_size - HEADER_PREFIX;
    if (org_len <= 0 || first_section_size > file_total) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(in);
        return 1;
    }

    char *org = (char *)malloc((size_t)org_len + 1);
    if (!org) {
        fprintf(stderr, "Bellek yetersiz!\n");
        fclose(in);
        return 1;
    }
    if ((long)fread(org, 1, (size_t)org_len, in) != org_len) {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        free(org);
        fclose(in);
        return 1;
    }
    org[org_len] = '\0';

    /* Ilk karakter '|' olmalidir */
    if (org[0] != '|') {
        fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
        free(org);
        fclose(in);
        return 1;
    }

    /* Hedef dizini olustur (girilen dizin adinda bosluk yoksa). */
    if (strcmp(target, ".") != 0) {
        if (strchr(target, ' ') == NULL) {
            if (mkdir_p(target, 0755) != 0) {
                fprintf(stderr, "%s dizini olusturulamadi: %s\n",
                        target, strerror(errno));
                free(org);
                fclose(in);
                return 1;
            }
        } else {
            fprintf(stderr,
                "Dizin adinda bosluk olmamalidir: \"%s\"\n", target);
            free(org);
            fclose(in);
            return 1;
        }
    }

    /* Kayitlari ayristir ve dosyalari ac */
    char *p = org;
    int extracted = 0;
    while (*p) {
        if (*p != '|') {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org);
            fclose(in);
            return 1;
        }
        p++; /* | atla */

        char *next = strchr(p, '|');
        size_t reclen = next ? (size_t)(next - p) : strlen(p);

        char rec[MAX_PATH_LEN + 64];
        if (reclen >= sizeof(rec)) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org);
            fclose(in);
            return 1;
        }
        memcpy(rec, p, reclen);
        rec[reclen] = '\0';

        char *c1 = strchr(rec, ',');
        if (!c1) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org); fclose(in); return 1;
        }
        *c1 = '\0';
        char *c2 = strchr(c1 + 1, ',');
        if (!c2) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org); fclose(in); return 1;
        }
        *c2 = '\0';

        const char *fname  = rec;
        const char *permstr = c1 + 1;
        const char *sizestr = c2 + 1;

        /* izin sekizlik tabanda */
        char *endp = NULL;
        long perm = strtol(permstr, &endp, 8);
        if (!endp || *endp != '\0' || perm < 0 || perm > 0777) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org); fclose(in); return 1;
        }
        long fsize = strtol(sizestr, &endp, 10);
        if (!endp || *endp != '\0' || fsize < 0) {
            fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(org); fclose(in); return 1;
        }

        /* Cikis yolu */
        char outpath[MAX_PATH_LEN];
        const char *bn = path_basename(fname); /* arsivde sadece basename var ama yine de guvende olalim */
        int n = snprintf(outpath, sizeof(outpath), "%s/%s", target, bn);
        if (n < 0 || (size_t)n >= sizeof(outpath)) {
            fprintf(stderr, "Cikis yolu cok uzun: %s/%s\n", target, bn);
            free(org); fclose(in); return 1;
        }

        FILE *out = fopen(outpath, "wb");
        if (!out) {
            fprintf(stderr, "%s olusturulamadi: %s\n", outpath, strerror(errno));
            free(org); fclose(in); return 1;
        }

        long remaining = fsize;
        char io_buf[IO_BUFFER];
        while (remaining > 0) {
            size_t want = (remaining > (long)sizeof(io_buf))
                            ? sizeof(io_buf) : (size_t)remaining;
            size_t got = fread(io_buf, 1, want, in);
            if (got == 0) {
                fprintf(stderr, "Arsiv dosyasi uygunsuz veya bozuk!\n");
                fclose(out); free(org); fclose(in); return 1;
            }
            if (fwrite(io_buf, 1, got, out) != got) {
                fprintf(stderr, "%s yazma hatasi!\n", outpath);
                fclose(out); free(org); fclose(in); return 1;
            }
            remaining -= (long)got;
        }
        fclose(out);

        if (chmod(outpath, (mode_t)perm) != 0) {
            /* Izin atanamadi - kritik degil ama uyaralim. */
            fprintf(stderr,
                "Uyari: %s icin izin atanamadi (%s)\n",
                outpath, strerror(errno));
        }

        extracted++;
        if (!next) break;
        p = next;
    }

    free(org);
    fclose(in);

    printf("\"%s\" arsivi \"%s\" dizinine acildi (%d dosya).\n",
           archive, target, extracted);
    return 0;
}

/* ---------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        return cmd_archive(argc, argv);
    } else if (strcmp(argv[1], "-a") == 0) {
        return cmd_extract(argc, argv);
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    print_usage(argv[0]);
    return 1;
}
