#!/bin/bash

# Allure Report POC - Test Runner Script
# This script runs Go tests and generates Allure reports

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
RESULTS_DIR="results"
OUTPUT_DIR="reports"
TEST_PATH="./tests/..."
DEBUG=false
SERVE=false

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -r, --results DIR   Results directory (default: results)"
    echo "  -o, --output DIR    Output directory (default: reports)"
    echo "  -t, --test PATH     Test path to run (default: ./tests/...)"
    echo "  -d, --debug         Enable debug output"
    echo "  -s, --serve         Serve the report after generation"
    echo "  -c, --clean         Clean previous results before running"
    echo ""
    echo "Examples:"
    echo "  $0                    # Run tests with default settings"
    echo "  $0 -d -s              # Run with debug and serve report"
    echo "  $0 -t ./tests/calculator_test.go  # Run specific test"
    echo "  $0 -c                 # Clean and run tests"
}

# Function to check if Allure is installed
check_allure() {
    if ! command -v allure &> /dev/null; then
        print_error "Allure CLI is not installed or not in PATH"
        echo ""
        echo "Install Allure CLI:"
        echo "  Windows: scoop install allure"
        echo "  macOS: brew install allure"
        echo "  Linux: See https://allurereport.org/docs/getting-started/installation/"
        exit 1
    fi
    print_success "Allure CLI found: $(allure --version)"
}

# Function to clean previous results
clean_results() {
    if [ -d "$RESULTS_DIR" ]; then
        print_status "Cleaning previous results..."
        rm -rf "$RESULTS_DIR"
    fi
    if [ -d "$OUTPUT_DIR" ]; then
        print_status "Cleaning previous reports..."
        rm -rf "$OUTPUT_DIR"
    fi
}

# Function to run tests
run_tests() {
    print_status "Running Go tests..."
    
    # Run tests with JSON output and pipe to our generator
    go test "$TEST_PATH" -v -json | go run main.go \
        -results="$RESULTS_DIR" \
        -output="$OUTPUT_DIR" \
        -test="$TEST_PATH" \
        -debug="$DEBUG"
}

# Function to serve report
serve_report() {
    if [ "$SERVE" = true ]; then
        print_status "Starting Allure server..."
        allure serve "$RESULTS_DIR"
    else
        echo ""
        print_success "Test execution completed!"
        echo ""
        echo "To view the report:"
        echo "  allure serve $RESULTS_DIR"
        echo "  or"
        echo "  allure open $OUTPUT_DIR"
        echo ""
        echo "Report files:"
        echo "  Results: $RESULTS_DIR"
        echo "  Report:  $OUTPUT_DIR"
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -r|--results)
            RESULTS_DIR="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -t|--test)
            TEST_PATH="$2"
            shift 2
            ;;
        -d|--debug)
            DEBUG=true
            shift
            ;;
        -s|--serve)
            SERVE=true
            shift
            ;;
        -c|--clean)
            clean_results
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
main() {
    print_status "Starting Allure Report POC..."
    
    # Check prerequisites
    check_allure
    
    # Check if Go is available
    if ! command -v go &> /dev/null; then
        print_error "Go is not installed or not in PATH"
        exit 1
    fi
    
    # Check if we're in a Go module
    if [ ! -f "go.mod" ]; then
        print_error "Not in a Go module. Run 'go mod init' first."
        exit 1
    fi
    
    # Download dependencies
    print_status "Downloading dependencies..."
    go mod tidy
    
    # Run tests
    run_tests
    
    # Serve report if requested
    serve_report
}

# Run main function
main 