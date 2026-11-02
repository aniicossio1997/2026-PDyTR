# TP4 — Ejercicio 1

> Enunciado: en una plataforma JADE de dos contenedores (Main-Container y
> Container-1), programar un agente `AgenteA` que cree un agente `AgenteB` y
> lo migre al otro contenedor. `AgenteB` debe mostrar que se completó la
> migración. Correrlo en dos computadoras distintas (una puede ser virtual) y
> explicar por qué hace falta movilidad de código.

## Diseño

| VM | IP | Rol |
|---|---|---|
| `e1a` | 192.168.56.20 | Main-Container |
| `e1b` | 192.168.56.21 | Container-1 (acá nacen `AgenteA` y `AgenteB`) |

- **`AgenteA`** ([myexamples/AgenteA.java](myexamples/AgenteA.java)): al arrancar (`setup()`), crea a
  `AgenteB` en su mismo contenedor usando `getContainerController().createNewAgent(...)`
  y le pasa como argumento el nombre del contenedor destino (`"Main-Container"`).
  `AgenteA` no migra: solo dispara la creación y queda en Container-1.
- **`AgenteB`** ([myexamples/AgenteB.java](myexamples/AgenteB.java)): en `setup()` lee ese argumento y
  llama `doMove(new ContainerID(destino, null))`. JADE lo migra al terminar
  `setup()`. En `afterMove()` (ya en el destino) imprime que la migración se
  completó, con el contenedor y el host donde está corriendo.

### ¿Por qué hace falta movilidad de código?

`AgenteB` se define una sola vez, en `Container-1` (en la VM `e1b`). El
`Main-Container` (VM `e1a`) **nunca tiene el `.class` de `AgenteB`** — de
hecho el target `main` del Makefile arranca el contenedor sin el classpath
`classes`, a propósito. Si igual el agente aparece corriendo ahí, con su
`afterMove()` ejecutándose, es porque `doMove()` no mandó solo un mensaje:
mandó el bytecode de la clase más el estado de sus atributos, y la JVM del
Main-Container lo cargó dinámicamente para poder ejecutarlo. Sin movilidad de
código, el Main-Container no tendría forma de "saber" qué es un `AgenteB` ni
cómo ejecutarlo — habría que instalar/copiar esa clase ahí de antemano.

### Sobre los *behaviours*

No se usan. El enunciado pide una única acción secuencial y disparada una
sola vez (crear → migrar → avisar), sin esperar mensajes, sin reintentos ni
nada que se repita en el tiempo — todo entra naturalmente en `setup()` /
`afterMove()`, que JADE ya llama en el momento correcto. Un `Behaviour`
(simple o cíclico) agregaría una capa de planificación cooperativa que acá no
resuelve nada: no hay nada que deba convivir con otras tareas del agente ni
que deba quedar esperando una condición. Tendría sentido recién si, por
ejemplo, `AgenteA` tuviera que esperar una confirmación por mensaje ACL de
que la migración realmente terminó (ver ejercicio 2).

## Cómo correrlo

### 1. Levantar las VMs

Desde esta carpeta (`T4-E1/`):

```bash
vagrant up
```

### 2. Compilar (en `e1b`)

```bash
vagrant ssh e1b
cd T4-E1
mkdir -p classes
javac -classpath lib/jade.jar -d classes myexamples/AgenteA.java myexamples/AgenteB.java
```

### 3. Terminal 1 — Main-Container (en `e1a`)

```bash
vagrant ssh e1a
cd T4-E1
java -cp lib/jade.jar jade.Boot -local-host 192.168.56.20
```

Esperar `Agent container Main-Container@192.168.56.20 is ready.`

- Va **sin** `classes` en el classpath a propósito (ver explicación arriba).
- Va **sin** `-gui`: las VMs no tienen pantalla.

### 4. Terminal 2 — Container-1 con AgenteA (en `e1b`)

```bash
vagrant ssh e1b
cd T4-E1
java -cp lib/jade.jar:classes jade.Boot -container -host 192.168.56.20 -local-host 192.168.56.21 -agents agenteA:AgenteA\(Main-Container\)
```

(En Windows, usando `jade.ps1`: `.\jade.ps1 compile`, luego `.\jade.ps1 main`
en una terminal y `.\jade.ps1 container` en otra — ver la variante host+VM
más abajo.)

### 5. Qué se tiene que ver

En `e1b` (Container-1):

```
[e1b | Container-1] nazco en Container-1  ->  voy a crear a AgenteB y mandarlo a Main-Container
[e1b | Container-1] AgenteB creado y arrancado
[e1b | Container-1 | AgenteB] setup()  nazco en Container-1  pido migrar a Main-Container
```

En `e1a` (Main-Container):

```
[e1a | Main-Container | AgenteB] afterMove()  migración completada: ahora estoy en Main-Container
```

El agente nació en `e1b` y terminó ejecutándose en `e1a`, en una JVM que
nunca tuvo el `.class` de `AgenteB` precompilado.

### 6. Cortar y apagar

- `Ctrl+C` en cada terminal corta los contenedores.
- `vagrant halt` apaga las VMs, `vagrant destroy -f` las borra.

## Variante: host + VM

Igual que en el ejemplo de cátedra, una de las dos "computadoras" puede ser
el host (Windows/Mac) y la otra una VM. Revisar la sección correspondiente en
[`../ejemplo/README.md`](../ejemplo/README.md) para la IP `192.168.56.1` del
host, el tema de classpath con `;` en Windows (ya resuelto en `jade.ps1`) y
los problemas de firewall/antivirus documentados ahí (aplican igual acá).

## Problemas frecuentes

Los mismos que en `ejemplo/` (ver su README): `Node ... is not reachable`,
`Connection refused`, `directory not found: classes`,
`UnsupportedClassVersionError`, `HeadlessException`. Soluciones idénticas.
