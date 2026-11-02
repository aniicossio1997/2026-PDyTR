# PowerShell script to compile and run Ejercicio 1 (AgenteA / AgenteB)
# Uso: .\jade.ps1 <target>   (compile, main, container)
# Pensado para la variante host + VM (ver README): la VM e1b corre
# AgenteA/AgenteB, el host (Windows) hace de Main-Container con -gui.

param (
    [string]$target
)

$JAVAC = "javac"
$JAVA = "java"
$CLASSPATH = "lib/jade.jar"
$SRC_DIR = "myexamples"
$OUT_DIR = "classes"

# IP del host en la red privada de Vagrant (ver "Variante: host + VM" del README del ejemplo)
$HOST_IP = "192.168.56.1"
$B_IP = "192.168.56.21"

function Compile {
    Write-Output "Compiling AgenteA / AgenteB..."
    if (-not (Test-Path $OUT_DIR)) { New-Item -ItemType Directory -Path $OUT_DIR | Out-Null }
    & $JAVAC -classpath $CLASSPATH -d $OUT_DIR "$SRC_DIR/AgenteA.java" "$SRC_DIR/AgenteB.java"
}

function Main {
    Write-Output "Starting Main-Container (host) with GUI..."
    & $JAVA -cp $CLASSPATH jade.Boot -gui -local-host $HOST_IP
}

# Para correr el Container-1 directamente en Windows (en vez de en la VM e1b)
function ContainerWin {
    Write-Output "Starting Container-1 on Windows..."
    & $JAVA -cp "lib/jade.jar;$OUT_DIR" jade.Boot -container -host $HOST_IP -local-host $HOST_IP -agents "agenteA:AgenteA(Main-Container)"
}

switch ($target) {
    "compile" { Compile }
    "main" { Main }
    "container" { ContainerWin }
    default { Write-Output "Usage: .\jade.ps1 <target> (compile, main, container)" }
}
