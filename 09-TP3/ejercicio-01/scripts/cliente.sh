#!/bin/sh
# Corre el cliente (en mqttpub), con el eco ya corriendo en mqttsub.
#   sh ~/ejercicio-01/scripts/cliente.sh c         [-h broker] [-p puerto] [-q qos] [-n reps] [-e emin] [-E emax]
#   sh ~/ejercicio-01/scripts/cliente.sh python    [-b broker] [-p puerto] [-q qos] [-n reps] [-e emin] [-E emax]
# El CSV queda en ~/ejercicio-01/resultados/ (carpeta compartida con la maquina propia);
# la tabla de promedios sale por pantalla.
lang="$1"
[ $# -gt 0 ] && shift
dir="$(cd "$(dirname "$0")/.." && pwd)"
out="$dir/resultados/resultados_cliente_${lang}_$(date +%Y%m%d_%H%M%S).csv"
case "$lang" in
  c)      /tmp/pp_client "$@" > "$out" ;;
  python) python3 "$dir/fuentes/python/pp_client.py" "$@" > "$out" ;;
  *)      echo "uso: sh cliente.sh c|python [opciones]"; exit 1 ;;
esac
rc=$?
echo "CSV: $out (salida $rc)"
exit $rc
