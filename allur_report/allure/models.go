package allure

import (
	"encoding/json"
	"time"
)

// TestResult represents a single test result in Allure format
type TestResult struct {
	UUID          string                 `json:"uuid"`
	Name          string                 `json:"name"`
	FullName      string                 `json:"fullName"`
	Status        string                 `json:"status"`
	StatusDetails *StatusDetails         `json:"statusDetails,omitempty"`
	Stage         string                 `json:"stage"`
	Description   string                 `json:"description,omitempty"`
	DescriptionHtml string               `json:"descriptionHtml,omitempty"`
	Start         int64                  `json:"start"`
	Stop          int64                  `json:"stop"`
	Duration      int64                  `json:"duration"`
	Steps         []Step                 `json:"steps,omitempty"`
	Attachments   []Attachment           `json:"attachments,omitempty"`
	Parameters    []Parameter            `json:"parameters,omitempty"`
	Labels        []Label                `json:"labels,omitempty"`
	Links         []Link                 `json:"links,omitempty"`
	HistoryID     string                 `json:"historyId,omitempty"`
	TestCaseID    string                 `json:"testCaseId,omitempty"`
	Extra         map[string]interface{} `json:"extra,omitempty"`
}

// StatusDetails provides additional information about test status
type StatusDetails struct {
	Known   bool   `json:"known"`
	Muted   bool   `json:"muted"`
	Flaky   bool   `json:"flaky"`
	Message string `json:"message,omitempty"`
	Trace   string `json:"trace,omitempty"`
}

// Step represents a test step
type Step struct {
	Name          string                 `json:"name"`
	Status        string                 `json:"status"`
	StatusDetails *StatusDetails         `json:"statusDetails,omitempty"`
	Stage         string                 `json:"stage"`
	Start         int64                  `json:"start"`
	Stop          int64                  `json:"stop"`
	Duration      int64                  `json:"duration"`
	Steps         []Step                 `json:"steps,omitempty"`
	Attachments   []Attachment           `json:"attachments,omitempty"`
	Parameters    []Parameter            `json:"parameters,omitempty"`
	Extra         map[string]interface{} `json:"extra,omitempty"`
}

// Attachment represents a file attached to a test
type Attachment struct {
	Name   string `json:"name"`
	Type   string `json:"type"`
	Source string `json:"source"`
}

// Parameter represents a test parameter
type Parameter struct {
	Name  string `json:"name"`
	Value string `json:"value"`
}

// Label represents a test label (feature, story, severity, etc.)
type Label struct {
	Name  string `json:"name"`
	Value string `json:"value"`
}

// Link represents a link to external resources
type Link struct {
	Name string `json:"name"`
	URL  string `json:"url"`
	Type string `json:"type"`
}

// Test status constants
const (
	StatusPassed  = "passed"
	StatusFailed  = "failed"
	StatusSkipped = "skipped"
	StatusBroken  = "broken"
)

// Label types
const (
	LabelFeature   = "feature"
	LabelStory     = "story"
	LabelSeverity  = "severity"
	LabelSuite     = "suite"
	LabelPackage   = "package"
	LabelTestClass = "testClass"
	LabelTestMethod = "testMethod"
	LabelEpic      = "epic"
	LabelOwner     = "owner"
	LabelLead      = "lead"
	LabelHost      = "host"
	LabelThread    = "thread"
	LabelLanguage  = "language"
	LabelFramework = "framework"
)

// Severity levels
const (
	SeverityBlocker  = "blocker"
	SeverityCritical = "critical"
	SeverityNormal   = "normal"
	SeverityMinor    = "minor"
	SeverityTrivial  = "trivial"
)

// Stage constants
const (
	StageScheduled = "scheduled"
	StageRunning   = "running"
	StageFinished  = "finished"
	StagePending   = "pending"
	StageInterrupted = "interrupted"
)

// NewTestResult creates a new test result with default values
func NewTestResult(name, fullName string) *TestResult {
	return &TestResult{
		UUID:     generateUUID(),
		Name:     name,
		FullName: fullName,
		Status:   StatusPassed,
		Stage:    StageFinished,
		Start:    time.Now().UnixMilli(),
		Stop:     time.Now().UnixMilli(),
		Duration: 0,
		Steps:    []Step{},
		Attachments: []Attachment{},
		Parameters:  []Parameter{},
		Labels:      []Label{},
		Links:       []Link{},
		Extra:       make(map[string]interface{}),
	}
}

// AddStep adds a step to the test result
func (tr *TestResult) AddStep(step Step) {
	tr.Steps = append(tr.Steps, step)
}

// AddAttachment adds an attachment to the test result
func (tr *TestResult) AddAttachment(name, attachmentType, source string) {
	tr.Attachments = append(tr.Attachments, Attachment{
		Name:   name,
		Type:   attachmentType,
		Source: source,
	})
}

// AddParameter adds a parameter to the test result
func (tr *TestResult) AddParameter(name, value string) {
	tr.Parameters = append(tr.Parameters, Parameter{
		Name:  name,
		Value: value,
	})
}

// AddLabel adds a label to the test result
func (tr *TestResult) AddLabel(name, value string) {
	tr.Labels = append(tr.Labels, Label{
		Name:  name,
		Value: value,
	})
}

// AddLink adds a link to the test result
func (tr *TestResult) AddLink(name, url, linkType string) {
	tr.Links = append(tr.Links, Link{
		Name: name,
		URL:  url,
		Type: linkType,
	})
}

// SetStatus sets the test status with optional details
func (tr *TestResult) SetStatus(status, message, trace string) {
	tr.Status = status
	if message != "" || trace != "" {
		tr.StatusDetails = &StatusDetails{
			Known:   true,
			Muted:   false,
			Flaky:   false,
			Message: message,
			Trace:   trace,
		}
	}
}

// SetDuration sets the test duration
func (tr *TestResult) SetDuration(duration time.Duration) {
	tr.Duration = duration.Milliseconds()
	tr.Stop = tr.Start + tr.Duration
}

// ToJSON converts the test result to JSON
func (tr *TestResult) ToJSON() ([]byte, error) {
	return json.MarshalIndent(tr, "", "  ")
} 