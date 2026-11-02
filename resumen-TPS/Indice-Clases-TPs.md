# Índice de clases teóricas de PDyTR y con qué TP conecta cada una

Revisé el contenido real de cada PDF de clase (no solo el nombre de la carpeta) y armé esta
tabla con **qué trata cada clase** y **con qué TP (o TPs) hace match**, en el mismo orden en
que aparecen en el cronograma/numeración de carpetas.

Convención de la columna "TP":
- **TP1 / TP2 / TP3 / TP4**: conexión directa y fuerte con ese trabajo práctico.
- **Transversal**: es marco conceptual que se usa en todos los TPs, no uno en particular.
- Cuando conecta con más de uno, están los dos, con el más fuerte primero.

---

## Tabla general

| # | Archivo | De qué trata (contenido real de las diapositivas) | TP con el que conecta | Por qué |
|---|---|---|---|---|
| 1 | `00-Inicio.pdf` | Presentación de la cátedra: docentes, plataforma Ideas, cronograma, bibliografía (Coulouris, Tanenbaum/Van Steen, Liu), formato de aprobación. | **Transversal** | Es la clase organizativa/administrativa, no toca contenido técnico de ningún TP puntual. |
| 2 | `01-Virt/clase-01-Virt.pdf` | Virtualización: ambientes virtuales, "bare metal/host/guest", hipervisores tipo I y II, y **Vagrant** en detalle (boxes, vagrantfile, `vagrant up/ssh/halt/destroy`, "mini red de VMs"). | **TP1 y TP2** (y en menor medida TP3) | Vagrant es la herramienta de despliegue que pide explícitamente TP1 y, sobre todo, TP2 (automatizar 2 VMs o VM+host). TP3 también usa 3 VMs con Vagrant (`mqttpub`/`mqttbroker`/`mqttsub`), pero el contenido técnico de esta clase es 100% Vagrant/VirtualBox, la base de *todas* las prácticas. |
| 3 | `01b-MasVirt.pdf` | Comparación Vagrant vs. Terraform vs. Kubernetes (consulta a asistentes de IA: diferencias, cuándo usar cada uno). | **Transversal** (apoya TP1/TP2) | Profundiza por qué se usa Vagrant y no Docker/K8s/Terraform en la cursada — contexto de herramientas, no un TP en particular. |
| 4 | `01c-Materiales.pdf` | Repaso administrativo + anuncia que las clases 02-DefEvol y 03-C-S quedan como video, aclarando **"03-C-S también para el TP1"**. | **Transversal → anticipa TP1** | Es la propia cátedra la que marca explícitamente que la clase de Cliente/Servidor (03) es material de TP1. |
| 5 | `02-DefEvol/clase-02-Def-Evol.pdf` | Definiciones de "sistema distribuido" (Tanenbaum, Coulouris, Liu) y evolución histórica: redes locales → salto a Internet (IP, sockets) → aplicaciones web → apps móviles ("retroceso" a c/s "casi plano"). | **Transversal** (fundamento conceptual de TP1) | Es el marco teórico general de qué es un SD; la parte de "sockets, API(s), local/no local" es la antesala directa de TP1. |
| 6 | `03-C-S/clase-03-C-S.pdf` | Modelo cliente/servidor: diagramas temporales de petición/respuesta (tep, trp, ter, trr), las 9 características del modelo c/s (cliente activo, servidor pasivo, interfaces bien definidas, etc.), y la aclaración **"c/s no implica necesariamente conexión TCP"**. | **TP1 y TP2** | Es la teoría formal del modelo que se implementa a mano en TP1/TP2 con sockets; el diagrama tep/trp/ter/trr es exactamente lo que después se mide con t0/t1 en TP2. |
| 7 | `04-TP1/Expl1-Sockets.pdf` | Explicación de práctica: sockets en C y Java, secuencia socket/bind/listen/accept, TCP (stream) vs UDP (datagram). | **TP1** | Es la explicación de práctica del TP1 en sí. |
| 8 | `05-Caracteristicas/clase-05-Caracteristicas.pdf` | Características de los SD: sin estado global, concurrencia y recursos compartidos, escalabilidad, seguridad, apertura/heterogeneidad, y las **8 "transparencias"** (acceso, ubicación, concurrencia, réplica, fallas, **movilidad**, rendimiento, escala). | **Transversal** (con guiños a TP3/TP4) | Marco general de la materia; la *transparencia de movilidad* anticipa JADE (TP4) y la de *fallas/réplica* anticipa la tolerancia a fallas que pide TP3 (QoS). |
| 9 | `06-VentajasTipos/clase-06-VentajasTipos.pdf` | Ventajas/desventajas de distribuido vs. centralizado, y tipos de SD: *Distributed Computing Systems* (cluster/grid/cloud), *Distributed Information Systems*, y **Distributed Pervasive Systems** (sensores, "home systems", redes de sensores con/sin procesamiento local). | **TP3** (con alcance transversal) | La sección de "sensor networks" / sistemas pervasivos es exactamente el escenario de uso de MQTT (sensores que publican, por ejemplo `casa/cocina/temp`). |
| 10 | `07-Soporte/clase-07-Soporte.pdf` | SOD vs. SOR vs. **Middleware** (con la definición extensa de middleware como "el software en el medio del cliente/servidor"), tabla comparativa de transparencia/escalabilidad por tipo, hilos en cliente y servidor (thread-per-request/connection/object), y las "falsas suposiciones" de Van Steen sobre la red (latencia 0, ancho de banda infinito, etc.). | **TP3** (middleware) **y TP1/TP2** (hilos) | El *broker* de MQTT (Mosquitto) es, literalmente, el middleware que describe esta clase. La parte de hilos en servidor conecta con cómo se podría escalar un servidor de sockets en TP1/TP2. |
| 11 | `08-EstilosArq/clase-08-EstilosArq.pdf` | **Estilos arquitectónicos** de sistemas distribuidos: Layered (capas), Object-based (objetos distribuidos/RMI), Resource-centered (REST), y **Publish-Subscribe** en detalle (publicadores/suscriptores, event bus, desacople en espacio y en tiempo, comunicación asincrónica unidireccional). | **TP3** (y TP1/TP2 vía Layered) | Esta es **la clase bisagra**: acá se explica formalmente el estilo "publicación/suscripción" que después se concreta con MQTT en TP3 (el mismo vocabulario: desacoplado en espacio, acoplado en tiempo, event bus = broker). El estilo "Layered" es el que fundamenta cliente/servidor de TP1/TP2. |
| 12 | `09-TP3/Expl3-PubSub-MQTT.pdf` | Explicación de práctica: de pub/sub genérico a MQTT concreto — tópicos, comodines `+`/`#`, QoS 0/1/2, retained, last will, laboratorio con Vagrant (`mqttpub`/`mqttbroker`/`mqttsub`). | **TP3** | Es la explicación de práctica del TP3 en sí. |
| 13 | `10-EA-Capas/Clase-10-EA-Capas.pdf` | Profundización del estilo "Layered": arquitecturas de 1 capa (monolito), 2 capas (c/s puro), 3 capas (interfaz/procesamiento/datos, "gordo vs. flaco"), y la discusión (con los propios Tanenbaum y Van Steen) de cuántas capas tiene sentido tener en producción. | **TP1 y TP2** | Es el desarrollo completo del estilo cliente/servidor en capas — el fundamento arquitectónico de lo que TP1/TP2 implementan con sockets en 2 capas (cliente/servidor). |
| 14 | `clase-11-TR.pdf` | Definición técnica de **Tiempo Real**: crítica al uso comercial del término, clasificación hard/soft/firm (destrucción vs. calidad vs. descarte), tareas periódicas/aperiódicas/esporádicas, diagrama t0-t1-t2-t3 (activación, inicio, fin, plazo), y planificación (scheduling) orientada a cumplir plazos. | **Transversal fuerte → TP2 y TP3** | Da el marco conceptual de "Tiempo Real" (el nombre de la materia): TP2 mide tiempos de comunicación (promedio vs. peor caso = exactamente esta clase), y TP3 usa QoS (0/1/2), que es una clasificación de garantías muy parecida a hard/soft/firm. |
| 15 | `12-TP4/Expl4-JADE.pdf` | Explicación de práctica: de cliente/servidor y MQTT a **JADE** — agentes móviles, plataforma/contenedor/agente, movilidad débil vs. fuerte, ciclo `setup()`/`doMove()`/`beforeMove()`/`afterMove()`, demo de migración. | **TP4** | Es la explicación de práctica del TP4 en sí. |

---

## Resumen visual: qué clase prepara a qué TP

```
TP1 (Sockets)         ← 02-DefEvol, 03-C-S, 10-EA-Capas (capas 2), 01-Virt (Vagrant)
TP2 (Sockets + tiempos)← mismas que TP1, + clase-11-TR (cómo medir/clasificar tiempo)
TP3 (MQTT / pub-sub)  ← 06-VentajasTipos (sist. pervasivos/sensores), 07-Soporte (middleware),
                          08-EstilosArq (publish-subscribe ¡clave!), clase-11-TR (QoS)
TP4 (JADE / agentes)  ← 05-Caracteristicas (transparencia de movilidad), 08-EstilosArq (object-based)
```

**La clase más importante para entender el "por qué" de cada TP es `08-EstilosArq`**: ahí es
donde la cátedra define, en abstracto, los cuatro estilos arquitectónicos (capas, objetos,
recursos/REST y publicación-suscripción) — y TP1/TP2, TP3 y TP4 son, cada uno, la
implementación concreta de uno de esos estilos.
