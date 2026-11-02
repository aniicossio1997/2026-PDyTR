import argparse
import time
import paho.mqtt.client as mqtt

# Igual que pub1.py pero parametrizado
parser = argparse.ArgumentParser(description="Publicador MQTT")
parser.add_argument("-b", "--broker", default="192.168.56.10", help="IP o nombre del broker")
parser.add_argument("-p", "--port", type=int, default=1883, help="puerto del broker")
parser.add_argument("-t", "--topic", default="test00", help="topico")
parser.add_argument("-m", "--message", default="hola", help="contenido del mensaje")
parser.add_argument("-q", "--qos", type=int, choices=[0, 1, 2], default=0, help="QoS de la publicacion")
parser.add_argument("-n", "--count", type=int, default=1, help="cantidad de mensajes")
parser.add_argument("-d", "--delay", type=float, default=0, help="segundos entre mensajes")
parser.add_argument("-r", "--retain", action="store_true", help="el broker guarda el ultimo mensaje del topico")
args = parser.parse_args()

# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
else:
    client = mqtt.Client()

client.connect(args.broker, args.port, 60)

# Hilo de la biblioteca que resuelve la comunicacion con el broker
client.loop_start()

for i in range(args.count):
    texto = args.message if args.count == 1 else f"{args.message} {i + 1}"
    info = client.publish(args.topic, texto, qos=args.qos, retain=args.retain)
    info.wait_for_publish()
    print(f"Publicado en '{args.topic}' (QoS {args.qos}): {texto}")
    if args.delay:
        time.sleep(args.delay)

client.loop_stop()
client.disconnect()
