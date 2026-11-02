import argparse
import sys
import threading
import time
import paho.mqtt.client as mqtt

# Ping-pong MQTT (cliente): publica B bytes en pingpong/ping, espera que
# pp_echo.py los devuelva en pingpong/pong y mide el tiempo de ida y vuelta.
# Una sola conexion al broker para todo el experimento.
TOPIC_PING = "pingpong/ping"
TOPIC_PONG = "pingpong/pong"
TOPIC_READY = "pingpong/ready"
TIMEOUT_S = 10

parser = argparse.ArgumentParser(description="Ping-pong MQTT: cliente")
parser.add_argument("-b", "--broker", default="192.168.56.10", help="IP o nombre del broker")
parser.add_argument("-p", "--port", type=int, default=1883, help="puerto del broker")
parser.add_argument("-q", "--qos", type=int, choices=[0, 1, 2], default=0, help="QoS de publicacion y suscripcion")
parser.add_argument("-n", "--reps", type=int, default=20, help="repeticiones por tamano")
parser.add_argument("-e", "--emin", type=int, default=1, help="exponente minimo (10^e bytes)")
parser.add_argument("-E", "--emax", type=int, default=6, help="exponente maximo (10^e bytes)")
args = parser.parse_args()

subs_done = threading.Event()
ready = threading.Event()
got_pong = threading.Event()
pong = {"payload": None, "t1": 0, "total": 0}

#################################################################
# Callback: se suscribe al conectar (un solo SUBACK para los dos topicos)
def on_connect(client, userdata, flags, rc):
    client.subscribe([(TOPIC_PONG, args.qos), (TOPIC_READY, args.qos)])

def on_subscribe(client, userdata, mid, granted_qos):
    subs_done.set()

#################################################################
# Callback (hilo de la biblioteca): llega el "ready" del eco o un pong.
# t1 se toma lo primero, antes de tocar el payload.
def on_message(client, userdata, msg):
    t1 = time.perf_counter_ns()
    if msg.topic == TOPIC_READY:
        # payload vacio = el eco se fue
        ready.set() if msg.payload else ready.clear()
    elif msg.topic == TOPIC_PONG:
        pong["payload"] = msg.payload
        pong["total"] += 1
        pong["t1"] = t1
        got_pong.set()

#################################################################
# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
else:
    client = mqtt.Client()

client.on_connect = on_connect
client.on_subscribe = on_subscribe
client.on_message = on_message

# Una unica conexion para todo el experimento
client.connect(args.broker, args.port, 60)
client.loop_start()

# Sincronizacion de arranque: publicar antes de que el eco este suscripto
# perderia el mensaje. El eco deja un "ready" retenido en el broker, asi que
# no importa cual de los dos arranque primero. Queda fuera de la medicion.
if not subs_done.wait(TIMEOUT_S):
    sys.exit("ERROR: sin SUBACK del broker")
while not ready.wait(5):
    print("eco no disponible, esperando...", file=sys.stderr)

print("exp,bytes,rep,rtt_ms,rtt2_ms,ok")
medias = {}
enviados = 0

for e in range(args.emin, args.emax + 1):
    size = 10 ** e
    # Patron determinista i % 256, igual que en el TP2 y en la version en C
    payload = (bytes(range(256)) * (size // 256 + 1))[:size]
    suma = 0.0

    for r in range(1, args.reps + 1):
        got_pong.clear()
        t0 = time.perf_counter_ns()
        client.publish(TOPIC_PING, payload, qos=args.qos)
        enviados += 1

        if not got_pong.wait(TIMEOUT_S):
            sys.exit(f"ERROR: timeout esperando el pong (exp={e} rep={r})")

        # Mas pongs que pings: hay dos ecos corriendo y los tiempos no sirven
        if pong["total"] > enviados:
            sys.exit(f"ERROR: {pong['total']} pongs para {enviados} pings (hay mas de un eco corriendo?)")

        rtt = (pong["t1"] - t0) / 1e6
        ok = int(pong["payload"] == payload)
        print(f"{e},{size},{r},{rtt:.6f},{rtt / 2:.6f},{ok}")
        suma += rtt

    medias[e] = suma / args.reps

print(f"\n{'exp':<8}{'bytes':<10}{'RTT medio ms':<16}{'RTT/2 medio ms':<16}", file=sys.stderr)
for e, m in medias.items():
    print(f"{e:<8}{10 ** e:<10}{m:<16.6f}{m / 2:<16.6f}", file=sys.stderr)

# Un segundo eco responde un poco despues del primero: dar tiempo a que llegue
time.sleep(0.3)
if pong["total"] != enviados:
    sys.exit(f"ERROR: {pong['total']} pongs para {enviados} pings (hay mas de un eco corriendo?), descartar los resultados")

client.disconnect()
client.loop_stop()
