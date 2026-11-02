import jade.core.Agent;
import jade.core.ContainerID;

import java.net.InetAddress;

// Experimento propio: ¿qué pasa si un agente hace doMove() varias veces,
// una migración después de la otra, a lo largo de su vida? Este agente
// rebota entre Main-Container y Container-1 hasta completar maxSaltos.
public class AgenteViajero extends Agent {

	private static final String MAIN = "Main-Container";
	private static final String UNO  = "Container-1";

	private int salto = 0;       // cuántas migraciones ya completó (viaja con el agente)
	private int maxSaltos = 4;   // tope para que no migre para siempre

	protected void setup() {
		Object[] args = getArguments();
		if (args != null && args.length > 0) {
			try {
				maxSaltos = Integer.parseInt(args[0].toString());
			} catch (NumberFormatException e) {
				log("arg invalido '" + args[0] + "', uso maxSaltos=" + maxSaltos);
			}
		}
		log("nazco  (maxSaltos=" + maxSaltos + ")");
		pedirSiguienteSalto();
	}

	protected void beforeMove() {
		log("beforeMove()  salto " + (salto + 1) + " en camino");
	}

	protected void afterMove() {
		salto++;
		log("afterMove()  salto=" + salto);
		if (salto < maxSaltos) {
			pedirSiguienteSalto();
		} else {
			log("listo, me quedo acá después de " + salto + " saltos");
		}
	}

	private void pedirSiguienteSalto() {
		String actual = here().getName();
		String destino = actual.equals(MAIN) ? UNO : MAIN;
		log("pido migrar de " + actual + " a " + destino);
		doMove(new ContainerID(destino, null));
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
