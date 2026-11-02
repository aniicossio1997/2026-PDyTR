# TP4 — Ejemplo JADE: paso a paso

Dos contenedores JADE en dos máquinas virtuales (Vagrant + VirtualBox), igual que en TP1 y TP3:

| VM | IP | Rol |
|---|---|---|
| `jadea` | 192.168.56.10 | Main-Container |
| `jadeb` | 192.168.56.11 | Container-1 (acá se crea el agente) |

La carpeta `ejemplo/` se comparte en `/home/vagrant/ejemplo` en las dos VMs.

## 0. Requisitos

- Vagrant y VirtualBox instalados.
- La box `bento/ubuntu-22.04` (se descarga sola la primera vez).

## 1. Levantar las VMs

Desde esta carpeta (`ejemplo/`):

```bash
vagrant up
```

La primera vez tarda varios minutos: instala el JDK en cada VM.

## 2. Compilar (en `jadeb`)

```bash
vagrant ssh jadeb
cd ejemplo
mkdir -p classes
javac -classpath lib/jade.jar -d classes myexamples/DemoMovilidad.java
```

Compilar dentro de la VM evita problemas de versión: las VMs tienen Java 11, y una clase compilada con un Java más nuevo en el host no carga en ellas.

## 3. Terminal 1 — Main-Container (en `jadea`)

```bash
vagrant ssh jadea
cd ejemplo
java -cp lib/jade.jar jade.Boot -local-host 192.168.56.10
```

Esperar `Agent container Main-Container@192.168.56.10 is ready.`

- Va **sin** `classes` en el classpath a propósito: si igual ejecuta el agente, es porque el código viajó.
- Va **sin** `-gui`: las VMs no tienen pantalla.

## 4. Terminal 2 — Container-1 con el agente (en `jadeb`)

```bash
vagrant ssh jadeb
cd ejemplo
java -cp lib/jade.jar:classes jade.Boot -container -host 192.168.56.10 -local-host 192.168.56.11 -agents demo:DemoMovilidad
```

## 5. Qué se tiene que ver

En `jadeb` (Container-1):

```
[jadeb | Container-1] setup()  contador=1  local=42  nota=anotado en el origen
[jadeb | Container-1] línea siguiente a doMove()
```

En `jadea` (Main-Container):

```
[jadea | Main-Container] afterMove()  contador=2  nota=null
```

Cada línea muestra la máquina y el contenedor: el agente nació en `jadeb` y terminó en `jadea`.

## 6. Probar una migración que falla

En `myexamples/DemoMovilidad.java`, cambiar `"Main-Container"` por `"Container-9"`, recompilar (paso 2) y repetir el paso 4. El agente se queda en `jadeb`, su código no recibe ningún error, y en el log aparece:

```
SEVERE: Destination Container-9 does not exist or does not support mobility
```

## 7. Cortar y apagar

- `Ctrl+C` en cada terminal corta los contenedores.
- Si al cerrar el Container-1 el Main-Container muestra `EOF reached` o `PING ... exited with exception`, es normal: avisa que el otro contenedor se fue.
- Apagar las VMs: `vagrant halt` · Borrarlas: `vagrant destroy -f`

## Variante: host + VM

El enunciado admite que una de las dos computadoras sea virtual. La Mac o la PC está en la misma red privada que las VMs como `192.168.56.1`:

```bash
# en el host, desde ejemplo/
java -cp lib/jade.jar jade.Boot -gui -local-host 192.168.56.1

# en jadeb
java -cp lib/jade.jar:classes jade.Boot -container -host 192.168.56.1 -local-host 192.168.56.11 -agents demo:DemoMovilidad
```

- En Windows el classpath se separa con `;` (ver `jade.ps1`).
- En macOS puede aparecer un pedido de permiso de **Red local** para Java o para la Terminal: hay que aceptarlo. Si no aparece y el contenedor no logra unirse (`EOF reached`), revisar *Configuración del Sistema → Privacidad y seguridad → Red local*.

### Caso real: un firewall que acepta y corta

Probando esta variante en una Mac con el antivirus ESET, el contenedor de `jadeb` no logró unirse. Los dos lados mostraban esto:

```
# host (Main-Container)
SEVERE: Communication error reading incoming packet from /1.140.37.67:0
java.net.SocketException: Socket is not connected

# jadeb (Container-1)
jade.core.ProfileException: Can't get a proxy to the Platform Manager
  ... ICPException delivering command to address jicp://192.168.56.1:1099. - Caused by:  EOF reached
```

Las pistas:

- No es `Connection refused`: la conexión TCP **se establece** y se corta enseguida. Del lado de `jadeb` eso se ve como `EOF reached`.
- La IP de origen `1.140.37.67:0` no existe en esta red: es basura. El socket ya estaba cortado cuando JADE quiso averiguar quién se conectaba.

Para aislar el problema, se sacó JADE de la ecuación y se probó con conexiones TCP simples entre `jadeb` y el host:

| Prueba | Resultado |
|---|---|
| Host → `jadeb` con Java (conectarse al puerto 22) | Anda |
| `jadeb` → host, escuchando con `nc -l` (binario de Apple) | Anda |
| `jadeb` → host, escuchando con un `ServerSocket` de Java | Se acepta y se corta: `Broken pipe` |
| Lo mismo con el firewall de ESET desactivado | Anda |

Conclusión: el firewall de ESET deja pasar las conexiones entrantes a los programas firmados por Apple y se las corta a un `java` que no conoce. El firewall propio de macOS estaba apagado; el que bloqueaba era el del antivirus. El mismo síntoma aparece con otros antivirus o con el Firewall de Windows.

Solución: en el firewall, agregar la red `192.168.56.0/24` como **red de confianza** (o, solo para la práctica, desactivarlo).

¿Y si se invierten los roles: Main-Container en la VM y contenedor en el host? Tampoco anda. La comunicación entre contenedores va en los dos sentidos. El contenedor le informa al Main su propia dirección (`jicp://192.168.56.1:1099`), y el Main se conecta ahí para mandarle comandos. Además, cuando el agente migra, el destino le pide el código de la clase al contenedor de origen. Cualquiera de las dos máquinas tiene que poder recibir conexiones de la otra.

## Problemas frecuentes

| Síntoma | Causa probable | Solución |
|---|---|---|
| `Node <Container-1> ... is not reachable` | El contenedor anuncia una IP que el otro no alcanza (por ejemplo la 10.0.2.15 de NAT de Vagrant) | Pasar `-local-host` con la IP 192.168.56.x de esa máquina |
| `Connection refused` | El Main-Container no está corriendo, o `-host` apunta a otra IP | Revisar el paso 3 y la IP |
| `EOF reached` en el contenedor y `Socket is not connected` con una IP rara en el Main | Un firewall (antivirus, Firewall de Windows) acepta la conexión y la corta | Ver *Caso real: un firewall que acepta y corta* |
| `directory not found: classes` | La carpeta no existe | `mkdir -p classes` antes de compilar |
| `UnsupportedClassVersionError` | Se compiló con un Java más nuevo que el que ejecuta | Compilar dentro de la VM (paso 2) |
| `HeadlessException` | Se usó `-gui` dentro de una VM sin pantalla | Sacar `-gui` en las VMs |
