# Ping-pong MQTT (ejercicios 1 y 2)

Mismo experimento que el ej. 3 del TP2, con MQTT. Hay dos versiones equivalentes: C (libmosquitto) y Python (paho).

| VM | IP | Programa |
|---|---|---|
| mqttbroker | 192.168.56.10 | Mosquitto |
| mqttpub | 192.168.56.11 | cliente: publica y mide |
| mqttsub | 192.168.56.12 | eco: devuelve lo que recibe |

## Cómo funciona

- El cliente publica B bytes en `pingpong/ping`; el eco los republica sin tocarlos en `pingpong/pong`.
- El cliente mide el RTT: `t0` justo antes de publicar y `t1` al entrar al callback del pong, los dos con el reloj local de `mqttpub` (CLOCK_MONOTONIC / perf_counter). No hace falta sincronizar relojes.
- RTT/2 se reporta igual que en el TP2. Con broker, es el tiempo de un sentido pasando por el broker.
- Tamaños 10^1 a 10^6 bytes, 20 repeticiones, patrón `i % 256` verificado en el cliente (columna `ok`).
- **Una sola conexión** para todo el experimento. No se reconecta entre repeticiones.
- QoS 0 por defecto, igual en publicación y suscripción (`-q` lo cambia).

### Sincronización de arranque (equivale al reintento de `connect()` del TP2)

Publicar antes de que el eco esté suscripto pierde el mensaje. El eco publica un `pingpong/ready` **retenido** cuando ya está suscripto, y el cliente espera ese mensaje antes de medir. Así no importa quién arranque primero, y la espera queda fuera de la medición. Si el eco muere sin avisar, su last will borra el `ready`.

## Uso

Los archivos hay que dejarlos visibles dentro de las VMs (esta carpeta no está en las carpetas compartidas del Vagrantfile; copiarla o agregarla al `synced_folder`).

**C** (en `mqttpub` y `mqttsub`):

```bash
cd /tmp && cp ~/TP-3/c/* . && sh tocomp pp_client && sh tocomp pp_echo
```

```bash
# mqttsub
/tmp/pp_echo
# mqttpub
/tmp/pp_client > resultados_c.csv
```

**Python**:

```bash
# mqttsub
python3 ~/TP-3/python/pp_echo.py
# mqttpub
python3 ~/TP-3/python/pp_client.py > resultados_py.csv
```

Opciones del cliente: `-h/-b` broker, `-p` puerto, `-q` QoS, `-n` repeticiones, `-e`/`-E` exponente mínimo y máximo. El CSV sale por stdout (`exp,bytes,rep,rtt_ms,rtt2_ms,ok`) y la tabla resumen por stderr.

### Cosas a tener en cuenta

- **Un solo eco a la vez.** Si hay dos, cada ping recibe dos pongs y los tiempos quedan mal sin que `ok` lo note. El cliente lo detecta (cuenta pings y pongs) y termina con error; en ese caso se descartan los resultados.
- **La primera repetición es más lenta** (en las pruebas 30 a 45 ms contra 3 a 4 ms para 10 bytes): es el arranque en frío de la conexión única.
- Para frenar el eco usar Ctrl+C en una terminal; así limpia el `ready`. Si se lanza en segundo plano desde un script, el shell puede ignorar SIGINT y el eco Python no lo recibe.

## Diferencias de código respecto al TP2

| TP2 (sockets TCP) | MQTT |
|---|---|
| Bucle `while (recibidos < size) read(...)` | Un solo callback con el mensaje completo; no hay conteo de lecturas |
| El cliente bloquea en `read()` | Hay que esperar el pong explícitamente (variable de condición / `Event`) |
| `ECONNREFUSED`: reintentar `connect()` | Mensaje perdido si no hay suscriptor: handshake con `ready` retenido |
| Dos procesos, una conexión directa | Tres procesos, el broker en el medio: 4 tramos de red por ping-pong |
| Una conexión por repetición | Una conexión para todo (por ahora) |
