package allure

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"
)

// generateUUID generates a random UUID for test results
func generateUUID() string {
	bytes := make([]byte, 16)
	rand.Read(bytes)
	return hex.EncodeToString(bytes)
}

// WriteTestResult writes a test result to a JSON file in the results directory
func WriteTestResult(result *TestResult, resultsDir string) error {
	// Ensure results directory exists
	if err := os.MkdirAll(resultsDir, 0755); err != nil {
		return fmt.Errorf("failed to create results directory: %w", err)
	}

	// Generate JSON content
	jsonData, err := result.ToJSON()
	if err != nil {
		return fmt.Errorf("failed to marshal test result: %w", err)
	}

	// Create filename with UUID
	filename := fmt.Sprintf("%s-result.json", result.UUID)
	filepath := filepath.Join(resultsDir, filename)

	// Write to file
	if err := os.WriteFile(filepath, jsonData, 0644); err != nil {
		return fmt.Errorf("failed to write test result file: %w", err)
	}

	return nil
}

// WriteAttachment writes an attachment file to the results directory
func WriteAttachment(name string, data []byte, resultsDir string) (string, error) {
	// Ensure results directory exists
	if err := os.MkdirAll(resultsDir, 0755); err != nil {
		return "", fmt.Errorf("failed to create results directory: %w", err)
	}

	// Generate unique filename
	uuid := generateUUID()
	extension := getFileExtension(name)
	filename := fmt.Sprintf("%s-attachment%s", uuid, extension)
	filepath := filepath.Join(resultsDir, filename)

	// Write attachment data
	if err := os.WriteFile(filepath, data, 0644); err != nil {
		return "", fmt.Errorf("failed to write attachment file: %w", err)
	}

	return filename, nil
}

// getFileExtension extracts the file extension from a filename
func getFileExtension(filename string) string {
	ext := filepath.Ext(filename)
	if ext == "" {
		return ""
	}
	return ext
}

// GetMimeType returns the MIME type based on file extension
func GetMimeType(filename string) string {
	ext := strings.ToLower(filepath.Ext(filename))
	switch ext {
	case ".png":
		return "image/png"
	case ".jpg", ".jpeg":
		return "image/jpeg"
	case ".gif":
		return "image/gif"
	case ".svg":
		return "image/svg+xml"
	case ".txt":
		return "text/plain"
	case ".log":
		return "text/plain"
	case ".json":
		return "application/json"
	case ".xml":
		return "application/xml"
	case ".html", ".htm":
		return "text/html"
	case ".css":
		return "text/css"
	case ".js":
		return "application/javascript"
	case ".pdf":
		return "application/pdf"
	case ".zip":
		return "application/zip"
	case ".mp4":
		return "video/mp4"
	case ".avi":
		return "video/x-msvideo"
	case ".mov":
		return "video/quicktime"
	default:
		return "application/octet-stream"
	}
}

// CreateEnvironmentFile creates an environment.properties file
func CreateEnvironmentFile(resultsDir string, env map[string]string) error {
	if err := os.MkdirAll(resultsDir, 0755); err != nil {
		return fmt.Errorf("failed to create results directory: %w", err)
	}

	filepath := filepath.Join(resultsDir, "environment.properties")
	
	var content strings.Builder
	for key, value := range env {
		content.WriteString(fmt.Sprintf("%s=%s\n", key, value))
	}

	if err := os.WriteFile(filepath, []byte(content.String()), 0644); err != nil {
		return fmt.Errorf("failed to write environment file: %w", err)
	}

	return nil
}

// CreateCategoriesFile creates a categories.json file for test categorization
func CreateCategoriesFile(resultsDir string, categories []Category) error {
	if err := os.MkdirAll(resultsDir, 0755); err != nil {
		return fmt.Errorf("failed to create results directory: %w", err)
	}

	filepath := filepath.Join(resultsDir, "categories.json")
	
	jsonData, err := json.MarshalIndent(categories, "", "  ")
	if err != nil {
		return fmt.Errorf("failed to marshal categories: %w", err)
	}

	if err := os.WriteFile(filepath, jsonData, 0644); err != nil {
		return fmt.Errorf("failed to write categories file: %w", err)
	}

	return nil
}

// Category represents a test category for Allure
type Category struct {
	Name             string   `json:"name"`
	MatchedStatuses  []string `json:"matchedStatuses,omitempty"`
	MessageRegex     string   `json:"messageRegex,omitempty"`
	TraceRegex       string   `json:"traceRegex,omitempty"`
}

// DefaultCategories returns default test categories
func DefaultCategories() []Category {
	return []Category{
		{
			Name:            "Ignored tests",
			MatchedStatuses: []string{"skipped"},
		},
		{
			Name:            "Product defects",
			MatchedStatuses: []string{"failed"},
		},
		{
			Name:            "Test defects",
			MatchedStatuses: []string{"broken"},
		},
		{
			Name:            "Flaky tests",
			MatchedStatuses: []string{"failed", "broken"},
		},
	}
}

// CopyFile copies a file from source to destination
func CopyFile(src, dst string) error {
	sourceFile, err := os.Open(src)
	if err != nil {
		return fmt.Errorf("failed to open source file: %w", err)
	}
	defer sourceFile.Close()

	destFile, err := os.Create(dst)
	if err != nil {
		return fmt.Errorf("failed to create destination file: %w", err)
	}
	defer destFile.Close()

	_, err = io.Copy(destFile, sourceFile)
	if err != nil {
		return fmt.Errorf("failed to copy file: %w", err)
	}

	return nil
}

// FormatDuration formats a duration in a human-readable format
func FormatDuration(duration time.Duration) string {
	if duration < time.Millisecond {
		return fmt.Sprintf("%dns", duration.Nanoseconds())
	}
	if duration < time.Second {
		return fmt.Sprintf("%.2fms", float64(duration.Microseconds())/1000)
	}
	if duration < time.Minute {
		return fmt.Sprintf("%.2fs", duration.Seconds())
	}
	return fmt.Sprintf("%.2fm", duration.Minutes())
}

// SanitizeFilename removes invalid characters from filename
func SanitizeFilename(filename string) string {
	// Replace invalid characters with underscore
	invalidChars := []string{"<", ">", ":", "\"", "/", "\\", "|", "?", "*"}
	result := filename
	for _, char := range invalidChars {
		result = strings.ReplaceAll(result, char, "_")
	}
	return result
} 