#!/bin/bash
cd app

# starting api
echo "Starting API..."
xterm -T "API" -hold -e "python3 api.py" &
sleep 1
# staring webapp
echo "Starting Webapp..."
xterm -T "Webapp" -hold -e "python3 app.py" &

read -p "Press enter to close webapp and api"
echo "Closing.."
killall xterm
killall python3
echo "Closed."