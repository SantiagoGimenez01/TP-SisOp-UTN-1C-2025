#!/bin/bash

OVERRIDES=()

if [[ -z "$1" || -z "$2" ]]; then
  echo "Uso: $0 <nombre_archivo_pseudocodigo> <tamanio_proceso> [-c=CONFIG_KEY=VALUE]"
  echo "Opcionalmente, podes usar -c=CONFIG_KEY=VALUE  para cambiar configs"
  echo "Ejemplo: $0 ESTABILIDAD_GENERAL 0 -c=ALGORITMO_PLANIFICACION=FIFO"
  exit 1
fi

PSEUDOCODE_FILE=$1
PROCESS_SIZE=$2

SOURCE_CONFIG_PATH="./configs/$1.config"
DEST_CONFIG_PATH="./kernel.config"

shift 2

while [[ "$#" -gt 0 ]]; do
  arg="$1"
  if [[ "$arg" == -c=* ]]; then
    override_pair="${arg#-c=}"
    if [[ "$override_pair" == *"="* ]]; then
      OVERRIDES+=("$override_pair")
    else
      echo "[WARN]: Formato de override '$arg' incorrecto. Se espera '-c=KEY=VALUE'."
    fi
  else
    echo "[WARN]: Argumento desconocido ignorado: '$arg'"
  fi
  shift
done

if [ ! -f "$SOURCE_CONFIG_PATH" ]; then
  echo "[ERROR]: Archivo de configuracion para $PSEUDOCODE_FILE no encontrado."
  exit 1
fi

cat "$SOURCE_CONFIG_PATH" > "$DEST_CONFIG_PATH"

if [ $? -eq 0 ]; then
  echo "[INFO] Config '$SOURCE_CONFIG_PATH' copiada con exito a '$DEST_CONFIG_PATH'."
else
  echo "[ERROR]: No se pudo copiar la config de '$SOURCE_CONFIG_PATH' a '$DEST_CONFIG_PATH'."
  exit 1
fi


if [ ${#OVERRIDES[@]} -gt 0 ]; then
  echo "[INFO] Aplicando overrides a '$DEST_CONFIG_PATH'..."
  for override in "${OVERRIDES[@]}"; do
    KEY=$(echo "$override" | cut -d'=' -f1)
    VALUE=$(echo "$override" | cut -d'=' -f2-)
    
    sed -i "s|^\($KEY\s*=\).*|\1$VALUE|" "$DEST_CONFIG_PATH"

    if [ $? -eq 0 ]; then
      echo "[INFO] Override aplicado: $KEY=$VALUE"
    else
      echo "[ERROR]: Fallo al aplicar override para $KEY. (Puede que la clave no exista en el archivo)."
    fi
  done
else
  echo "[INFO] No se detectaron overrides."
fi

make