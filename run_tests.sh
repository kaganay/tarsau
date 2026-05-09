#!/usr/bin/env bash
# tarsau icin otomatik testler
# Kullanim: bash run_tests.sh
set -u

PASS=0
FAIL=0
TARSAU=./tarsau

note() { printf "\n=== %s ===\n" "$1"; }
ok()   { printf "  [PASS] %s\n" "$1"; PASS=$((PASS+1)); }
nok()  { printf "  [FAIL] %s\n" "$1"; FAIL=$((FAIL+1)); }

if [ ! -x "$TARSAU" ]; then
    echo "Once 'make' calistirin."
    exit 1
fi

# Temizle
rm -rf out1 out2 out3 demo.sau a.sau bad.sau bin.bin
mkdir -p test_files

# 1) Temel arsivleme + acma + icerik dogrulamasi
note "Test 1: temel arsivleme ve acma"
$TARSAU -b test_files/t1.txt test_files/t2.txt test_files/t3.txt \
        test_files/t4.txt test_files/t5.dat -o demo.sau >/dev/null
[ -f demo.sau ] && ok "demo.sau olusturuldu" || nok "demo.sau olusturulmadi"

$TARSAU -a demo.sau out1 >/dev/null
for f in t1.txt t2.txt t3.txt t4.txt t5.dat; do
    if cmp -s "test_files/$f" "out1/$f"; then
        ok "$f icerigi ayni"
    else
        nok "$f icerigi farkli"
    fi
done

# 2) Izinlerin korunmasi (Windows DrvFs mount'u her zaman 777 raporlar.
#    Bu yuzden testi gercek Linux dosya sisteminde yapiyoruz.)
note "Test 2: izinler korunuyor mu?"
TMPDIR=$(mktemp -d)
cp "$TARSAU" "$TMPDIR/tarsau"
cp test_files/t1.txt test_files/t2.txt "$TMPDIR/"
(
    cd "$TMPDIR"
    chmod 640 t1.txt
    chmod 600 t2.txt
    ./tarsau -b t1.txt t2.txt -o demo.sau >/dev/null
    rm -rf out
    ./tarsau -a demo.sau out >/dev/null
    p1=$(stat -c '%a' out/t1.txt)
    p2=$(stat -c '%a' out/t2.txt)
    echo "$p1 $p2"
) > "$TMPDIR/perm_result"
read p1 p2 < "$TMPDIR/perm_result"
[ "$p1" = "640" ] && ok "t1.txt izinleri 640" || nok "t1.txt izinleri $p1"
[ "$p2" = "600" ] && ok "t2.txt izinleri 600" || nok "t2.txt izinleri $p2"
rm -rf "$TMPDIR"

# 3) Varsayilan a.sau
note "Test 3: arsiv adi belirtilmediginde a.sau"
rm -f a.sau
$TARSAU -b test_files/t1.txt >/dev/null
[ -f a.sau ] && ok "a.sau olusturuldu" || nok "a.sau olusturulmadi"
rm -f a.sau

# 4) Metin disi (binary) dosya tespiti
note "Test 4: ikili (binary) dosya reddi"
printf 'binary\x00\x01\x02data' > test_files/binary.bin
out=$($TARSAU -b test_files/t1.txt test_files/binary.bin -o demo.sau 2>&1)
echo "$out" | grep -q "uyumsuzdur" && ok "binary dosya reddedildi" || nok "binary dosya kabul edildi: $out"
rm -f test_files/binary.bin

# 5) Bozuk arsiv reddi
note "Test 5: bozuk arsiv reddi"
echo "bu gecersiz bir arsivdir" > bad.sau
out=$($TARSAU -a bad.sau out3 2>&1)
echo "$out" | grep -q "uygunsuz veya bozuk" && ok "bozuk arsiv reddedildi" || nok "bozuk arsiv kabul edildi: $out"

# 6) Yanlis uzantili arsiv reddi
note "Test 6: yanlis uzantili arsiv reddi"
out=$($TARSAU -a demo.txt out3 2>&1)
echo "$out" | grep -q "uygunsuz veya bozuk" && ok ".txt arsiv reddedildi" || nok ".txt arsiv kabul edildi"

# 7) Bos hedef = mevcut dizin
note "Test 7: hedef dizin verilmedi (mevcut dizin)"
mkdir -p out_inplace && cd out_inplace
cp ../demo.sau .
../$TARSAU -a demo.sau >/dev/null
[ -f t1.txt ] && ok "mevcut dizine acildi" || nok "mevcut dizine acilamadi"
cd ..

# 8) 33 dosya verince hata
note "Test 8: 32 dosya siniri"
mkdir -p test_files/many
for i in $(seq 1 33); do echo "f$i" > "test_files/many/f$i.txt"; done
out=$($TARSAU -b test_files/many/f*.txt -o demo.sau 2>&1)
echo "$out" | grep -q "32" && ok "32+ dosya reddedildi" || nok "32+ dosya kabul edildi: $out"
rm -rf test_files/many

# Ozet
echo
echo "==============================="
echo "  PASS: $PASS    FAIL: $FAIL"
echo "==============================="
[ "$FAIL" -eq 0 ] || exit 1
