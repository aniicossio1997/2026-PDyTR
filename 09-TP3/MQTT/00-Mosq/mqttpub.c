#include <stdio.h>
#include <mosquitto.h>

int main() 
{
  // Para usar lib mosquitto
  struct mosquitto *mosq;

  // Propio de inic lib 
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, true, NULL);

  // Conectarse al broker
  mosquitto_connect(mosq, "192.168.56.10", 1883, 60);

  // Parametros: hndlr_lib, id_mensaje, topico, longitud, contenido, --, --
  mosquitto_publish(mosq, NULL, "test00", 5, "hola", 0, false);

  // Desconectarse del broker
  mosquitto_disconnect(mosq);

  // Propio de la finalizacion de la lib mosquitto
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return 0;
}
