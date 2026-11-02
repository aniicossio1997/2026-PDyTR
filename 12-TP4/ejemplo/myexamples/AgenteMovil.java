import jade.core.Agent;
import jade.core.Location;
import jade.core.ContainerID;

public class AgenteMovil extends Agent {

	// Executed once during agent creation
	protected void setup() {
		Location origen = here();
		System.out.println("\n\nHello, agent with local name " + getLocalName());
		System.out.println("And full name... " + getName());
		System.out.println("And in location " + origen.getID() + "\n\n");

		// To migrate the agent
		try {
			ContainerID destino = new ContainerID("Main-Container", null);
			System.out.println("Migrating the agent to " + destino.getID());
			doMove(destino);
		} catch (Exception e) {
			System.err.println("Migration failed: " + e.getMessage());
		}
	}

	// Executed upon arrival at a container as a result of migration
	protected void afterMove() {
		Location origen = here();
		System.out.println("\n\nHello, migrated agent with local name - Prubea de main-container " + getLocalName());
		System.out.println("And full name... " + getName());
		System.out.println("And in location " + origen.getID() + "\n\n");
	}
}
