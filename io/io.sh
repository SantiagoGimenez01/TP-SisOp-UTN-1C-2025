#!/bin/bash

DEST_CONFIG_PATH="./io.config"

if [[ -z "$1" ]]; then
  echo "Uso: $0 [-c=KEY=VALUE]"
  echo "Ejemplo: $0 -c=IP_KERNEL=192.168.0.11"
  exit 1
fi

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

