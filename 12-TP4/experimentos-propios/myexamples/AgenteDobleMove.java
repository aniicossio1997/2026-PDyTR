import jade.core.Agent;
import jade.core.ContainerID;

import java.net.InetAddress;

// Experimento propio: ¿qué pasa si pido doMove() dos veces seguidas, sin
// ceder el control entre una y otra (sin esperar a que la primera migración
// se concrete)? ¿Gana la primera petición, la segunda, o tira error?
public class AgenteDobleMove extends Agent {

	protected void setup() {
		log("nazco, pido doMove() dos veces seguidas: primero a Container-1, después a Main-Container");
		doMove(new ContainerID("Container-1", null));
		doMove(new ContainerID("Main-Container", null));
		log("las dos migraciones ya fueron pedidas, todavía sigo en el origen (" + here().getName() + ")");
	}

	protected void afterMove() {
		log("afterMove()  terminé en este contenedor");
	}

	private void log(String msg) {
		String host;
		try {
			host = InetAddress.getLocalHost().getHostName();
		} catch (Exception e) {
			host = "?";
		}
		System.out.println("[" + host + " | " + here().getName() + "] " + msg);
	}
}
