# PowerShell script to compile and run Java programs with JADE

# Parametros
param (
    [string]$target
)

# Variables
$JAVAC = "javac"
$JAVA = "java"
$CLASSPATH = "lib/jade.jar"
$SRC_DIR = "myexamples"
$OUT_DIR = "classes"
$MAIN_CLASS = "AgenteMovil"

# Function to compile Java code
function Compile {
    Write-Output "Compiling Java code..."
    & $JAVAC -classpath $CLASSPATH -d $OUT_DIR "$SRC_DIR/$MAIN_CLASS.java"
}

# Function to run JADE in GUI mode
function Gui {
    Write-Output "Starting JADE with GUI..."
    & $JAVA -cp $CLASSPATH jade.Boot -gui
}

# Function to run an agent on Windows
function Agente_Win {
    Write-Output "Starting JADE agent on Windows..."
    & $JAVA -cp "lib/jade.jar;$OUT_DIR" jade.Boot -gui -container -host localhost -agents "mol:$MAIN_CLASS"
}

# Main logic to call functions based on input parameter
switch ($target) {
    "compile" { Compile }
    "gui" { Gui }
    "agente" { Agente_Win }
    default { Write-Output "Usage: .\jade.ps1 <target> (compile, gui, agente)" }
}
