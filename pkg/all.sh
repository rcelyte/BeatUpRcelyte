#!/bin/sh
cp PKGBUILD .PKGBUILD
mkdir -p src/BeatUpRcelyte/pkg/
cp -r ../makefile ../*.c ../src/ ../mbedtls/ src/BeatUpRcelyte/
cp beatupserver.* src/BeatUpRcelyte/pkg/
makepkg -cefsp .PKGBUILD
rm .PKGBUILD
