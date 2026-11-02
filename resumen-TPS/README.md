# PDyTR 2026 — Resumen: TP1, TP2, TP3 y TP4

Resumen propio (no oficial de la cátedra) armado a partir del material de la carpeta
`MaterialEstudiantesPDyTR2026` para entender **qué conecta a los cuatro trabajos prácticos**,
qué tecnología usa cada uno, y por qué la cátedra los ordena en ese orden.

---

## 1. La idea general de la materia

La materia (Programación Distribuida y Tiempo Real) va mostrando, TP tras TP, **distintas formas
de comunicar procesos que corren en máquinas distintas**, cada vez con un modelo más desacoplado
que el anterior:

```
TP1/TP2                TP3                      TP4
Sockets            →   MQTT (pub/sub)       →   JADE (agentes móviles)
"yo hablo con vos"     "yo grito al aire,        "en vez de mandar un
 directamente"          alguien me escucha"       mensaje, mando el
                                                   programa entero"
```

Es la misma pregunta de fondo en los tres casos: **¿cómo hacen dos (o más) procesos en
computadoras distintas para intercambiar información?** Lo que cambia es **cuánto se conocen
entre sí, si tienen que estar los dos activos al mismo tiempo, y qué es lo que efectivamente
viaja por la red** (datos, mensajes, o el código mismo).

La teoría (clases 01 a 11, ver carpetas `01-Virt` a `clase-11-TR.pdf`) va en paralelo dando el
marco conceptual: qué es un sistema distribuido, sus características, ventajas/desventajas,
estilos arquitectónicos (cliente/servidor, capas, pub/sub, agentes, P2P, etc.) y nociones de
tiempo real. Las prácticas son la base **experimental**: tomarle el tiempo a esas arquitecturas
para verificar en carne propia lo que la teoría describe en abstracto.

---

## 2. Qué tema "toca" cada clase teórica (mapa rápido)

| Carpeta | Tema |
|---|---|
| `01-Virt`, `01b-MasVirt` | Virtualización (Vagrant, VMs) — la infraestructura que se usa en *todos* los TPs |
| `02-DefEvol` | Definición de sistema distribuido y su evolución histórica |
| `03-C-S` | Modelo cliente/servidor (la base conceptual del TP1/TP2) |
| `04-TP1` | Sockets — primera implementación concreta de cliente/servidor |
| `05-Caracteristicas` | Características de los sistemas distribuidos (transparencia, concurrencia, etc.) |
| `06-VentajasTipos` | Ventajas y tipos de sistemas distribuidos |
| `07-Soporte` | Soporte de sistema operativo / middleware para distribución |
| `08-EstilosArq` | Estilos arquitectónicos (acá aparece la idea de pub/sub, capas, etc. — preparación para TP3) |
| `09-TP3` | Publicación/suscripción y MQTT |
| `10-EA-Capas` | Estilos arquitectónicos en capas |
| `clase-11-TR` | Tiempo real (por qué medir tiempos, determinismo, QoS) |
| `12-TP4` | Movilidad de código y agentes (JADE) |

**Patrón:** cada TP tiene su propia teoría "preparatoria" un par de clases antes, y una
`Expl-N` (explicación de práctica) que es la bajada a tierra de esa teoría para el TP puntual.

---

## 3. TP1 — Sockets (C y Java), lo más bajo nivel

- **Qué es:** comunicación cliente/servidor directa con **sockets** (interfaz UNIX de
  comunicación entre procesos), en C y en Java.
- **Qué viaja por la red:** datos crudos (bytes). El cliente arma un pedido, el servidor
  responde.
- **Quién espera a quién:** el cliente espera al servidor (es *bloqueante*: `read()`/`write()`
  se quedan esperando).
- **Flujo típico:**
  - Servidor: `socket()` → `bind()` → `listen()` → `accept()` → `read()`/`write()`
  - Cliente: `socket()` → `connect()` → `write()`/`read()`
- **TCP vs UDP:** socket *stream* (TCP) = orientado a conexión, orden garantizado → es la base
  del modelo cliente/servidor. Socket *datagram* (UDP) = sin conexión, cada paquete viaja solo.
- **C vs Java:** en C se lee/escribe directo sobre el *file descriptor* del socket. En Java
  siempre hay un `InputStream`/`OutputStream` de por medio (`Socket`, `ServerSocket`).
- **Por qué se enseña primero:** es el nivel más bajo y más explícito. Obliga a ver "a mano"
  los pasos de inicialización, acoplamiento (cliente y servidor se tienen que conocer por
  IP/puerto) y sincronización (el server tiene que estar en `listen()` antes de que el cliente
  pueda conectar) — todo lo que las tecnologías de los TPs siguientes *esconden*.

## 4. TP2 — Sockets + medición de tiempos + ambiente real distribuido

TP2 **no es una tecnología nueva**: es el mismo mecanismo de sockets del TP1, pero llevado a un
escenario más realista y con foco en **medir** (empieza a aparecer el costado "Tiempo Real" de
la materia):

