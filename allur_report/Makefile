# Allure Report POC - Makefile
# Provides common commands for managing the project

.PHONY: help install test test-verbose test-debug clean report serve all

# Default target
help:
	@echo "Allure Report POC - Available Commands:"
	@echo ""
	@echo "  install     - Install dependencies"
	@echo "  test        - Run tests and generate Allure report"
	@echo "  test-verbose- Run tests with verbose output"
	@echo "  test-debug  - Run tests with debug output"
	@echo "  clean       - Clean previous results and reports"
	@echo "  report      - Generate report from existing results"
	@echo "  serve       - Serve the Allure report locally"
	@echo "  all         - Clean, test, and serve report"
	@echo "  help        - Show this help message"
	@echo ""

# Install dependencies
install:
	@echo "Installing dependencies..."
	go mod tidy
	go mod download

# Run tests and generate Allure report
test:
	@echo "Running tests and generating Allure report..."
	go test ./tests/... -v -json | go run main.go -results=results -output=reports

# Run tests with verbose output
test-verbose:
	@echo "Running tests with verbose output..."
	go test ./tests/... -v -json | go run main.go -results=results -output=reports -debug=true

# Run tests with debug output
test-debug:
	@echo "Running tests with debug output..."
	go test ./tests/... -v -json | go run main.go -results=results -output=reports -debug=true

# Clean previous results and reports
clean:
	@echo "Cleaning previous results and reports..."
	rm -rf results/
	rm -rf reports/

# Generate report from existing results
report:
	@echo "Generating report from existing results..."
	go run main.go -generate -results=results -output=reports

# Serve the Allure report locally
serve:
	@echo "Starting Allure server..."
	allure serve results/

# Clean, test, and serve report
all: clean test serve

# Run specific test
test-calculator:
	@echo "Running calculator tests..."
	go test ./tests/calculator_test.go -v -json | go run main.go -results=results -output=reports

test-web:
	@echo "Running web tests..."
	go test ./tests/web_test.go -v -json | go run main.go -results=results -output=reports

# Run tests with coverage
test-coverage:
	@echo "Running tests with coverage..."
	go test ./tests/... -v -cover -json | go run main.go -results=results -output=reports

# Run tests in parallel
test-parallel:
	@echo "Running tests in parallel..."
	go test ./tests/... -v -parallel=4 -json | go run main.go -results=results -output=reports

# Build the application
build:
	@echo "Building application..."
	go build -o allure-poc main.go

# Run the application
run:
	@echo "Running application..."
	go run main.go

# Format code
fmt:
	@echo "Formatting code..."
	go fmt ./...

# Run linter
lint:
	@echo "Running linter..."
	golangci-lint run

# Run all checks
check: fmt lint test

# Create sample tests
create-tests:
	@echo "Creating sample tests..."
	go run main.go

# Show project structure
tree:
	@echo "Project structure:"
	@tree -I 'node_modules|.git|results|reports'

# Show test statistics
stats:
	@echo "Test statistics:"
	@echo "Total tests: $$(go test ./tests/... -list=. | grep -c '^Test')"
	@echo "Calculator tests: $$(go test ./tests/calculator_test.go -list=. | grep -c '^Test')"
	@echo "Web tests: $$(go test ./tests/web_test.go -list=. | grep -c '^Test')"

# Install Allure CLI (macOS)
install-allure-mac:
	@echo "Installing Allure CLI on macOS..."
	brew install allure

# Install Allure CLI (Windows)
install-allure-win:
	@echo "Installing Allure CLI on Windows..."
	scoop install allure

# Install Allure CLI (Linux)
install-allure-linux:
	@echo "Installing Allure CLI on Linux..."
	wget -qO- https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz | sudo tar -xz -C /opt/
	sudo ln -s /opt/allure-2.24.0/bin/allure /usr/bin/allure

# Check if Allure is installed
check-allure:
	@echo "Checking Allure installation..."
	@if command -v allure >/dev/null 2>&1; then \
		echo "Allure CLI found: $$(allure --version)"; \
	else \
		echo "Allure CLI not found. Please install it:"; \
		echo "  macOS: make install-allure-mac"; \
		echo "  Windows: make install-allure-win"; \
		echo "  Linux: make install-allure-linux"; \
		exit 1; \
	fi

# Setup development environment
setup: install check-allure
	@echo "Development environment setup complete!"

# Show version information
version:
	@echo "Go version: $$(go version)"
	@echo "Go modules: $$(go list -m all | head -5)"
	@if command -v allure >/dev/null 2>&1; then \
		echo "Allure version: $$(allure --version)"; \
	else \
		echo "Allure: Not installed"; \
	fi 