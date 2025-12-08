#!/bin/bash
#===============================================================================
# Build and Run Script for Docker and Singularity
# ResimHTMLReport - KPI Server & Interactive Plot Applications
#===============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

usage() {
    echo "Usage: $0 <command>"
    echo ""
    echo "Docker Commands:"
    echo "  docker-build          Build all Docker images"
    echo "  docker-build-kpi      Build KPI server Docker image"
    echo "  docker-build-plot     Build Interactive Plot Docker image"
    echo "  docker-run-kpi        Run KPI server (ZMQ mode)"
    echo "  docker-run-plot       Run Interactive Plot"
    echo "  docker-up             Start all services with docker-compose"
    echo "  docker-down           Stop all services"
    echo "  docker-logs           Show container logs"
    echo "  docker-clean          Remove all containers and images"
    echo ""
    echo "Singularity Commands:"
    echo "  singularity-build     Build both Singularity images"
    echo "  singularity-build-kpi Build KPI server Singularity image"
    echo "  singularity-build-plot Build Interactive Plot Singularity image"
    echo "  singularity-run-kpi   Run KPI server Singularity container"
    echo "  singularity-run-plot  Run Interactive Plot Singularity container"
    echo ""
    echo "Conversion Commands:"
    echo "  docker-to-singularity Convert Docker images to Singularity"
    echo ""
    echo "Utility Commands:"
    echo "  test-generator        Run KPI dummy HDF generator"
    echo "  clean-all             Remove all Docker and Singularity artifacts"
    echo ""
}

#===============================================================================
# Docker Functions
#===============================================================================

docker_build_all() {
    print_header "Building All Docker Images"
    docker-compose build
    print_success "All Docker images built successfully"
}

docker_build_kpi() {
    print_header "Building KPI Server Docker Image"
    docker build -f Dockerfile.kpi -t kpi-server:latest .
    print_success "KPI Server Docker image built: kpi-server:latest"
}

docker_build_plot() {
    print_header "Building Interactive Plot Docker Image"
    docker build -f Dockerfile.interactiveplot -t interactiveplot:latest .
    print_success "Interactive Plot Docker image built: interactiveplot:latest"
}

docker_run_kpi() {
    print_header "Running KPI Server (ZMQ Mode)"
    echo "Starting KPI server on port 5555..."
    docker run -it --rm \
        --name kpi-server \
        -p 5555:5555 \
        -v "$SCRIPT_DIR/hdf_DB:/app/hdf_DB" \
        -v "$SCRIPT_DIR/html:/app/html" \
        -v "$SCRIPT_DIR/logs:/app/logs" \
        kpi-server:latest \
        python -m KPI.kpi_server zmq 5555
}

docker_run_plot() {
    print_header "Running Interactive Plot"
    docker run -it --rm \
        --name interactiveplot \
        --network kpi-zmq-network \
        -e KPI_SERVER_HOST=kpi-server \
        -e KPI_SERVER_PORT=5555 \
        -v "$SCRIPT_DIR/hdf_DB:/app/data" \
        -v "$SCRIPT_DIR/html:/app/html" \
        -v "$SCRIPT_DIR/plots:/app/plots" \
        interactiveplot:latest \
        "$@"
}

docker_up() {
    print_header "Starting All Services with Docker Compose"
    docker-compose up -d
    echo ""
    echo "Services started:"
    docker-compose ps
    print_success "All services running"
}

docker_down() {
    print_header "Stopping All Services"
    docker-compose down
    print_success "All services stopped"
}

docker_logs() {
    docker-compose logs -f
}

docker_clean() {
    print_header "Cleaning Docker Resources"
    docker-compose down --rmi all --volumes --remove-orphans 2>/dev/null || true
    docker image prune -f
    print_success "Docker resources cleaned"
}

#===============================================================================
# Singularity Functions
#===============================================================================

singularity_build_kpi() {
    print_header "Building KPI Server Singularity Image"
    
    if ! command -v singularity &> /dev/null; then
        print_error "Singularity is not installed"
        exit 1
    fi
    
    # Build from definition file
    sudo singularity build kpi_server.sif singularity_kpi.def
    print_success "KPI Server Singularity image built: kpi_server.sif"
}

