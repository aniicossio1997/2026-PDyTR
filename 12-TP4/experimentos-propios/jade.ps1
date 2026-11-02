# PowerShell para los experimentos propios (sin Vagrant: todo en Windows / localhost)
# Uso:
#   .\jade.ps1 compile
#   .\jade.ps1 main
#   .\jade.ps1 container "viajero:AgenteViajero(6)"
#   .\jade.ps1 container "doble:AgenteDobleMove"

param (
    [string]$target,
    [string]$agentSpec
)

$JAVAC = "javac"
$JAVA = "java"
$CLASSPATH = "lib\jade.jar"
$SRC_DIR = "myexamples"
$OUT_DIR = "classes"

function Compile {
    Write-Output "Compiling experimentos propios..."
    if (-not (Test-Path $OUT_DIR)) { New-Item -ItemType Directory -Path $OUT_DIR | Out-Null }
    & $JAVAC -classpath $CLASSPATH -d $OUT_DIR "$SRC_DIR\AgenteViajero.java" "$SRC_DIR\AgenteDobleMove.java"
}

function Main {
    Write-Output "Starting Main-Container (localhost, con GUI)..."
    & $JAVA -cp $CLASSPATH jade.Boot -gui
}

function ContainerRun {
    if (-not $agentSpec) {
        Write-Output "Falta el agente. Ejemplo: .\jade.ps1 container 'viajero:AgenteViajero(6)'"
        return
    }
    Write-Output "Starting Container-1 con $agentSpec ..."
    & $JAVA -cp "$CLASSPATH;$OUT_DIR" jade.Boot -container -agents $agentSpec
}

switch ($target) {
    "compile"   { Compile }
    "main"      { Main }
    "container" { ContainerRun }
    default     { Write-Output "Uso: .\jade.ps1 <compile|main|container> [agentSpec]" }
}
