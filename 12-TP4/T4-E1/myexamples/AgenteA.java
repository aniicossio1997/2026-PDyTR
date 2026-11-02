import jade.core.Agent;
import jade.wrapper.AgentController;
import jade.wrapper.StaleProxyException;

// TP4 - Ejercicio 1.
// AgenteA crea un AgenteB en su mismo contenedor y le indica a qué
// contenedor tiene que migrar. AgenteA no se mueve: solo dispara la migración.
public class AgenteA extends Agent {

	protected void setup() {
		Object[] args = getArguments();
		String destino = (args != null && args.length > 0) ? (String) args[0] : "Main-Container";

		log("nace en " + here().getName() + "  ->  voy a crear a AgenteB y mandarlo a " + destino);

		try {
			AgentController ac = getContainerController().createNewAgent(
					"agenteB", "AgenteB", new Object[] { destino });
			ac.start();
			log("AgenteB creado y arrancado");
		} catch (StaleProxyException e) {
			System.err.println("[AgenteA] No se pudo crear/arrancar a AgenteB: " + e.getMessage());
		}
	}

	private void log(String msg) {
		System.out.println("[AgenteA | " + here().getName() + "] " + msg);
	}
}
