#!/bin/bash
# trigger change
#docker compose up --build --force-recreate -d
docker compose down -v --rmi all; docker compose up --build -d --force-recreate