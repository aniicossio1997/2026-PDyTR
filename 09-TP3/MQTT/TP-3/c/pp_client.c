// Ping-pong MQTT (cliente): publica B bytes en pingpong/ping, espera que
// pp_echo los devuelva en pingpong/pong y mide el tiempo de ida y vuelta.
// Una sola conexion al broker para todo el experimento.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <mosquitto.h>

#define TOPIC_PING  "pingpong/ping"
#define TOPIC_PONG  "pingpong/pong"
#define TOPIC_READY "pingpong/ready"
#define TIMEOUT_S   10

static pthread_mutex_t mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
static int subs = 0, subs_done = 0, ready = 0, got_pong = 0;
static unsigned char *pong_buf;
static long pong_cap, pong_len, pongs = 0;
static struct timespec t_pong;

/***************************************************************/
// Callback de la biblioteca (hilo de red): cuenta los SUBACK recibidos
void on_subscribe(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
  pthread_mutex_lock(&mtx);
  if (++subs == 2) subs_done = 1;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mtx);
}

/***************************************************************/
// Callback de la biblioteca (hilo de red): llega el "ready" del eco o un pong.
// t1 se toma lo primero, antes de copiar el payload.
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);

  pthread_mutex_lock(&mtx);
  if (strcmp(msg->topic, TOPIC_READY) == 0) {
    ready = msg->payloadlen > 0;   // payload vacio = el eco se fue
  } else if (strcmp(msg->topic, TOPIC_PONG) == 0) {
    if (msg->payloadlen <= pong_cap) memcpy(pong_buf, msg->payload, msg->payloadlen);
    pong_len = msg->payloadlen;
    pongs++;
    t_pong = t1;
    got_pong = 1;
  }
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mtx);
}

/***************************************************************/
// Espera (hasta seg segundos) que *flag sea distinto de 0
static int esperar(int *flag, int seg)
{
  struct timespec lim;
  int rc = 0, ok;
  clock_gettime(CLOCK_REALTIME, &lim);
  lim.tv_sec += seg;
  pthread_mutex_lock(&mtx);
  while (!*flag && rc == 0) rc = pthread_cond_timedwait(&cond, &mtx, &lim);
  ok = *flag;
  pthread_mutex_unlock(&mtx);
  return ok;
}

