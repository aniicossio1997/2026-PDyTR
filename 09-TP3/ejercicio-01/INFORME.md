# Ejercicio 1: ping-pong con MQTT en tres máquinas virtuales

> Enunciado: desarrollar con MQTT, en tres máquinas virtuales, el código "ping-pong" de experimentación para evaluar el rendimiento de las comunicaciones, similar al de la práctica anterior. Explicar por qué podrían compararse ambos sistemas aunque la transferencia de datos se haga con protocolos diferentes. No se muestran resultados: solo se documentan las diferencias en el código y por qué sería comparable.

El código está en `fuentes/` (C y Python, equivalentes entre sí). La forma de ejecutarlo está en `PASOS.md`.

## 1. Qué se desarrolló

### Arquitectura

```
   mqttpub (192.168.56.11)        mqttbroker (192.168.56.10)       mqttsub (192.168.56.12)

   pp_client  ── ping ───────────►   Mosquitto   ── ping ───────────►  pp_echo
   (publica y mide) ◄── pong ─────   (puerto 1883) ◄── pong ─────────  (devuelve igual)
                                         ▲
                                         │ pingpong/ready (retenido)
                                 el eco avisa que ya está suscripto
```

| Máquina | Rol | Código propio |
|---|---|---|
| `mqttbroker` | Mosquitto: recibe y reparte los mensajes | ninguno (infraestructura) |
| `mqttpub` | Cliente: publica B bytes en `pingpong/ping`, espera `pingpong/pong` y mide | `pp_client` |
| `mqttsub` | Eco: devuelve cada mensaje, sin tocarlo, en `pingpong/pong` | `pp_echo` |

### Funcionamiento

1. El eco se suscribe a `pingpong/ping` y publica `pingpong/ready` **retenido** cuando ya está suscripto.
2. El cliente se suscribe a `pingpong/pong` y a `pingpong/ready`, y espera ese `ready` antes de medir.
3. Para cada tamaño (10¹ a 10⁶ bytes) y cada repetición (20 por defecto): el cliente toma `t0`, publica B bytes en `ping`, y toma `t1` al llegar el `pong`. `RTT = t1 − t0` y `RTT/2` estima el tiempo en un sentido, igual que en el TP2.
4. El cliente verifica el contenido recibido byte a byte (patrón `i % 256`, el mismo del TP2).
5. Se usa **una sola conexión** al broker durante todo el experimento y QoS 0 por defecto.

La parte de comunicaciones del cliente (versión en C), que es el equivalente del bloque medido en el TP2:

```c
clock_gettime(CLOCK_MONOTONIC, &t0);
mosquitto_publish(mosq, NULL, "pingpong/ping", size, payload, qos, false);
esperar(&got_pong, TIMEOUT_S);          // bloquea hasta que on_message recibe el pong
// t_pong se toma en la primera instrucción de on_message
rtt = t_pong - t0;
```

## 2. Diferencias en el código respecto al TP2

El TP2 (`ejercicio-03`) hacía el ping-pong con sockets TCP: un cliente y un servidor conectados directamente, y el bloque medido era `write` de ida + `read` de vuelta.

| Aspecto | TP2 (sockets TCP) | Este ejercicio (MQTT) |
|---|---|---|
| Biblioteca | API de sockets POSIX | libmosquitto (C) y paho (Python) |
| Procesos | 2: cliente y servidor | 3: cliente, eco y broker |
| Direccionamiento | El cliente conoce IP y puerto del servidor | Cliente y eco solo conocen el broker y los tópicos; no se conocen entre sí |
| Establecimiento | `connect()` directo al servidor | `mosquitto_connect()` al broker + suscripciones |
| Sincronización de arranque | Reintentar `connect()` ante `ECONNREFUSED` (el cliente puede arrancar antes) | El broker siempre acepta la conexión, pero un mensaje publicado sin suscriptor **se pierde**. Se resuelve con el `ready` retenido: el cliente no mide hasta recibirlo |
| Envío | `while (enviados < size) write(...)`: hay que reintentar si `write` envía menos | Una sola llamada `mosquitto_publish`; la biblioteca se encarga de enviar el mensaje completo |
| Recepción | `while (recibidos < size) read(...)`: TCP es un flujo y puede entregar en pedazos | Un callback `on_message` recibe el mensaje completo; no hay bucle ni conteo de lecturas |
| Espera de la respuesta | `read()` bloquea el hilo principal | El hilo principal espera con una variable de condición (C) o un `Event` (Python); el callback corre en el hilo de red de la biblioteca |
| Punto de `t1` | Después del último `read` | Primera instrucción del callback `on_message` del pong (antes de copiar el payload) |
| Servidor / eco | Un servidor por tamaño: conoce `esperados` y lee exactamente esa cantidad | El eco no conoce el tamaño: reenvía cada mensaje tal cual |
| Conexión | Una por repetición (el servidor se reinicia cada vez) | Una para todo el experimento |
| Contadores | `llamadas_write`, `lecturas_read` | No existen: la biblioteca oculta la fragmentación |
| Control de errores propio | — | Cuenta pings y pongs: si hay más pongs que pings, hay dos ecos corriendo y se aborta |
| Salida | Una línea por ejecución (`bytes_ida bytes_vuelta rtt_ms rtt/2 ...`) | CSV con todas las repeticiones: `exp,bytes,rep,rtt_ms,rtt2_ms,ok` |

