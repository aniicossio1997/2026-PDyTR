# Ejercicio 1: paso a paso

Cómo levantar las tres VMs y ejecutar el ping-pong MQTT, en C y en Python. Qué se hizo y por qué es comparable con el TP2 está en `INFORME.md`.

```
ejercicio-01/
├── PASOS.md              este archivo
├── INFORME.md            diseño, diferencias de código y comparabilidad
├── Vagrantfile           3 VMs
├── fuentes/
│   ├── c/                pp_client.c, pp_echo.c
│   └── python/           pp_client.py, pp_echo.py
├── scripts/              compilar.sh, eco.sh, cliente.sh
└── resultados/           acá quedan los CSV
```

## 0. Requisitos

| Herramienta | Versión | Comprobar con |
|---|---|---|
| VirtualBox | 7.1 o superior | `VBoxManage --version` |
| Vagrant | 2.4 o superior | `vagrant --version` |
| RAM libre | unos 6 GB (tres VMs de 2 GB) | |

Si las VMs no arrancan o andan muy lentas, Hyper-V está ocupando la virtualización; ver *Problemas frecuentes* de `../MQTT/PASO-A-PASO.md`.

## 1. Levantar las VMs

Desde la carpeta `ejercicio-01`.

En PowerShell o CMD:

```powershell
vagrant up
vagrant status
```

La primera vez descarga la box `bento/ubuntu-22.04` e instala los paquetes: lleva varios minutos. Resultado esperado de `vagrant status`:

```
mqttbroker                running (virtualbox)
mqttpub                   running (virtualbox)
mqttsub                   running (virtualbox)
```

| VM | IP | Rol |
|---|---|---|
| mqttbroker | 192.168.56.10 | broker Mosquitto, puerto 1883 |
| mqttpub | 192.168.56.11 | cliente: publica y mide |
| mqttsub | 192.168.56.12 | eco: devuelve lo que recibe |

En `mqttpub` y `mqttsub` esta carpeta está compartida como `~/ejercicio-01`: lo que se edita afuera se ve adentro, y los CSV que genera el cliente aparecen en `resultados/` de la máquina propia.

Si una VM falla con *Timed out while waiting for the machine to boot*, suele ser lentitud al arrancar: repetir `vagrant up NOMBRE`.

## 2. Abrir tres terminales

Todas desde la carpeta `ejercicio-01`:

| Terminal | Comando | Rol |
|---|---|---|
| A | `vagrant ssh mqttbroker` | broker |
| B | `vagrant ssh mqttsub` | eco |
| C | `vagrant ssh mqttpub` | cliente |

A partir de acá los comandos se ejecutan **dentro de las VMs**.

## 3. Comprobar broker y red

**Terminal A:**

```bash
systemctl is-active mosquitto
ss -ltn | grep 1883
```

Resultado esperado: `active` y una línea con `0.0.0.0:1883`.

**Terminal C:**

```bash
ping -c 2 192.168.56.10
```

## 4. Versión en C

**Terminales B y C:** compilar (se hace en `/tmp`; se borra al reiniciar la VM).

```bash
sh ~/ejercicio-01/scripts/compilar.sh
```

Resultado esperado: `compilado en /tmp: pp_client pp_echo`.

**Terminal B:** arrancar el eco. Queda esperando.

```bash
sh ~/ejercicio-01/scripts/eco.sh c
```

Resultado esperado: `eco listo (QoS 0)`.

**Terminal C:** primero una prueba corta (3 repeticiones, tamaños 10 a 1000 bytes) y después la corrida completa.

```bash
sh ~/ejercicio-01/scripts/cliente.sh c -n 3 -E 3
sh ~/ejercicio-01/scripts/cliente.sh c
```

Cada corrida imprime la tabla de promedios y guarda el CSV en `~/ejercicio-01/resultados/resultados_cliente_c_FECHA.csv`, con las columnas `exp,bytes,rep,rtt_ms,rtt2_ms,ok`.

