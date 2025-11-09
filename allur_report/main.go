package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"allure-report-poc/allure"
)

func main() {
	// Parse command line flags
	var (
		resultsDir = flag.String("results", "results", "Directory for Allure results")
		outputDir  = flag.String("output", "reports", "Directory for generated reports")
		debug      = flag.Bool("debug", false, "Enable debug output")
		testPath   = flag.String("test", "./tests/...", "Test path to run")
		serve      = flag.Bool("serve", false, "Serve the report after generation")
		generate   = flag.Bool("generate", false, "Generate report from existing results")
	)
	flag.Parse()

	// Create Allure generator
	generator := allure.NewGenerator(*resultsDir, *debug)

	if *generate {
		// Generate report from existing results
		if err := generator.GenerateReport(*outputDir); err != nil {
			log.Fatalf("Failed to generate report: %v", err)
		}
		fmt.Printf("Report generated successfully in: %s\n", *outputDir)
		return
	}

	// Run tests and generate Allure results
	if err := runTestsAndGenerateResults(generator, *testPath); err != nil {
		log.Fatalf("Failed to run tests: %v", err)
	}

	// Generate report
	if err := generator.GenerateReport(*outputDir); err != nil {
		log.Fatalf("Failed to generate report: %v", err)
	}

	fmt.Printf("Allure report generated successfully!\n")
	fmt.Printf("Results directory: %s\n", *resultsDir)
	fmt.Printf("Report directory: %s\n", *outputDir)

	if *serve {
		serveReport(*resultsDir)
	} else {
		fmt.Printf("\nTo view the report, run:\n")
		fmt.Printf("  allure serve %s\n", *resultsDir)
		fmt.Printf("  or\n")
		fmt.Printf("  allure open %s\n", *outputDir)
	}
}

// runTestsAndGenerateResults runs Go tests and pipes output to Allure generator
func runTestsAndGenerateResults(generator *allure.Generator, testPath string) error {
	// Build test command
	cmd := exec.Command("go", "test", testPath, "-v", "-json")
	
	// Get stdout and stderr pipes
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return fmt.Errorf("failed to create stdout pipe: %w", err)
	}

	stderr, err := cmd.StderrPipe()
	if err != nil {
		return fmt.Errorf("failed to create stderr pipe: %w", err)
	}

	// Start the command
	if err := cmd.Start(); err != nil {
		return fmt.Errorf("failed to start test command: %w", err)
	}

	// Create a channel to signal when processing is done
	done := make(chan error, 1)

	// Process stdout in a goroutine
	go func() {
		defer stdout.Close()
		// Copy stdout to stdin for the generator
		_, err := os.Stdout.ReadFrom(stdout)
		done <- err
	}()

	// Process stderr in a goroutine
	go func() {
		defer stderr.Close()
		// Copy stderr to stderr
		_, err := os.Stderr.ReadFrom(stderr)
		if err != nil {
			done <- err
		}
	}()

	// Wait for the command to finish
	if err := cmd.Wait(); err != nil {
		return fmt.Errorf("test command failed: %w", err)
	}

	// Wait for processing to complete
	if err := <-done; err != nil {
		return fmt.Errorf("failed to process test output: %w", err)
	}

	return nil
}

// serveReport serves the Allure report using the Allure CLI
func serveReport(resultsDir string) {
	fmt.Printf("Starting Allure server...\n")
	
	cmd := exec.Command("allure", "serve", resultsDir)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		fmt.Printf("Failed to start Allure server: %v\n", err)
		fmt.Printf("Make sure Allure CLI is installed and in your PATH\n")
	}
}

