
#!/bin/bash

# Build Docker image
docker build -t resim-html-report .

# Convert Docker image to Singularity
singularity build resim-html-report.sif docker-daemon://resim-html-report:latest