Lo que **no** cambia: el patrón de datos (`i % 256`), la verificación byte a byte fuera de la medición, el reloj monótono, los tamaños 10¹ a 10⁶, las 20 repeticiones, y que el tiempo se mide de punta a punta en el cliente, como **un solo bloque** que envuelve la ida y la vuelta.

### Equivalencia entre las versiones C y Python

`pp_client.c` / `pp_echo.c` y `pp_client.py` / `pp_echo.py` implementan el mismo protocolo y los mismos tópicos, por lo que se pueden combinar (cliente en C con eco en Python y viceversa). La versión en C es la más cercana al TP2, que también estaba en C; la de Python permite ver cuánto aporta el lenguaje al tiempo medido.

## 3. Por qué los dos sistemas se pueden comparar

Comparar no exige que el protocolo sea el mismo. Exige que se mida **lo mismo, en condiciones controladas**, y que las diferencias que queden sean justamente lo que se quiere estudiar. Acá el protocolo (y la arquitectura que implica) es la variable bajo estudio; todo lo demás se mantiene.

1. **Es el mismo fenómeno medido.** En ambos casos se envían B bytes de una máquina a otra, se devuelven los mismos B bytes y se mide el tiempo de ida y vuelta, desde el punto de vista del cliente. La pregunta es la misma: ¿cuánto tarda en completarse un intercambio de B bytes?
2. **Es el mismo transporte debajo.** MQTT funciona sobre TCP. Entonces la comparación no es "TCP contra algo ajeno", sino "TCP directo contra TCP más una capa de mensajería con un intermediario". La diferencia medida se puede atribuir a esa capa.
3. **Las condiciones experimentales son iguales.** Mismo tipo de entorno (VMs de Vagrant con `bento/ubuntu-22.04` en una red privada), mismos tamaños (10¹ a 10⁶ bytes), misma cantidad de repeticiones, mismo patrón de datos, mismo reloj monótono y la misma regla de medición: un único par de marcas de tiempo alrededor de ida y vuelta, con el reloj de una sola máquina (no hace falta sincronizar relojes).
4. **La preparación queda fuera de la medición en los dos casos.** En el TP2 se excluían los reintentos de `connect()`; acá se excluyen la conexión, las suscripciones y la espera del `ready`.
5. **La misma métrica.** Se reporta el RTT y RTT/2, y se analiza igual la proporcionalidad entre bytes y tiempo (ejercicio 2), de modo que los números son directamente confrontables.
6. **El lenguaje se puede igualar.** La versión en C usa el mismo lenguaje y la misma función de reloj que el TP2, así que el lenguaje no se mezcla con el efecto del protocolo.

### Diferencias que hay que declarar al comparar

Estas diferencias no invalidan la comparación; son justamente lo que explica los resultados y deben mencionarse al interpretarlos.

- **El broker agrega un salto.** Un ping-pong son 4 tramos de red (cliente→broker→eco→broker→cliente) contra 2 en sockets directos. Además el broker procesa cada mensaje completo (lo recibe entero antes de entregarlo), así que no hay solapamiento entre recibir y reenviar.
- **Sobrecarga por mensaje.** Cada PUBLISH lleva cabecera MQTT (1 byte fijo, de 1 a 3 de longitud restante para estos tamaños, 2 de longitud del tópico y 13 del tópico `pingpong/ping`/`pingpong/pong`): unos 17 a 19 bytes por mensaje. Pesa en los tamaños chicos y es despreciable en los grandes.
- **Mensaje contra flujo de bytes.** TCP entrega un flujo que puede llegar en pedazos; MQTT entrega mensajes completos. El tiempo MQTT incluye el reensamblado del mensaje, que en el TP2 se veía como varias lecturas.
- **RTT/2 no significa lo mismo.** En el TP2 era una estimación del tiempo en un sentido entre dos computadoras; acá es el tiempo en un sentido **pasando por el broker**.
- **Una conexión contra una conexión por repetición.** El TP2 abría una conexión nueva en cada repetición; acá se reutiliza una. La primera repetición de cada corrida incluye el arranque en frío de esa conexión. Es una decisión de diseño que puede cambiarse si se quiere un criterio idéntico.
- **QoS.** Con QoS 0, MQTT no agrega confirmaciones propias (TCP sigue confirmando por debajo), que es lo más parecido al caso sin broker. Con QoS 1 y 2 se suman intercambios adicionales y la comparación cambia de naturaleza.
- **Entorno virtual.** Las VMs comparten la CPU del anfitrión y la red es virtual (igual que en el escenario VM↔VM del TP2), de modo que parte del tiempo es costo de virtualización, no de la red.

## 4. Resumen

Se desarrolló un ping-pong sobre MQTT con la misma estructura de medición que el TP2: un cliente que mide, de punta a punta, el tiempo de enviar B bytes y recibirlos de vuelta, y un eco que los devuelve. Las diferencias en el código son consecuencia directa del modelo publicación/suscripción: no hay conexión directa, no hay bucles de lectura y escritura, la espera de la respuesta es explícita y el arranque se sincroniza con un mensaje retenido. Ambos sistemas son comparables porque miden lo mismo, sobre el mismo transporte y en las mismas condiciones, y lo que los distingue (el intermediario y el encapsulado de mensajes) es precisamente el objeto de comparación.
