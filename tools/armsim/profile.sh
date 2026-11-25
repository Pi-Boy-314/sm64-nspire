#!/bin/sh
# Per-function profile of the ARMSIM benchmark build (make ARMSIM=1). Run from a folder holding
# sm64_config.txt.tns. It runs the benchmark twice, with bench_ticks 100 and 400, and reports
# the difference per tick, so boot, level loading and warmup cancel out. Needs QEMU, glib
# (installed with QEMU) and arm-none-eabi-objdump on PATH. Extra arguments go to profile.py,
# e.g. --annotate rast_fn_6 to list one function's instructions with their counts.
set -e
here=$(cd "$(dirname "$0")" && pwd)
build=$here/../../build/us_nsp_armsim
elf=$build/sm64.us.f3dex2
plugin=$build/libtbprof.so
if [ ! -f "$plugin" ] || [ "$here/plugin/tbprof.c" -nt "$plugin" ]; then
    extra=
    [ "$(uname)" = Darwin ] && extra="-undefined dynamic_lookup"
    cc -shared -fPIC -O2 $(pkg-config --cflags glib-2.0) -I "$(dirname "$(command -v qemu-system-arm)")/../include" \
        -o "$plugin" "$here/plugin/tbprof.c" $(pkg-config --libs glib-2.0) $extra
fi
cp sm64_config.txt.tns sm64_config.orig
for n in 100 400; do
    sed "s/^bench_ticks .*/bench_ticks $n/; s/^bench_dump_frames .*/bench_dump_frames false/" \
        sm64_config.orig > sm64_config.txt.tns
    qemu-system-arm -M versatilepb -cpu arm926 -m 256M \
        -display none -monitor none -serial none -nodefaults -audio none \
        -icount shift=0,align=off,sleep=off \
        -semihosting-config enable=on,target=native \
        -plugin "$plugin,out=tbprof_$n.txt" -kernel "$elf" > /dev/null
done
mv sm64_config.orig sm64_config.txt.tns
python3 "$here/profile.py" "$elf" tbprof_400.txt tbprof_100.txt 300 "$@"
