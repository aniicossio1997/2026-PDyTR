# Experimentos propios — doMove() repetido

Todo local en Windows, sin Vagrant: Main-Container y Container-1 corren en
`localhost`, en dos terminales de la misma máquina.

## Pregunta

¿Qué pasa si un agente hace `doMove()` varias veces? Hay dos variantes de la
pregunta, y cada una tiene su propio agente:

1. **Varios saltos a lo largo del tiempo** ([`AgenteViajero`](myexamples/AgenteViajero.java)):
   el agente migra, en `afterMove()` pide la siguiente migración, y así hasta
   completar `maxSaltos`. Rebota entre `Main-Container` y `Container-1`.
2. **Dos `doMove()` seguidos, sin ceder el control** ([`AgenteDobleMove`](myexamples/AgenteDobleMove.java)):
   en el mismo `setup()`, sin esperar a que la primera migración se concrete,
   se pide `doMove()` a `Container-1` y enseguida a `Main-Container`. ¿Gana
   la primera petición, la segunda, o pasa algo raro?

## Cómo correrlo

### 1. Compilar

```bash
.\jade.ps1 compile
```

(o directo: `javac -classpath lib\jade.jar -d classes myexamples\AgenteViajero.java myexamples\AgenteDobleMove.java`)

### 2. Terminal 1 — Main-Container (con GUI)

```bash
.\jade.ps1 main
```

Esperar `Agent container Main-Container@... is ready.`

### 3a. Terminal 2 — Experimento 1: AgenteViajero

```bash
.\jade.ps1 container "viajero:AgenteViajero(6)"
```

El número entre paréntesis es `maxSaltos` (default 4 si no se pasa nada).
Mirar en la consola de Container-1 y en la GUI del Main-Container (pestaña
del contenedor, consola de logs) cómo el agente va apareciendo y
desapareciendo de cada uno a medida que rebota.

**Qué anotar:** ¿el agente efectivamente hace los `maxSaltos` saltos
pedidos? ¿`salto` se mantiene coherente (viaja con el agente, es un
atributo) o se resetea en algún punto? ¿Cuánto tarda cada salto (se nota
demora creciente, o es constante)?

### 3b. Terminal 2 — Experimento 2: AgenteDobleMove

(Correr después de cerrar o en una Container-1 nueva)

```bash
.\jade.ps1 container "doble:AgenteDobleMove"
```

**Qué anotar:** ¿el agente termina en `Container-1` (primera petición) o en
`Main-Container` (segunda)? ¿Aparece algún error/warning en el log por pedir
dos migraciones sin esperar? ¿Se imprime la línea "las dos migraciones ya
fueron pedidas..." antes o después de que el agente ya no esté en el
contenedor de origen (es decir, `doMove()` corta la ejecución ahí mismo o
sigue corriendo el resto del método)?

## Notas

- Classpath con `;` porque es Windows (en Linux/VM sería `:`).
- Si el Main-Container no arranca con `Cannot bind server socket to
  localhost port 1099`, hay otro proceso `java` viejo ocupando el puerto
  1099 — cerrarlo (`Get-NetTCPConnection -LocalPort 1099` para encontrar el
  PID, `Stop-Process -Id <pid>` para matarlo) y reintentar.
- `Ctrl+C` en cada terminal corta los contenedores.
