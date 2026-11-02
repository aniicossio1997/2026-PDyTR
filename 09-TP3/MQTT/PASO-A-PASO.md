# Laboratorio MQTT: paso a paso

Guía para levantar el laboratorio de la Práctica 3 y comprobar que todo funciona: broker, clientes de línea de comandos, ejemplos en C y en Python, comodines, QoS, retained, sesión persistente y last will.

```
TP3/
├── Expl3-PubSub-MQTT.pdf       explicación
├── EnuncTP3/                   enunciado de la Práctica 3
└── MQTT/
    ├── PASO-A-PASO.md          este archivo
    ├── 00-Mosq/                ejemplo en C (mqttpub.c, mqttsub.c, tocomp)
    ├── 01-Python/              mqtt1 (valores fijos) y mqtt2 (parametrizado)
    └── MQTT-Gral/
        ├── macos/Vagrantfile
        └── windows/Vagrantfile
```

## 0. Requisitos

| Herramienta | Versión | Comprobar con |
|---|---|---|
| VirtualBox | 7.1 o superior (Apple Silicon necesita 7.1+) | `VBoxManage --version` |
| Vagrant | 2.4 o superior | `vagrant --version` |
| RAM libre | unos 3 GB (tres VMs de 1 GB) | |
| Disco libre | unos 6 GB | |

Instaladores: <https://www.virtualbox.org/wiki/Downloads> y <https://developer.hashicorp.com/vagrant/install>.

**macOS:** al instalar VirtualBox, aceptar la extensión en *Configuración del Sistema > Privacidad y seguridad*. El mismo Vagrantfile sirve para Mac con Apple Silicon (M1, M2…) y para Mac Intel.