// createSampleTests creates sample test files for demonstration
func createSampleTests() error {
	testsDir := "tests"
	if err := os.MkdirAll(testsDir, 0755); err != nil {
		return err
	}

	// Create calculator test
	calculatorTest := `package tests

import (
	"testing"
	"time"
)

// Calculator represents a simple calculator
type Calculator struct {
	result float64
}

// NewCalculator creates a new calculator
func NewCalculator() *Calculator {
	return &Calculator{result: 0}
}

// Add adds a number to the current result
func (c *Calculator) Add(value float64) {
	c.result += value
}

// Subtract subtracts a number from the current result
func (c *Calculator) Subtract(value float64) {
	c.result -= value
}

// Multiply multiplies the current result by a number
func (c *Calculator) Multiply(value float64) {
	c.result *= value
}

// Divide divides the current result by a number
func (c *Calculator) Divide(value float64) error {
	if value == 0 {
		return fmt.Errorf("division by zero")
	}
	c.result /= value
	return nil
}

// GetResult returns the current result
func (c *Calculator) GetResult() float64 {
	return c.result
}

// Clear resets the calculator
func (c *Calculator) Clear() {
	c.result = 0
}

func TestCalculatorAddition(t *testing.T) {
	calc := NewCalculator()
	
	// Test addition
	calc.Add(5)
	calc.Add(3)
	
	result := calc.GetResult()
	if result != 8 {
		t.Errorf("Expected 8, got %f", result)
	}
}

func TestCalculatorSubtraction(t *testing.T) {
	calc := NewCalculator()
	
	// Test subtraction
	calc.Add(10)
	calc.Subtract(3)
	
	result := calc.GetResult()
	if result != 7 {
		t.Errorf("Expected 7, got %f", result)
	}
}

func TestCalculatorMultiplication(t *testing.T) {
	calc := NewCalculator()
	
	// Test multiplication
	calc.Add(4)
	calc.Multiply(3)
	
	result := calc.GetResult()
	if result != 12 {
		t.Errorf("Expected 12, got %f", result)
	}
}

func TestCalculatorDivision(t *testing.T) {
	calc := NewCalculator()
	
	// Test division
	calc.Add(15)
	err := calc.Divide(3)
	if err != nil {
		t.Errorf("Unexpected error: %v", err)
	}
	
	result := calc.GetResult()
	if result != 5 {
		t.Errorf("Expected 5, got %f", result)
	}
}

func TestCalculatorDivisionByZero(t *testing.T) {
	calc := NewCalculator()
	
	// Test division by zero
	calc.Add(10)
	err := calc.Divide(0)
	if err == nil {
		t.Error("Expected error for division by zero")
	}
}

func TestCalculatorClear(t *testing.T) {
	calc := NewCalculator()
	
	// Test clear functionality
	calc.Add(5)
	calc.Clear()
	
	result := calc.GetResult()
	if result != 0 {
		t.Errorf("Expected 0, got %f", result)
	}
}

func TestCalculatorPerformance(t *testing.T) {
	calc := NewCalculator()
	
	// Performance test
	start := time.Now()
	for i := 0; i < 1000; i++ {
		calc.Add(1)
	}
	duration := time.Since(start)
	
	if duration > time.Millisecond*10 {
		t.Errorf("Performance test failed: operation took %v", duration)
	}
}

func TestCalculatorSkipped(t *testing.T) {
	t.Skip("This test is intentionally skipped for demonstration")
}

func TestCalculatorFlaky(t *testing.T) {
	// Simulate a flaky test (sometimes fails)
	if time.Now().UnixNano()%2 == 0 {
		t.Error("Flaky test failed")
	}
}
`

	if err := os.WriteFile(filepath.Join(testsDir, "calculator_test.go"), []byte(calculatorTest), 0644); err != nil {
		return err
	}

	// Create web test
	webTest := `package tests

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"
)

func TestWebServerHealth(t *testing.T) {
	// Create a test server
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("OK"))
	}))
	defer server.Close()

	// Test health endpoint
	resp, err := http.Get(server.URL + "/health")
	if err != nil {
		t.Fatalf("Failed to make request: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		t.Errorf("Expected status 200, got %d", resp.StatusCode)
	}
}

func TestWebServerNotFound(t *testing.T) {
	// Create a test server
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusNotFound)
		w.Write([]byte("Not Found"))
	}))
	defer server.Close()

	// Test non-existent endpoint
	resp, err := http.Get(server.URL + "/nonexistent")
	if err != nil {
		t.Fatalf("Failed to make request: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusNotFound {
		t.Errorf("Expected status 404, got %d", resp.StatusCode)
	}
}

func TestWebServerTimeout(t *testing.T) {
	// Create a test server with delay
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		time.Sleep(100 * time.Millisecond)
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("Delayed Response"))
	}))
	defer server.Close()

	// Test with timeout
	client := &http.Client{
		Timeout: 50 * time.Millisecond,
	}

	_, err := client.Get(server.URL + "/slow")
	if err == nil {
		t.Error("Expected timeout error")
	}
}

func TestWebServerConcurrent(t *testing.T) {
	// Create a test server
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("Concurrent Response"))
	}))
	defer server.Close()

	// Test concurrent requests
	done := make(chan bool, 10)
	for i := 0; i < 10; i++ {
		go func(id int) {
			resp, err := http.Get(server.URL + "/concurrent")
			if err != nil {
				t.Errorf("Request %d failed: %v", id, err)
			} else {
				resp.Body.Close()
			}
			done <- true
		}(i)
	}

	// Wait for all requests to complete
	for i := 0; i < 10; i++ {
		<-done
	}
}
`

	if err := os.WriteFile(filepath.Join(testsDir, "web_test.go"), []byte(webTest), 0644); err != nil {
		return err
	}

	return nil
}

func init() {
	// Create sample tests if they don't exist
	if _, err := os.Stat("tests"); os.IsNotExist(err) {
		if err := createSampleTests(); err != nil {
			fmt.Printf("Warning: Failed to create sample tests: %v\n", err)
		}
	}
} 