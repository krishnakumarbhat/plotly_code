package allure

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// TestEvent represents a Go test event from JSON output
type TestEvent struct {
	Time    time.Time `json:"Time"`
	Action  string    `json:"Action"`
	Package string    `json:"Package"`
	Test    string    `json:"Test"`
	Elapsed float64   `json:"Elapsed"`
	Output  string    `json:"Output"`
}

// Generator handles the conversion of Go test results to Allure format
type Generator struct {
	resultsDir string
	debug      bool
}

// NewGenerator creates a new Allure report generator
func NewGenerator(resultsDir string, debug bool) *Generator {
	return &Generator{
		resultsDir: resultsDir,
		debug:      debug,
	}
}

// ProcessTestOutput processes Go test JSON output and generates Allure results
func (g *Generator) ProcessTestOutput() error {
	// Create results directory
	if err := os.MkdirAll(g.resultsDir, 0755); err != nil {
		return fmt.Errorf("failed to create results directory: %w", err)
	}

	// Create default environment file
	env := map[string]string{
		"Language":   "Go",
		"Framework":  "testing",
		"Version":    "1.0.0",
		"Environment": "Development",
		"Platform":   "All",
	}
	if err := CreateEnvironmentFile(g.resultsDir, env); err != nil {
		return fmt.Errorf("failed to create environment file: %w", err)
	}

	// Create default categories file
	if err := CreateCategoriesFile(g.resultsDir, DefaultCategories()); err != nil {
		return fmt.Errorf("failed to create categories file: %w", err)
	}

	// Process test events from stdin
	scanner := bufio.NewScanner(os.Stdin)
	var currentTest *TestResult
	var testOutput strings.Builder

	for scanner.Scan() {
		line := scanner.Text()
		
		// Try to parse as JSON test event
		var event TestEvent
		if err := json.Unmarshal([]byte(line), &event); err != nil {
			// If not JSON, treat as output for current test
			if currentTest != nil {
				testOutput.WriteString(line)
				testOutput.WriteString("\n")
			}
			continue
		}

		// Process test event
		switch event.Action {
		case "run":
			// Start of a test
			if currentTest != nil {
				// Save previous test if exists
				g.saveTestResult(currentTest, testOutput.String())
			}
			
			currentTest = NewTestResult(event.Test, fmt.Sprintf("%s.%s", event.Package, event.Test))
			currentTest.Start = event.Time.UnixMilli()
			currentTest.AddLabel(LabelPackage, event.Package)
			currentTest.AddLabel(LabelTestMethod, event.Test)
			currentTest.AddLabel(LabelLanguage, "go")
			currentTest.AddLabel(LabelFramework, "testing")
			
			testOutput.Reset()

		case "pass":
			// Test passed
			if currentTest != nil {
				currentTest.SetStatus(StatusPassed, "", "")
				currentTest.SetDuration(time.Duration(event.Elapsed * float64(time.Second)))
				g.saveTestResult(currentTest, testOutput.String())
				currentTest = nil
			}

		case "fail":
			// Test failed
			if currentTest != nil {
				currentTest.SetStatus(StatusFailed, testOutput.String(), "")
				currentTest.SetDuration(time.Duration(event.Elapsed * float64(time.Second)))
				g.saveTestResult(currentTest, testOutput.String())
				currentTest = nil
			}

		case "skip":
			// Test skipped
			if currentTest != nil {
				currentTest.SetStatus(StatusSkipped, testOutput.String(), "")
				currentTest.SetDuration(time.Duration(event.Elapsed * float64(time.Second)))
				g.saveTestResult(currentTest, testOutput.String())
				currentTest = nil
			}

		case "output":
			// Test output
			if currentTest != nil {
				testOutput.WriteString(event.Output)
			}
		}
	}

	// Save any remaining test
	if currentTest != nil {
		g.saveTestResult(currentTest, testOutput.String())
	}

	if g.debug {
		fmt.Printf("Allure results generated in: %s\n", g.resultsDir)
	}

	return nil
}

// saveTestResult saves a test result and its attachments
func (g *Generator) saveTestResult(test *TestResult, output string) {
	// Add output as attachment if not empty
	if strings.TrimSpace(output) != "" {
		outputBytes := []byte(output)
		filename, err := WriteAttachment("test-output.txt", outputBytes, g.resultsDir)
		if err == nil {
			test.AddAttachment("Test Output", "text/plain", filename)
		}
	}

	// Add test steps based on output analysis
	g.addTestSteps(test, output)

	// Write test result
	if err := WriteTestResult(test, g.resultsDir); err != nil {
		if g.debug {
			fmt.Printf("Error writing test result: %v\n", err)
		}
	}
}