**Windows:** VirtualBox choca con Hyper-V. Si las VMs no arrancan o andan muy lentas, ver [Problemas frecuentes](#problemas-frecuentes). En Windows, `VBoxManage` puede no estar en el PATH; en ese caso la versión se ve en *Ayuda > Acerca de* de VirtualBox.

## 1. Levantar las VMs

Descomprimir el zip y entrar a la carpeta del sistema operativo correspondiente.

**macOS** (Terminal):

```bash
cd TP3/MQTT/MQTT-Gral/macos
vagrant up
```

**Windows** (PowerShell o CMD):

```powershell
cd TP3\MQTT\MQTT-Gral\windows
vagrant up
```

La primera vez descarga la box `bento/ubuntu-22.04` e instala los paquetes, lo que lleva varios minutos. Después arranca en segundos.

```bash
vagrant status
```

Resultado esperado:

```
mqttbroker                running (virtualbox)
mqttpub                   running (virtualbox)
mqttsub                   running (virtualbox)
```

| VM | IP | Rol |
|---|---|---|
| mqttbroker | 192.168.56.10 | broker Mosquitto, puerto 1883 |
| mqttpub | 192.168.56.11 | cliente publicador |
| mqttsub | 192.168.56.12 | cliente suscriptor |

En `mqttpub` y `mqttsub` las carpetas `~/00-Mosq` y `~/01-Python` son las mismas de la máquina propia: lo que se edita afuera se ve adentro.

## 2. Abrir tres terminales

Todas desde la misma carpeta del paso 1 (`MQTT-Gral/macos` o `MQTT-Gral/windows`):

| Terminal | Comando |
|---|---|
| A | `vagrant ssh mqttbroker` |
| B | `vagrant ssh mqttsub` |
| C | `vagrant ssh mqttpub` |

A partir de acá todos los comandos se ejecutan **dentro de las VMs** (Ubuntu), sin importar el sistema operativo de la máquina propia.

## 3. Broker y red

**Terminal A (broker):**

```bash
systemctl is-active mosquitto
ss -ltn | grep 1883
```

Resultado esperado: `active` y una línea con `0.0.0.0:1883`. Si escuchara solo en `127.0.0.1`, los clientes de las otras VMs no podrían conectarse.

**Terminal C (pub):**

```bash
ping -c 2 192.168.56.10
```

## 4. Clientes de línea de comandos

**Terminal B (sub):** queda esperando.

```bash
mosquitto_sub -h 192.168.56.10 -t test00 -v
```

**Terminal C (pub):**

```bash
mosquitto_pub -h 192.168.56.10 -t test00 -m hola
```

Resultado esperado en B: `test00 hola`

Cortar el suscriptor con `Ctrl+C`.

## 5. Ejemplo en C

Compilar en las dos VMs cliente (B y C). Se compila en `/tmp` para no dejar binarios en la carpeta compartida:

```bash
cd /tmp && cp ~/00-Mosq/* . && sh tocomp mqttpub && sh tocomp mqttsub
```

**Terminal B:**

```bash
/tmp/mqttsub
```

**Terminal C:**

```bash
/tmp/mqttpub
```

Resultado esperado en B:

```
Topic: test00
Mensaje: hola
```

Para observar: `mqttpub` termina enseguida, mientras `mqttsub` sigue esperando dentro de `mosquitto_loop_forever()`.

## 6. Ejemplo Python mqtt1 (valores fijos)

**Terminal B:**

```bash
python3 ~/01-Python/mqtt1/sub1.py
```

**Terminal C:**

```bash
python3 ~/01-Python/mqtt1/pub1.py
```

Resultado esperado en B: `Topic: test00` / `Mensaje: hola`

**C y Python se entienden:** con `sub1.py` corriendo en B, ejecutar `/tmp/mqttpub` en C. El mensaje llega igual, pero con un carácter invisible al final: `mqttpub.c` publica 5 bytes, `"hola"` más el `\0` de C. El broker transporta bytes, no strings.

## 7. Comodines (mqtt2)

`pub2.py` y `sub2.py` aceptan parámetros; `python3 pub2.py -h` muestra la lista completa.

**Terminal B:** solo las temperaturas de cualquier habitación.

```bash
python3 ~/01-Python/mqtt2/sub2.py -t 'casa/+/temp'
```

**Terminal C:**

```bash
cd ~/01-Python/mqtt2
python3 pub2.py -t casa/cocina/temp -m 21.5
python3 pub2.py -t casa/cocina/hum  -m 60
python3 pub2.py -t casa/living/temp -m 19
```

Resultado esperado en B: llegan `cocina/temp` y `living/temp`, pero no `cocina/hum`.

Repetir con `-t 'casa/#'` en B: llegan los tres.

| Comodín | Significa | Ejemplo |
|---|---|---|
| `+` | exactamente un nivel | `casa/+/temp` |
| `#` | cualquier cantidad de niveles; va al final | `casa/#` |

## 8. QoS

**Terminal B:**

```bash
python3 ~/01-Python/mqtt2/sub2.py -t datos -q 2
```

**Terminal C:**

```bash
python3 ~/01-Python/mqtt2/pub2.py -t datos -m lectura -q 2 -n 5
```

Resultado esperado en B: 5 líneas `[datos] (QoS 2) lectura N`.

El QoS efectivo es el menor entre el de la publicación y el de la suscripción. Para comprobarlo, publicar con `-q 2` y suscribirse con `-q 0`.

## 9. Retained

**Terminal C:** se publica con nadie escuchando.

```bash
python3 ~/01-Python/mqtt2/pub2.py -t casa/living/temp -m 19 -r
```

**Terminal B:** el suscriptor llega tarde.

```bash
python3 ~/01-Python/mqtt2/sub2.py -t 'casa/#'
```

Resultado esperado: `[casa/living/temp] (QoS 0, retained) 19`, sin que nadie haya vuelto a publicar.

Para borrar el retained (mensaje vacío con `-r`):

```bash
mosquitto_pub -h 192.168.56.10 -t casa/living/temp -r -n
```

## 10. Sesión persistente y reinicio del broker (base del ejercicio 4)

**Terminal B:** registrar la sesión y cortar con `Ctrl+C`.

```bash
python3 ~/01-Python/mqtt2/sub2.py -i celular -c -q 1 -t alarma
```

**Terminal C:** publicar mientras el suscriptor no está.

```bash
python3 ~/01-Python/mqtt2/pub2.py -t alarma -m aviso -q 1 -n 3
```

**Terminal A:** reiniciar el broker.

```bash
sudo systemctl restart mosquitto
```

**Terminal B:** volver con el mismo id.

```bash
python3 ~/01-Python/mqtt2/sub2.py -i celular -c -q 1 -t alarma
```

Resultado esperado: llegan los 3 avisos. La sesión sobrevive al reinicio porque el Mosquitto de Ubuntu guarda las sesiones en disco (`persistence true`).

Variantes para el ejercicio 4:
- Sin `-c`: la sesión se borra al desconectarse y los avisos se pierden.
- Con `-q 0` en cualquiera de los dos extremos: tampoco se guardan.

## 11. Last will

**Terminal B:**

```bash
mosquitto_sub -h 192.168.56.10 -t 'estado/#' -v
```

**Terminal C:** un cliente que registra su will y después muere sin avisar.

```bash
mosquitto_sub -h 192.168.56.10 -i sensor1 -t nada --will-topic estado/sensor1 --will-payload caido &
kill -9 %1
```

Resultado esperado en B: `estado/sensor1 caido`, publicado por el broker.

Si en cambio se corta con `Ctrl+C`, el cliente se desconecta de forma ordenada y el will **no** se publica.

## 12. Apagar

Salir de las VMs con `exit` y, desde la carpeta del paso 1:

```bash
vagrant halt        # apaga; la próxima vez arranca en segundos
vagrant destroy -f  # borra las VMs (vagrant up las vuelve a crear desde cero)
```

## Problemas frecuentes

| Síntoma | Causa y solución |
|---|---|
| `vagrant up` falla con VirtualBox | Actualizar a VirtualBox 7.1 o superior. En macOS, aceptar la extensión en *Configuración del Sistema > Privacidad y seguridad*. |
| Windows: `VT-x is not available`, las VMs no arrancan o andan muy lentas | Hyper-V está ocupando la virtualización. Desactivar *Hyper-V*, *Plataforma de máquina virtual* y *Seguridad de Windows > Seguridad del dispositivo > Aislamiento del núcleo > Integridad de memoria*, y reiniciar. Revisar también que la virtualización esté habilitada en el BIOS/UEFI. |
| `A Vagrant environment or target machine is required` | El comando se corrió fuera de la carpeta `MQTT-Gral/macos` o `MQTT-Gral/windows`. |
| Windows: un script falla con `\r: command not found` o `$'\r'` | El archivo se editó en Windows con fin de línea CRLF. Dentro de la VM: `dos2unix ARCHIVO`. Para evitarlo, configurar el editor con fin de línea LF. |
| `Connection refused` desde un cliente | El broker no está activo (`sudo systemctl restart mosquitto`) o falta `listener 1883 0.0.0.0` en `/etc/mosquitto/conf.d/external.conf`. |
| `Fixed port collision for 22` | Hay otras VMs de Vagrant corriendo. Es solo un aviso: Vagrant elige otro puerto. |
| `DeprecationWarning: Callback API version 1` | Aparece con paho 2.x instalado con pip fuera de las VMs. Es un aviso y el programa funciona igual. En las VMs está paho 1.5.1 y no aparece. |
| El suscriptor tardío recibe un valor viejo | Quedó un retained de una prueba anterior. Borrarlo con `mosquitto_pub -h 192.168.56.10 -t TOPICO -r -n`. |
