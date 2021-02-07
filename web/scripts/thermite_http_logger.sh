#!/bin/bash

set -euo pipefail

URL_ROOT=$1

while true; do
  UNIX_DATE=$(date +"%Y-%m-%dT%H:%M:%S%z")
  curl -s -X GET "${URL_ROOT}/internalState" | jq -c ". + {unixDate: \"${UNIX_DATE}\"}"
  sleep 60
done