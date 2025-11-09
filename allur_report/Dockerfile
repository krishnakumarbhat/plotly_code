# Allure Report POC - Dockerfile
# Multi-stage build for testing and report generation

# Build stage
FROM golang:1.21-alpine AS builder

# Install build dependencies
RUN apk add --no-cache git

# Set working directory
WORKDIR /app

# Copy go mod files
COPY go.mod go.sum ./

# Download dependencies
RUN go mod download

# Copy source code
COPY . .

# Build the application
RUN go build -o allure-poc main.go

# Test stage
FROM golang:1.21-alpine AS tester

# Install test dependencies
RUN apk add --no-cache git curl

# Set working directory
WORKDIR /app

# Copy go mod files
COPY go.mod go.sum ./

# Download dependencies
RUN go mod download

# Copy source code
COPY . .

# Install Allure CLI
RUN wget -qO- https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz | tar -xz -C /opt/ && \
    ln -s /opt/allure-2.24.0/bin/allure /usr/bin/allure

# Create directories for results and reports
RUN mkdir -p /app/results /app/reports

# Expose port for Allure server
EXPOSE 8080

# Default command to run tests and generate report
CMD ["sh", "-c", "go test ./tests/... -v -json | ./allure-poc -results=/app/results -output=/app/reports -debug=true && allure serve /app/results -h 0.0.0.0 -p 8080"]

# Production stage (minimal)
FROM alpine:latest AS production

# Install runtime dependencies
RUN apk add --no-cache ca-certificates

# Set working directory
WORKDIR /app

# Copy binary from builder
COPY --from=builder /app/allure-poc .

# Create directories
RUN mkdir -p /app/results /app/reports

# Expose port
EXPOSE 8080

# Default command
CMD ["./allure-poc", "-help"] 