// addTestSteps analyzes test output and adds logical steps
func (g *Generator) addTestSteps(test *TestResult, output string) {
	lines := strings.Split(output, "\n")
	var currentStep *Step
	var stepOutput strings.Builder

	for _, line := range lines {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		// Detect step patterns
		if g.isStepStart(line) {
			// Save previous step if exists
			if currentStep != nil {
				g.finalizeStep(currentStep, stepOutput.String())
				test.AddStep(*currentStep)
			}

			// Start new step
			stepName := g.extractStepName(line)
			currentStep = &Step{
				Name:     stepName,
				Status:   StatusPassed,
				Stage:    StageFinished,
				Start:    time.Now().UnixMilli(),
				Stop:     time.Now().UnixMilli(),
				Duration: 0,
			}
			stepOutput.Reset()
		} else if currentStep != nil {
			stepOutput.WriteString(line)
			stepOutput.WriteString("\n")
		}
	}

	// Save final step
	if currentStep != nil {
		g.finalizeStep(currentStep, stepOutput.String())
		test.AddStep(*currentStep)
	}
}

// isStepStart checks if a line indicates the start of a test step
func (g *Generator) isStepStart(line string) bool {
	stepPatterns := []string{
		"=== RUN",
		"--- PASS:",
		"--- FAIL:",
		"--- SKIP:",
		"Step:",
		"Given",
		"When",
		"Then",
		"And",
		"But",
	}

	for _, pattern := range stepPatterns {
		if strings.Contains(line, pattern) {
			return true
		}
	}

	return false
}

// extractStepName extracts a meaningful step name from a line
func (g *Generator) extractStepName(line string) string {
	// Remove common prefixes
	line = strings.TrimPrefix(line, "=== RUN ")
	line = strings.TrimPrefix(line, "--- PASS: ")
	line = strings.TrimPrefix(line, "--- FAIL: ")
	line = strings.TrimPrefix(line, "--- SKIP: ")
	line = strings.TrimPrefix(line, "Step: ")

	// Clean up the name
	line = strings.TrimSpace(line)
	if len(line) > 100 {
		line = line[:97] + "..."
	}

	return line
}

// finalizeStep finalizes a step with output and duration
func (g *Generator) finalizeStep(step *Step, output string) {
	step.Stop = time.Now().UnixMilli()
	step.Duration = step.Stop - step.Start

	// Add output as attachment if not empty
	if strings.TrimSpace(output) != "" {
		outputBytes := []byte(output)
		filename, err := WriteAttachment(fmt.Sprintf("step-%s-output.txt", step.Name), outputBytes, g.resultsDir)
		if err == nil {
			step.Attachments = append(step.Attachments, Attachment{
				Name:   "Step Output",
				Type:   "text/plain",
				Source: filename,
			})
		}
	}
}

// GenerateReport generates an Allure report from existing results
func (g *Generator) GenerateReport(outputDir string) error {
	// Check if results directory exists
	if _, err := os.Stat(g.resultsDir); os.IsNotExist(err) {
		return fmt.Errorf("results directory does not exist: %s", g.resultsDir)
	}

	// Create output directory
	if err := os.MkdirAll(outputDir, 0755); err != nil {
		return fmt.Errorf("failed to create output directory: %w", err)
	}

	// Copy results to output directory
	if err := g.copyResults(g.resultsDir, outputDir); err != nil {
		return fmt.Errorf("failed to copy results: %w", err)
	}

	if g.debug {
		fmt.Printf("Report generated in: %s\n", outputDir)
		fmt.Printf("To view the report, run: allure serve %s\n", outputDir)
	}

	return nil
}

// copyResults copies all result files to the output directory
func (g *Generator) copyResults(src, dst string) error {
	return filepath.Walk(src, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		// Calculate relative path
		relPath, err := filepath.Rel(src, path)
		if err != nil {
			return err
		}

		dstPath := filepath.Join(dst, relPath)

		if info.IsDir() {
			return os.MkdirAll(dstPath, info.Mode())
		}

		return CopyFile(path, dstPath)
	})
}

// ParseJUnitXML parses JUnit XML results and converts to Allure format
func (g *Generator) ParseJUnitXML(xmlFile string) error {
	// This would implement JUnit XML parsing
	// For now, we'll use the JSON output method
	return fmt.Errorf("JUnit XML parsing not implemented yet")
} 