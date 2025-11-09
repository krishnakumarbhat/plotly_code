# Allure Report POC in Go

This is a Proof of Concept (POC) demonstrating how to integrate Allure Report with Go tests. The project shows how to generate Allure-compatible test results and build beautiful HTML reports.

## What is Allure Report?

Allure Report is an open-source tool for visualizing test results. It provides:
- Beautiful HTML reports that can be opened anywhere
- Support for multiple test frameworks and languages
- Advanced features like test steps, attachments, and defect categories
- Timeline analysis and test stability tracking

## Project Structure

```
allure-report-poc/
├── go.mod                 # Go module file
├── go.sum                 # Go dependencies
├── README.md             # This file
├── main.go               # Main application with sample tests
├── allure/               # Allure report utilities
│   ├── generator.go      # Allure report generator
│   ├── models.go         # Allure data models
│   └── utils.go          # Utility functions
├── tests/                # Sample test files
│   ├── calculator_test.go
│   └── web_test.go
├── results/              # Generated test results (JUnit XML)
└── reports/              # Generated Allure HTML reports
```

## Features Demonstrated

1. **JUnit XML Generation**: Convert Go test results to JUnit XML format
2. **Allure Report Generation**: Generate beautiful HTML reports from test results
3. **Test Steps**: Demonstrate how to add detailed test steps
4. **Attachments**: Show how to attach screenshots, logs, and other files
5. **Test Categories**: Organize tests by severity and type
6. **Parallel Test Execution**: Support for running tests in parallel

## Prerequisites

1. **Go 1.21+**: Make sure you have Go installed
2. **Allure Command Line Tool**: Install Allure Report CLI

### Installing Allure Report

#### Windows (using Scoop):
```bash
scoop install allure
```

#### macOS (using Homebrew):
```bash
brew install allure
```

#### Linux:
```bash
# Add repository
wget -qO- https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz | tar -xz -C /opt/
sudo ln -s /opt/allure-2.24.0/bin/allure /usr/bin/allure
```

## Quick Start

1. **Install dependencies**:
   ```bash
   go mod tidy
   ```

2. **Run tests and generate JUnit XML**:
   ```bash
   go test ./tests/... -v -json | go run main.go
   ```

3. **Generate Allure Report**:
   ```bash
   allure serve results/
   ```

4. **Open the report**:
   The report will automatically open in your default browser at `http://localhost:8080`

## Usage Examples

### Basic Test Execution
```bash
# Run all tests
go test ./tests/... -v

# Run specific test
go test ./tests/calculator_test.go -v

# Run tests with coverage
go test ./tests/... -v -cover
```

### Generate Allure Report
```bash
# Generate report from existing results
allure generate results/ -o reports/

# Serve report locally
allure serve results/

# Generate and open report
allure generate results/ -o reports/ && allure open reports/
```

## Sample Test Structure

The POC includes several types of tests:

1. **Unit Tests**: Basic calculator operations
2. **Integration Tests**: Web API testing
3. **Performance Tests**: Response time validation
4. **Parallel Tests**: Concurrent test execution

## Allure Report Features

### Test Steps
Tests are broken down into logical steps for better readability:
```go
func TestCalculator(t *testing.T) {
    allure.Step("Initialize calculator", func() {
        calc := NewCalculator()
        // test logic
    })
}
```

### Attachments
Add screenshots, logs, or any files to test results:
```go
allure.Attachment("screenshot.png", "image/png", screenshotBytes)
allure.Attachment("test.log", "text/plain", logBytes)
```

### Test Categories
Organize tests by severity and type:
```go
allure.Severity(allure.Critical)
allure.Feature("Calculator")
allure.Story("Basic Operations")
```

## Customization

### Environment Configuration
Create `environment.properties` in the results directory:
```properties
Browser=Chrome
Version=1.0.0
Environment=Staging
```

### Categories Configuration
Create `categories.json` to define test categories:
```json
[
  {
    "name": "Ignored tests",
    "matchedStatuses": ["skipped"]
  },
  {
    "name": "Product defects",
    "matchedStatuses": ["failed"]
  }
]
```

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: Tests with Allure Report
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-go@v4
        with:
          go-version: '1.21'
      - run: go mod tidy
      - run: go test ./tests/... -v -json | go run main.go
      - uses: simple-elf/allure-report-action@master
        with:
          allure_results: results/
          allure_report: reports/
```

## Troubleshooting

### Common Issues

1. **Allure command not found**: Make sure Allure is installed and in your PATH
2. **Empty reports**: Ensure test results are generated in the correct format
3. **Permission errors**: Check file permissions for results and reports directories

### Debug Mode
Run with verbose output:
```bash
go test ./tests/... -v -json | go run main.go -debug
```

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## References

- [Allure Report Documentation](https://allurereport.org/docs/)
- [Allure GitHub Repository](https://github.com/allure-framework/allure2)
- [Go Testing Package](https://golang.org/pkg/testing/) 