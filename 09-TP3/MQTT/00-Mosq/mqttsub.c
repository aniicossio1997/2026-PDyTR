#include <stdio.h>
#include <mosquitto.h>

/***************************************************************/
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
  printf("Topic: %s\n", msg->topic);
  printf("Mensaje: %s\n", (char *)msg->payload);
}

/***************************************************************/
int main()
{
  // Para usar lib mosquitto
  struct mosquitto *mosq;

  // Propio de inic lib
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, true, NULL);

  // Propio del asincronismo pub/sub
  mosquitto_message_callback_set(mosq, on_message);

  // Conectarse al broker
  mosquitto_connect(mosq, "192.168.56.10", 1883, 60);

  // Suscripcion al topico/evento
  mosquitto_subscribe(mosq, NULL, "test00", 0);

  // Lib ==> Solo esperar a que llegue el topico/evento
  mosquitto_loop_forever(mosq, -1, 1);

  // Nunca llega a este punto, seria lo propio de la finalizacion de la lib mosquitto
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return 0;
}

