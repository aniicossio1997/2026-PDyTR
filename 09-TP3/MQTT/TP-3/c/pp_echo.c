// Ping-pong MQTT (eco): se suscribe a pingpong/ping y devuelve cada mensaje,
// sin tocarlo, en pingpong/pong. No conoce el tamano: sirve para cualquier B.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mosquitto.h>

#define TOPIC_PING  "pingpong/ping"
#define TOPIC_PONG  "pingpong/pong"
#define TOPIC_READY "pingpong/ready"

static volatile sig_atomic_t run = 1;
static volatile int cleared = 0;
static int qos = 0;

/***************************************************************/
void on_connect(struct mosquitto *mosq, void *userdata, int rc)
{
  // Se suscribe al conectar, asi al reconectarse vuelve a suscribirse
  if (rc == 0) mosquitto_subscribe(mosq, NULL, TOPIC_PING, qos);
}

/***************************************************************/
// Ya suscripto: avisa que esta listo. El mensaje queda retenido en el broker,
// asi el cliente lo recibe aunque arranque despues.
void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
  mosquitto_publish(mosq, NULL, TOPIC_READY, 5, "ready", 1, true);
  fprintf(stderr, "eco listo (QoS %d)\n", qos);
}

/***************************************************************/
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
  mosquitto_publish(mosq, NULL, TOPIC_PONG, msg->payloadlen, msg->payload, qos, false);
}

/***************************************************************/
void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
  if (!run) cleared = 1;
}

void on_signal(int sig)
{
  run = 0;
}

/***************************************************************/
int main(int argc, char *argv[])
{
  const char *broker = "192.168.56.10";
  int port = 1883, opt;

  while ((opt = getopt(argc, argv, "h:p:q:")) != -1) {
    switch (opt) {
      case 'h': broker = optarg; break;
      case 'p': port = atoi(optarg); break;
      case 'q': qos = atoi(optarg); break;
      default:
        fprintf(stderr, "uso: %s [-h broker] [-p puerto] [-q qos]\n", argv[0]);
        return 1;
    }
  }
  if (qos < 0 || qos > 2) { fprintf(stderr, "ERROR: qos entre 0 y 2\n"); return 1; }

  signal(SIGINT, on_signal);
  signal(SIGTERM, on_signal);

  // Para usar lib mosquitto
  mosquitto_lib_init();
  struct mosquitto *mosq = mosquitto_new("ppeco", true, NULL);
  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_subscribe_callback_set(mosq, on_subscribe);
  mosquitto_message_callback_set(mosq, on_message);
  mosquitto_publish_callback_set(mosq, on_publish);

  // Last will: si el eco muere sin avisar, el broker borra el "ready" retenido
  // (payload vacio + retained) y ningun cliente cree que sigue vivo.
  mosquitto_will_set(mosq, TOPIC_READY, 0, NULL, 1, true);

  if (mosquitto_connect(mosq, broker, port, 60) != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "ERROR: no se pudo conectar a %s:%d\n", broker, port);
    return 1;
  }

  while (run) {
    int rc = mosquitto_loop(mosq, 100, 1);
    if (rc != MOSQ_ERR_SUCCESS && run) {
      sleep(1);
      mosquitto_reconnect(mosq);
    }
  }

  // Salida ordenada: el will no se publica, asi que se borra el "ready" a mano
  mosquitto_publish(mosq, NULL, TOPIC_READY, 0, NULL, 1, true);
  for (int i = 0; i < 40 && !cleared; i++) mosquitto_loop(mosq, 50, 1);

  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  return 0;
}