/***************************************************************/
int main(int argc, char *argv[])
{
  const char *broker = "192.168.56.10";
  int port = 1883, qos = 0, reps = 20, emin = 1, emax = 6, opt;

  while ((opt = getopt(argc, argv, "h:p:q:n:e:E:")) != -1) {
    switch (opt) {
      case 'h': broker = optarg; break;
      case 'p': port = atoi(optarg); break;
      case 'q': qos = atoi(optarg); break;
      case 'n': reps = atoi(optarg); break;
      case 'e': emin = atoi(optarg); break;
      case 'E': emax = atoi(optarg); break;
      default:
        fprintf(stderr, "uso: %s [-h broker] [-p puerto] [-q qos] [-n repeticiones] [-e exp_min] [-E exp_max]\n", argv[0]);
        return 1;
    }
  }
  if (emin < 1 || emax > 8 || emin > emax || qos < 0 || qos > 2) {
    fprintf(stderr, "ERROR: exponentes entre 1 y 8, qos entre 0 y 2\n");
    return 1;
  }

  long maxsize = 1;
  for (int i = 0; i < emax; i++) maxsize *= 10;
  pong_cap = maxsize;
  pong_buf = malloc(maxsize);
  unsigned char *payload = malloc(maxsize);
  if (!pong_buf || !payload) { perror("malloc"); return 1; }

  // Para usar lib mosquitto
  mosquitto_lib_init();
  char id[32];
  snprintf(id, sizeof(id), "ppcli-%d", getpid());
  struct mosquitto *mosq = mosquitto_new(id, true, NULL);
  mosquitto_subscribe_callback_set(mosq, on_subscribe);
  mosquitto_message_callback_set(mosq, on_message);

  // Una unica conexion para todo el experimento
  if (mosquitto_connect(mosq, broker, port, 60) != MOSQ_ERR_SUCCESS) {
    fprintf(stderr, "ERROR: no se pudo conectar a %s:%d\n", broker, port);
    return 1;
  }
  mosquitto_subscribe(mosq, NULL, TOPIC_PONG, qos);
  mosquitto_subscribe(mosq, NULL, TOPIC_READY, qos);

  // Hilo de la biblioteca que atiende la red y llama a los callbacks
  mosquitto_loop_start(mosq);

  // Sincronizacion de arranque: publicar antes de que el eco este suscripto
  // perderia el mensaje. El eco deja un "ready" retenido en el broker, asi que
  // no importa cual de los dos arranque primero. Queda fuera de la medicion.
  if (!esperar(&subs_done, TIMEOUT_S)) { fprintf(stderr, "ERROR: sin SUBACK del broker\n"); return 1; }
  while (!esperar(&ready, 5))
    fprintf(stderr, "eco no disponible, esperando...\n");

  printf("exp,bytes,rep,rtt_ms,rtt2_ms,ok\n");
  double suma[10] = {0};
  long enviados = 0;

  for (int e = emin; e <= emax; e++) {
    long size = 1;
    for (int i = 0; i < e; i++) size *= 10;
    for (long i = 0; i < size; i++) payload[i] = (unsigned char)(i % 256);

    for (int r = 1; r <= reps; r++) {
      pthread_mutex_lock(&mtx);
      got_pong = 0;
      pthread_mutex_unlock(&mtx);

      struct timespec t0;
      clock_gettime(CLOCK_MONOTONIC, &t0);
      // Parametros: lib, id_mensaje, topico, longitud, contenido, qos, retained
      int rc = mosquitto_publish(mosq, NULL, TOPIC_PING, size, payload, qos, false);
      if (rc != MOSQ_ERR_SUCCESS) { fprintf(stderr, "ERROR al publicar: %s\n", mosquitto_strerror(rc)); return 1; }
      enviados++;

      if (!esperar(&got_pong, TIMEOUT_S)) {
        fprintf(stderr, "ERROR: timeout esperando el pong (exp=%d rep=%d)\n", e, r);
        return 1;
      }

      // Mas pongs que pings: hay dos ecos corriendo y los tiempos no sirven
      pthread_mutex_lock(&mtx);
      long recibidos = pongs;
      pthread_mutex_unlock(&mtx);
      if (recibidos > enviados) {
        fprintf(stderr, "ERROR: %ld pongs para %ld pings (hay mas de un eco corriendo?)\n", recibidos, enviados);
        return 1;
      }

      double rtt = ((t_pong.tv_sec - t0.tv_sec) + (t_pong.tv_nsec - t0.tv_nsec) / 1e9) * 1000.0;
      int ok = (pong_len == size) && memcmp(pong_buf, payload, size) == 0;
      printf("%d,%ld,%d,%.6f,%.6f,%d\n", e, size, r, rtt, rtt / 2, ok);
      suma[e] += rtt;
    }
  }

  fprintf(stderr, "\n%-8s %-10s %-14s %-14s\n", "exp", "bytes", "RTT medio ms", "RTT/2 medio ms");
  for (int e = emin; e <= emax; e++) {
    long size = 1;
    for (int i = 0; i < e; i++) size *= 10;
    fprintf(stderr, "%-8d %-10ld %-14.6f %-14.6f\n", e, size, suma[e] / reps, suma[e] / reps / 2);
  }

  // Un segundo eco responde un poco despues del primero: dar tiempo a que llegue
  usleep(300 * 1000);
  pthread_mutex_lock(&mtx);
  long total = pongs;
  pthread_mutex_unlock(&mtx);
  if (total != enviados) {
    fprintf(stderr, "ERROR: %ld pongs para %ld pings (hay mas de un eco corriendo?), descartar los resultados\n", total, enviados);
    return 1;
  }

  mosquitto_disconnect(mosq);
  mosquitto_loop_stop(mosq, false);
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  free(pong_buf);
  free(payload);
  return 0;
}
