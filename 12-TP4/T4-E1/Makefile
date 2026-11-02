# Makefile for TP4 - Ejercicio 1 (AgenteA / AgenteB)

JAVAC=javac
JAVA=java
CLASSPATH=lib/jade.jar
SRC_DIR=myexamples
OUT_DIR=classes

MAIN_IP=192.168.56.20
B_IP=192.168.56.21

compile:
	mkdir -p $(OUT_DIR)
	$(JAVAC) -classpath $(CLASSPATH) -d $(OUT_DIR) $(SRC_DIR)/AgenteA.java $(SRC_DIR)/AgenteB.java

# Terminal 1, en e1a (Main-Container). Sin "classes" a propósito: si el
# agente migrado igual corre acá, es porque el código viajó con él.
main:
	$(JAVA) -cp $(CLASSPATH) jade.Boot -local-host $(MAIN_IP)

# Terminal 2, en e1b (Container-1): arranca AgenteA, que crea a AgenteB
# y lo manda a migrar a Main-Container.
container:
	$(JAVA) -cp $(CLASSPATH):$(OUT_DIR) jade.Boot -container -host $(MAIN_IP) -local-host $(B_IP) -agents agenteA:AgenteA\(Main-Container\)

clean:
	rm -rf $(OUT_DIR)/*

.PHONY: compile main container clean
