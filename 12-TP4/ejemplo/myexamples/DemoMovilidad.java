import jade.core.Agent;
import jade.core.ContainerID;

import java.net.InetAddress;

// Demo para la Expl. 4: muestra qué viaja y qué no cuando un agente migra.
public class DemoMovilidad extends Agent {

	private int contador = 0;           // atributo: viaja con el agente
	private transient String nota;      // transient: no se serializa, no viaja

	protected void setup() {
		int local = 42;                 // variable local: vive solo en este método
		contador++;
		nota = "anotado en el origen";

		log("setup()  contador=" + contador + "  local=" + local + "  nota=" + nota);

		//doMove(new ContainerID("Main-Container", null));
		
		// Este es un ejemplo de migración a un contenedor que no existe: se lanza una excepción
		// y el agente sigue en el contenedor de origen.
		doMove(new ContainerID("Main-Container", null));

		// doMove() solo pide la migración: esta línea todavía corre en el origen
		log("línea siguiente a doMove()");
	}

	protected void afterMove() {
		contador++;
		log("afterMove()  contador=" + contador + "  nota=" + nota);
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