singularity_build_plot() {
    print_header "Building Interactive Plot Singularity Image"
    
    if ! command -v singularity &> /dev/null; then
        print_error "Singularity is not installed"
        exit 1
    fi
    
    # Build from definition file
    sudo singularity build interactiveplot.sif singularity_interactiveplot.def
    print_success "Interactive Plot Singularity image built: interactiveplot.sif"
}

singularity_build_all() {
    singularity_build_kpi
    singularity_build_plot
}

singularity_run_kpi() {
    print_header "Running KPI Server Singularity Container"
    
    if [ ! -f "kpi_server.sif" ]; then
        print_error "kpi_server.sif not found. Run 'singularity-build-kpi' first"
        exit 1
    fi
    
    echo "Starting KPI server on port 5555..."
    singularity run \
        -B "$SCRIPT_DIR/hdf_DB:/app/hdf_DB" \
        -B "$SCRIPT_DIR/html:/app/html" \
        -B "$SCRIPT_DIR/logs:/app/logs" \
        kpi_server.sif
}

singularity_run_plot() {
    print_header "Running Interactive Plot Singularity Container"
    
    if [ ! -f "interactiveplot.sif" ]; then
        print_error "interactiveplot.sif not found. Run 'singularity-build-plot' first"
        exit 1
    fi
    
    singularity run \
        -B "$SCRIPT_DIR/hdf_DB:/app/data" \
        -B "$SCRIPT_DIR/html:/app/html" \
        -B "$SCRIPT_DIR/plots:/app/plots" \
        interactiveplot.sif "$@"
}

#===============================================================================
# Docker to Singularity Conversion
#===============================================================================

docker_to_singularity() {
    print_header "Converting Docker Images to Singularity"
    
    if ! command -v singularity &> /dev/null; then
        print_error "Singularity is not installed"
        exit 1
    fi
    
    # First ensure Docker images are built
    docker_build_all
    
    # Convert KPI Server
    echo "Converting kpi-server:latest to Singularity..."
    singularity build kpi_server.sif docker-daemon://kpi-server:latest
    print_success "Created kpi_server.sif from Docker image"
    
    # Convert Interactive Plot
    echo "Converting interactiveplot:latest to Singularity..."
    singularity build interactiveplot.sif docker-daemon://interactiveplot:latest
    print_success "Created interactiveplot.sif from Docker image"
    
    print_success "Docker to Singularity conversion complete"
    echo ""
    echo "Generated files:"
    ls -lh *.sif 2>/dev/null || echo "No .sif files found"
}

#===============================================================================
# Utility Functions
#===============================================================================

test_generator() {
    print_header "Running KPI Dummy HDF Generator"
    docker-compose --profile tools run --rm kpi-generator
}

clean_all() {
    print_header "Cleaning All Resources"
    docker_clean
    rm -f *.sif 2>/dev/null || true
    print_success "All resources cleaned"
}

#===============================================================================
# Main
#===============================================================================

case "${1:-}" in
    # Docker commands
    docker-build)
        docker_build_all
        ;;
    docker-build-kpi)
        docker_build_kpi
        ;;
    docker-build-plot)
        docker_build_plot
        ;;
    docker-run-kpi)
        docker_run_kpi
        ;;
    docker-run-plot)
        shift
        docker_run_plot "$@"
        ;;
    docker-up)
        docker_up
        ;;
    docker-down)
        docker_down
        ;;
    docker-logs)
        docker_logs
        ;;
    docker-clean)
        docker_clean
        ;;
    
    # Singularity commands
    singularity-build)
        singularity_build_all
        ;;
    singularity-build-kpi)
        singularity_build_kpi
        ;;
    singularity-build-plot)
        singularity_build_plot
        ;;
    singularity-run-kpi)
        singularity_run_kpi
        ;;
    singularity-run-plot)
        shift
        singularity_run_plot "$@"
        ;;
    
    # Conversion commands
    docker-to-singularity)
        docker_to_singularity
        ;;
    
    # Utility commands
    test-generator)
        test_generator
        ;;
    clean-all)
        clean_all
        ;;
    
    *)
        usage
        exit 1
        ;;
esac
