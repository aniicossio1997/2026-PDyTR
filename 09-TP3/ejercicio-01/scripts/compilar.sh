#!/bin/sh
# Compila pp_client y pp_echo (version en C) en /tmp, para no dejar binarios
# en la carpeta compartida. Correr en mqttpub y en mqttsub:
#   sh ~/ejercicio-01/scripts/compilar.sh
# /tmp se borra al reiniciar la VM: hay que repetirlo despues de un vagrant halt.
cd "$(dirname "$0")/../fuentes/c" || exit 1
for p in pp_client pp_echo; do
  gcc "$p.c" -o "/tmp/$p" -lmosquitto -lpthread || exit 1
done
echo "compilado en /tmp: pp_client pp_echo"
