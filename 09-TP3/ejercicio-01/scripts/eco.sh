#!/bin/sh
# Arranca el eco (en mqttsub). Queda esperando; cortar con Ctrl+C.
#   sh ~/ejercicio-01/scripts/eco.sh c         [-h broker] [-p puerto] [-q qos]
#   sh ~/ejercicio-01/scripts/eco.sh python    [-b broker] [-p puerto] [-q qos]
lang="$1"
[ $# -gt 0 ] && shift
dir="$(cd "$(dirname "$0")/.." && pwd)"
case "$lang" in
  c)      exec /tmp/pp_echo "$@" ;;
  python) exec python3 "$dir/fuentes/python/pp_echo.py" "$@" ;;
  *)      echo "uso: sh eco.sh c|python [opciones]"; exit 1 ;;
esac
