#!/bin/bash
set -e

# Install Docker if not present for Amazon Linux
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Installing Docker for Amazon Linux..."
    
    # Update the system
    sudo yum update -y
    
    # Install Docker
    sudo yum install -y docker
    
    # Start Docker service
    sudo service docker start
    
    # Enable Docker to start on boot
    sudo chkconfig docker on
    
    echo "Docker installed successfully."
    
    # Verify Docker installation
    sudo docker --version
fi

# Configuration
AWS_REGION="eu-central-1"
ECR_REPO_NAME="ollama-model"
MODEL_NAME="nomic-embed-text:latest"

# Set your AWS account ID explicitly
# Replace this with your actual AWS account ID
AWS_ACCOUNT_ID="116489046547"  # <-- REPLACE THIS WITH YOUR ACTUAL AWS ACCOUNT ID

ECR_REPO_URI="${AWS_ACCOUNT_ID}.dkr.ecr.${AWS_REGION}.amazonaws.com/${ECR_REPO_NAME}"

# Check for required tools
if ! command -v aws &> /dev/null; then
    echo "AWS CLI is not installed. Installing AWS CLI..."
    
    # Install AWS CLI v2 for Amazon Linux
    curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "awscliv2.zip"
    unzip awscliv2.zip
    sudo ./aws/install
    rm -rf aws awscliv2.zip
fi

# Try to start Docker if it's not running
if ! sudo docker info &> /dev/null; then
    echo "Docker is not running. Attempting to start Docker..."
    sudo service docker start
    sleep 5
    
    # Check again if Docker is running
    if ! sudo docker info &> /dev/null; then
        echo "Failed to start Docker. Please check Docker installation."
        exit 1
    fi
fi

# Step 0: Create ECR repository first to ensure it exists
echo "Step 0: Creating ECR repository if it doesn't exist..."

# Create repository and capture the output
REPO_OUTPUT=$(aws ecr create-repository --repository-name ${ECR_REPO_NAME} --region ${AWS_REGION} 2>&1 || true)

# Check if repository already exists or was created successfully
if echo "$REPO_OUTPUT" | grep -q "RepositoryAlreadyExistsException"; then
    echo "ECR repository '${ECR_REPO_NAME}' already exists."
else
    echo "ECR repository '${ECR_REPO_NAME}' created or verified."
fi

# Verify repository exists
echo "Verifying repository exists..."
aws ecr describe-repositories --repository-names ${ECR_REPO_NAME} --region ${AWS_REGION}
if [ $? -ne 0 ]; then
    echo "Failed to verify ECR repository. Please check your permissions."
    exit 1
fi

echo "Step 1: Pulling latest Ollama image..."
sudo docker pull ollama/ollama:latest

echo "Step 2: Creating a Dockerfile to bake in the Nomic model..."
cat > Dockerfile << EOF
FROM ollama/ollama:latest

# Set environment variables if needed
ENV OLLAMA_HOST=0.0.0.0
ENV OLLAMA_MODELS=/tmp/ollama/models

# Create a script to pull the model
COPY pull_model.sh /pull_model.sh
RUN chmod +x /pull_model.sh

# Run the script during build to pull the model
RUN /pull_model.sh

EXPOSE 11434

# Use the original entrypoint
ENTRYPOINT ["/bin/ollama"]
CMD ["serve"]
EOF

cat > pull_model.sh << EOF
#!/bin/bash
set -e

# Start Ollama server in the background
/bin/ollama serve &
SERVER_PID=\$!

# Wait for Ollama server to start
echo "Waiting for Ollama server to start..."
until curl -s http://localhost:11434/api/tags > /dev/null 2>&1; do
    sleep 1
done

# Pull the model
echo "Pulling ${MODEL_NAME} model..."
/bin/ollama pull ${MODEL_NAME}

# Show the pulled models
echo "Listing pulled models:"
/bin/ollama list

# Stop the Ollama server
kill \$SERVER_PID
wait \$SERVER_PID || true
EOF

echo "Step 3: Building the Docker image with the Nomic model..."
sudo docker build -t ${ECR_REPO_NAME}:latest .

echo "Step 4: Logging in to ECR..."
# Using the EC2 instance role credentials
aws ecr get-login-password --region ${AWS_REGION} | sudo docker login --username AWS --password-stdin ${ECR_REPO_URI%/*}

echo "Step 5: Tagging the Docker image for ECR..."
sudo docker tag ${ECR_REPO_NAME}:latest ${ECR_REPO_URI}:latest

echo "Step 6: Pushing the Docker image to ECR..."
sudo docker push ${ECR_REPO_URI}:latest

echo "Step 7: Cleaning up temporary files..."
rm -f Dockerfile pull_model.sh

echo "Done! The Ollama image with the Nomic model has been pushed to ${ECR_REPO_URI}:latest"