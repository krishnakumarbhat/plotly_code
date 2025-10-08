#!/bin/bash

# Start Ollama server in the background
/bin/ollama serve &
pid=$!

# Give the server a few seconds to get up and running
sleep 5

echo "🔴 Retrieving nomic-embed-text model..."
ollama pull nomic-embed-text:latest
echo "🟢 nomic-embed-text pulled."

echo "🔴 Retrieving deepseek-r1 model..."
ollama pull deepseek-r1:1.5b
echo "🟢 deepseek-r1 pulled."

# Keep the container alive by waiting on the ollama serve process
wait $pid
