import jade.core.Agent;
import jade.core.ContainerID;

import java.net.InetAddress;

// TP4 - Ejercicio 1.
// AgenteB recibe por parámetro el nombre del contenedor destino, pide la
// migración en setup() y, una vez que llega, avisa en afterMove() que la
// migración se completó.
public class AgenteB extends Agent {

	protected void setup() {
		Object[] args = getArguments();
		String destino = (args != null && args.length > 0) ? (String) args[0] : "Container-1";

		log("setup()  nazco en " + here().getName() + "  pido migrar a " + destino);
		doMove(new ContainerID(destino, null));
		// doMove() solo deja pedida la migración: se concreta al terminar setup().
	}

	protected void afterMove() {
		log("afterMove()  migración completada: ahora estoy en " + here().getName());
	}

	private void log(String msg) {
		String host;
		try {
			host = InetAddress.getLocalHost().getHostName();
		} catch (Exception e) {
			host = "?";
		}
		System.out.println("[" + host + " | " + here().getName() + " | AgenteB] " + msg);
	}
}
