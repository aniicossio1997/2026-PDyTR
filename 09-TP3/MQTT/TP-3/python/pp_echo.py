import argparse
import sys
import time
import paho.mqtt.client as mqtt

# Ping-pong MQTT (eco): se suscribe a pingpong/ping y devuelve cada mensaje,
# sin tocarlo, en pingpong/pong. No conoce el tamano: sirve para cualquier B.
TOPIC_PING = "pingpong/ping"
TOPIC_PONG = "pingpong/pong"
TOPIC_READY = "pingpong/ready"

parser = argparse.ArgumentParser(description="Ping-pong MQTT: eco")
parser.add_argument("-b", "--broker", default="192.168.56.10", help="IP o nombre del broker")
parser.add_argument("-p", "--port", type=int, default=1883, help="puerto del broker")
parser.add_argument("-q", "--qos", type=int, choices=[0, 1, 2], default=0, help="QoS de suscripcion y republicacion")
args = parser.parse_args()

#################################################################
# Callback: se suscribe al conectar, asi al reconectarse vuelve a suscribirse
def on_connect(client, userdata, flags, rc):
    client.subscribe(TOPIC_PING, qos=args.qos)

#################################################################
# Ya suscripto: avisa que esta listo. El mensaje queda retenido en el broker,
# asi el cliente lo recibe aunque arranque despues.
def on_subscribe(client, userdata, mid, granted_qos):
    client.publish(TOPIC_READY, "ready", qos=1, retain=True)
    print(f"eco listo (QoS {args.qos})", file=sys.stderr)

#################################################################
def on_message(client, userdata, msg):
    client.publish(TOPIC_PONG, msg.payload, qos=args.qos)

#################################################################
# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id="ppeco-py")
else:
    client = mqtt.Client(client_id="ppeco-py")

client.on_connect = on_connect
client.on_subscribe = on_subscribe
client.on_message = on_message

# Last will: si el eco muere sin avisar, el broker borra el "ready" retenido
# (payload vacio + retained) y ningun cliente cree que sigue vivo.
client.will_set(TOPIC_READY, payload=None, qos=1, retain=True)

client.connect(args.broker, args.port, 60)
client.loop_start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    # Salida ordenada: el will no se publica, asi que se borra el "ready" a mano
    client.publish(TOPIC_READY, None, qos=1, retain=True).wait_for_publish()
    client.disconnect()
    client.loop_stop()