- **Qué agrega respecto del TP1:**
  - Experimentos en **dos computadoras físicamente distintas** (no simulado en una sola máquina).
  - **Medición de tiempos** de `write()`/`read()` para distintas cantidades de datos
    (10¹ a 10⁶ bytes), y análisis de si el tiempo escala proporcionalmente con los datos.
  - Experimento **"ping-pong"**: mandar datos ida y vuelta y dividir por 2 para estimar el
    tiempo de un solo sentido (porque normalmente no se puede medir un solo sentido de forma
    directa: los relojes de las dos máquinas no están sincronizados).
  - **Automatización con Vagrant**: levantar automáticamente las VMs que hacen de cliente y
    servidor (y resolver el problema de que el cliente no se puede conectar si arranca antes
    que el servidor — un primer contacto con el **asincronismo** entre procesos distribuidos).
  - Datos **binarios** (no texto/char), porque en un sistema real no se manda texto plano.
- **Idea pedagógica central:** mostrar con números reales cosas como "¿`write()` de 1000 bytes
  tarda 10 veces más que uno de 100 bytes?" — pasar de la teoría de sockets a **evidencia
  empírica** sobre el costo de comunicarse en red. Es el ensayo metodológico (cómo armar un
  experimento, cómo medir, cómo automatizar) que después se reutiliza en TP3 para comparar
  MQTT contra sockets en igualdad de condiciones.

## 5. TP3 — MQTT (publicación/suscripción)

- **Qué es:** un cambio de paradigma. En vez de que dos procesos se conecten directamente
  (cliente/servidor), aparece un tercer actor, el **broker**, y los procesos ya no se
  conocen entre sí:
  - **Publicador**: manda mensajes a un **tópico** (ej. `casa/cocina/temp`), sin saber quién
    (ni si alguien) los va a leer.
  - **Suscriptor**: se suscribe a un tópico (con comodines `+` o `#`) y recibe lo que se
    publique ahí, sin saber quién lo publicó.
  - **Broker** (en la práctica: **Mosquitto**): intermediario que reparte los mensajes.
- **Qué viaja por la red:** mensajes, siempre a través del broker.
- **Quién espera a quién:** el suscriptor espera (recibe asíncronamente); **el publicador no
  espera nada** — publicar no devuelve una respuesta.
- **Tres desacoplamientos que resuelve** (y que en cliente/servidor NO existen):
  1. **Espacio:** publicador y suscriptor no se conocen (ni IP ni puerto del otro).
  2. **Tiempo:** no hace falta que ambos estén activos al mismo tiempo.
  3. **Sincronización:** nadie queda bloqueado esperando al otro.
- **Mecanismos específicos de MQTT:**
  - **QoS 0/1/2** (a lo sumo una vez / al menos una vez / exactamente una vez) — relacionado
    directamente con tiempo real y tolerancia a fallas.
  - **retained**: el broker guarda el último mensaje de un tópico y se lo manda a quien se
    suscriba después (el "tablero" que quiere el valor actual apenas se conecta).
  - **last will and testament (LWT)**: mensaje que el broker publica automáticamente si un
    cliente se desconecta de golpe ("sensor caído").
- **Por qué se compara con TP1/TP2:** el TP3 pide repetir el experimento "ping-pong" con MQTT
  para comparar tiempos contra sockets puros, aunque el protocolo de transporte sea distinto —
  mostrando el **costo** (en latencia) de tener un intermediario, a cambio del desacoplamiento
  que gana el sistema.
- **Dónde encaja en el panorama más amplio:** MQTT es solo una familia dentro de los sistemas
  pub/sub (otras: Kafka, RabbitMQ/AMQP, Redis pub/sub, ZeroMQ, DDS). Se usa MQTT en la materia
  porque es liviano, corre en una VM chica, y es el estándar de IoT.

## 6. TP4 — JADE (agentes móviles / movilidad de código)

- **Qué es:** acá el cambio es más radical: **lo que viaja por la red ya no es un mensaje, es
  el programa**. JADE es un framework de **agentes móviles** en Java.
- **Conceptos de JADE:**
  - **Agente**: un objeto Java autónomo que puede moverse entre contenedores (`doMove()`).
  - **Contenedor**: un ambiente de ejecución (una JVM) donde viven agentes — equivalente,
    salvando las distancias, a dónde corre un proceso servidor en los TPs anteriores.
  - **Plataforma**: el conjunto de contenedores (potencialmente en máquinas distintas) que
    forman un sistema JADE.
- **Qué viaja y qué no (movilidad "débil, con estado de datos" en JADE):**
  - Viaja el **código** (las clases) y los **atributos** (datos) del agente.
  - **No** viaja el estado de ejecución: al llegar al destino, el agente no continúa en la
    línea donde estaba, arranca de nuevo desde `afterMove()`. Tampoco viajan las variables
    locales ni lo marcado `transient`.
  - Ciclo de métodos: `setup()` (solo al crear) → `doMove(destino)` (pide la migración, no
    migra en el instante) → `beforeMove()` (en el origen) → `afterMove()` (en el destino).
