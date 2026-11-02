import paho.mqtt.client as mqtt

BROKER = "192.168.56.10"
TOPICO = "test00"

#################################################################
# Callback: la biblioteca la llama cada vez que llega un mensaje del topico
def on_message(client, userdata, msg):
    print("Topic:", msg.topic)
    print("Mensaje:", msg.payload.decode())

#################################################################
# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
else:
    client = mqtt.Client()

# Propio del asincronismo pub/sub
client.on_message = on_message

# Conectarse al broker
client.connect(BROKER, 1883, 60)

# Suscripcion al topico/evento
client.subscribe(TOPICO, qos=0)

# Lib ==> Solo esperar a que llegue el topico/evento
client.loop_forever()
