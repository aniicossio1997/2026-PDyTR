import paho.mqtt.client as mqtt

BROKER = "192.168.56.10"
TOPICO = "test00"

# paho 2.x exige indicar la version de callbacks; paho 1.x (Ubuntu 22.04) no la tiene
if hasattr(mqtt, "CallbackAPIVersion"):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
else:
    client = mqtt.Client()

# Conectarse al broker
client.connect(BROKER, 1883, 60)

# Hilo de la biblioteca que resuelve la comunicacion con el broker
client.loop_start()

# Parametros: topico, contenido, QoS
info = client.publish(TOPICO, "hola", qos=0)

# Esperar a que la publicacion quede resuelta con el broker (no hay equivalente en mosquitto-C)
info.wait_for_publish()

# Terminar el hilo y desconectarse del broker
client.loop_stop()
client.disconnect()