- **Por qué mover código en vez de mandar un mensaje:** eficiencia (procesar donde están los
  datos, en vez de traer todos los datos a un servidor central), ejecutar código que el
  destino no tenía instalado de antemano, y acercar el cómputo al lugar donde hace falta.
  El caso de uso típico del TP: recolectar el estado (CPU, memoria, nombre) de muchas
  computadoras **sin tener que instalar un servidor en cada una** — el agente viaja, mide,
  y vuelve con los datos.
- **Comparación explícita que pide el TP4 (ejercicios 4 y 5):** contrastar JADE contra
  cliente/servidor para ese mismo problema de recolectar estado de varias máquinas, viendo
  qué pasa si se agregan/quitan computadoras dinámicamente, o si cambia el tipo de dato a
  recolectar — mostrando que con JADE no hace falta tener un "servidor" corriendo en cada
  máquina de antemano, ni recompilar nada si cambia lo que se quiere medir.

---

## 7. Tabla comparativa: las tres tecnologías, una al lado de la otra

| | **Sockets (TP1/TP2)** | **MQTT (TP3)** | **JADE (TP4)** |
|---|---|---|---|
| Paradigma | Cliente/servidor | Publicación/suscripción | Agentes móviles / código móvil |
| Qué viaja por la red | Datos (pedido y respuesta) | Mensajes, vía un broker | El programa (código + atributos) |
| Quién espera a quién | El cliente espera al servidor | El suscriptor espera; el publicador no | El agente "va, trabaja y vuelve" (nadie más está bloqueado) |
| Acoplamiento | Alto: cada extremo conoce IP/puerto del otro | Bajo: ni se conocen, ni coinciden en el tiempo, ni se bloquean | Variable: el agente sabe a dónde va, pero el destino no necesita conocerlo de antemano |
| Si se agrega un nuevo "consumidor"/nodo | Hay que tocar/recompilar el otro extremo | No: solo se suscribe al tópico | No: el agente simplemente visita el nuevo contenedor |
| Rol del intermediario | No hay (comunicación directa) | Broker (Mosquitto, HiveMQ) es imprescindible | No hay broker: la plataforma JADE enruta la migración |
| Qué mide/enseña el TP | Costo básico de la comunicación en red, sincronización manual | Costo/beneficio del desacoplamiento | Costo/beneficio de mover el procesamiento en vez de los datos |

---

## 8. Por qué se usa cada una (cuándo elegirla en la vida real)

- **Sockets / cliente-servidor:** cuando dos partes bien definidas necesitan "preguntar y
  obtener respuesta" y tiene sentido que se conozcan entre sí (la base de HTTP, bases de
  datos, RPC, etc.).
- **MQTT / pub-sub:** cuando hay muchos productores y/o consumidores que no se conocen, que
  se conectan y desconectan todo el tiempo, y donde "esperar una respuesta" no tiene sentido
  (sensores IoT, notificaciones, telemetría).
- **JADE / movilidad de código:** cuando conviene llevar el procesamiento a donde están los
  datos en vez de traer los datos al procesamiento, o cuando no se puede (o no conviene)
  tener instalado de antemano en cada nodo el software que lo va a procesar.

---

## 9. Cómo se conecta todo (la "lectura" de la cátedra)

1. **TP1** da el vocabulario mínimo (socket, bind, connect, cliente/servidor, TCP vs UDP) y
   muestra que comunicar dos procesos en red implica muchos pasos manuales y mucho
   acoplamiento.
2. **TP2** no cambia la tecnología, cambia la **pregunta**: deja de ser "¿cómo lo hago
   funcionar?" y pasa a ser "¿cuánto cuesta (en tiempo) hacerlo funcionar, y cómo lo mido
   bien?" — introduce método experimental, automatización (Vagrant) y el primer problema de
   sincronización distribuida real (quién arranca primero).
3. **TP3** usa exactamente la misma vara de medir que entrenaste en el TP2 (ping-pong, tiempos
   proporcionales a los datos) pero sobre una arquitectura **distinta**: pub/sub. Así se puede
   comparar, con números, el costo de ganar desacoplamiento.
4. **TP4** lleva la idea un paso más allá: si en TP3 lo que viajaba era un *mensaje* a través
   de un intermediario, en TP4 lo que viaja es *el programa entero*. Es la arquitectura más
   "distribuida" de las tres, porque el cómputo deja de tener una ubicación fija.

En definitiva, la progresión TP1→TP2→TP3→TP4 es una escalera de **desacoplamiento creciente**
entre las partes que se comunican, y en paralelo una escalera de **sofisticación en cómo medís
lo que pasa en esa comunicación** (tiempos, QoS, tolerancia a fallas). Los tres paradigmas
(cliente/servidor, pub/sub, agentes móviles) son, de hecho, tres de los "estilos
arquitectónicos" que ve la teoría (`08-EstilosArq`, `10-EA-Capas`) — los TPs son la forma de
tocarlos con las manos.
