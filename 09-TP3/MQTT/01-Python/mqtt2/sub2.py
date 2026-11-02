import argparse
import paho.mqtt.client as mqtt

# Igual que sub1.py pero parametrizado
parser = argparse.ArgumentParser(description="Suscriptor MQTT")
parser.add_argument("-b", "--broker", default="192.168.56.10", help="IP o nombre del broker")
parser.add_argument("-p", "--port", type=int, default=1883, help="puerto del broker")
parser.add_argument("-t", "--topic", default="test00", help="topico (admite comodines + y #)")
parser.add_argument("-q", "--qos", type=int, choices=[0, 1, 2], default=0, help="QoS de la suscripcion")
parser.add_argument("-i", "--id", default=None, help="id de cliente (necesario para sesion persistente)")
parser.add_argument("-c", "--persist", action="store_true", help="sesion persistente (clean_session=False)")
args = parser.parse_args()

#################################################################
# Callback: la biblioteca la llama cada vez que llega un mensaje de algun topico suscripto
def on_message(client, userdata, msg):
    print(f"[{msg.topic}] (QoS {msg.qos}{', retained' if msg.retain else ''}) {msg.payload.decode()}")

#################################################################
# Callback: se suscribe al conectar, asi al reconectarse vuelve a suscribirse
def on_connect(client, userdata, flags, rc):
    print(f"Conectado a {args.broker}:{args.port} (rc={rc}), suscripto a '{args.topic}' con QoS {args.qos}")
    client.subscribe(args.topic, qos=args.qos)

#################################################################
# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id=args.id, clean_session=not args.persist)
else:
    client = mqtt.Client(client_id=args.id, clean_session=not args.persist)

client.on_connect = on_connect
client.on_message = on_message

client.connect(args.broker, args.port, 60)

try:
    client.loop_forever()
except KeyboardInterrupt:
    client.disconnect()