Cortar el eco con `Ctrl+C` en la terminal B.

## 5. Versión en Python

Se usa el eco de Python con el cliente de Python.

**Terminal B:**

```bash
sh ~/ejercicio-01/scripts/eco.sh python
```

**Terminal C:**

```bash
sh ~/ejercicio-01/scripts/cliente.sh python -n 3 -E 3
sh ~/ejercicio-01/scripts/cliente.sh python
```

Cortar el eco con `Ctrl+C`. Los dos lenguajes hablan el mismo protocolo, así que también se pueden cruzar (cliente en C con eco en Python, y al revés).

## 6. Opciones

| Opción | Qué hace | Por defecto |
|---|---|---|
| `-h` (C) / `-b` (Python) | IP del broker | 192.168.56.10 |
| `-p` | puerto | 1883 |
| `-q` | QoS 0, 1 o 2 | 0 |
| `-n` | repeticiones por tamaño (solo cliente) | 20 |
| `-e` / `-E` | exponente mínimo y máximo, 10^e bytes (solo cliente) | 1 y 6 |

El eco acepta `-h`/`-b`, `-p` y `-q`. Si se usa un QoS, conviene el mismo en el eco y en el cliente.

## 7. Qué observar

- **Columna `ok`:** debe ser 1 en todas las filas.
- **Orden de arranque:** da igual quién arranque primero. Si el cliente va antes, imprime `eco no disponible, esperando...` hasta que el eco avisa. Probarlo es una buena forma de ver el `ready` retenido en acción.
- **Last will:** matar el eco con `kill -9` también borra el `ready`, porque el broker publica el will. Se ve desde la terminal C con `mosquitto_sub -h 192.168.56.10 -t pingpong/ready -v -W 3`: sin eco no aparece nada.
- **Un solo eco a la vez.** Con dos ecos cada ping recibe dos pongs; el cliente lo detecta y termina con `ERROR: N pongs para M pings`.
- **La primera repetición es más lenta** (arranque en frío de la conexión única).
- **Ver el tráfico:** en la terminal A, `mosquitto_sub -h 192.168.56.10 -t 'pingpong/#' -v | head` muestra los mensajes que pasan por el broker.

## 8. Apagar

Salir de las VMs con `exit` y, desde la carpeta `ejercicio-01`:

```bash
vagrant halt        # apaga; la próxima vez arranca en segundos
vagrant destroy -f  # borra las VMs
```

## Problemas frecuentes

| Síntoma | Causa y solución |
|---|---|
| `A Vagrant environment or target machine is required` | El comando se corrió fuera de `ejercicio-01`. |
| Ya existen VMs `mqttbroker`, `mqttpub` o `mqttsub` de `../MQTT/MQTT-Gral` | Los nombres coinciden. Apagarlas o borrarlas desde esa carpeta (`vagrant destroy -f`) antes de levantar estas. |
| `Connection refused` / `no se pudo conectar` | Broker apagado (`sudo systemctl restart mosquitto` en la terminal A), IP mal puesta, o falta alguna VM (`vagrant status`). |
| `eco no disponible, esperando...` sin fin | El eco no está corriendo o no llegó a suscribirse; revisar la terminal B. |
| `ERROR: N pongs para M pings` | Hay más de un eco. Detener los extras (`pkill -x pp_echo`, `pkill -f pp_echo.py`) y repetir. |
| `ERROR: timeout esperando el pong` | El eco se cayó o se perdió un mensaje. Reiniciar el eco y repetir. |
| `/tmp/pp_client: No such file or directory` | Falta compilar (paso 4); `/tmp` se borra al reiniciar la VM. |
| `\r: command not found` / `$'\r'` | El archivo se editó en Windows con fin de línea CRLF. Dentro de la VM: `dos2unix ARCHIVO`. |
| El eco de Python no se detiene con `Ctrl+C` | Pasa si se lanzó en segundo plano desde un script. Correrlo en primer plano. |
