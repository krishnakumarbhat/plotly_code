@echo off
setlocal enabledelayedexpansion

REM Allure Report POC - Test Runner Script for Windows
REM This script runs Go tests and generates Allure reports

set RESULTS_DIR=results
set OUTPUT_DIR=reports
set TEST_PATH=./tests/...
set DEBUG=false
set SERVE=false

REM Function to show usage
:show_usage
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -h, --help          Show this help message
echo   -r, --results DIR   Results directory ^(default: results^)
echo   -o, --output DIR    Output directory ^(default: reports^)
echo   -t, --test PATH     Test path to run ^(default: ./tests/...^)
echo   -d, --debug         Enable debug output
echo   -s, --serve         Serve the report after generation
echo   -c, --clean         Clean previous results before running
echo.
echo Examples:
echo   %0                    # Run tests with default settings
echo   %0 -d -s              # Run with debug and serve report
echo   %0 -t ./tests/calculator_test.go  # Run specific test
echo   %0 -c                 # Clean and run tests
goto :eof

REM Function to check if Allure is installed
:check_allure
allure --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Allure CLI is not installed or not in PATH
    echo.
    echo Install Allure CLI:
    echo   Windows: scoop install allure
    echo   or download from: https://github.com/allure-framework/allure2/releases
    exit /b 1
)
echo [SUCCESS] Allure CLI found
goto :eof

REM Function to clean previous results
:clean_results
if exist "%RESULTS_DIR%" (
    echo [INFO] Cleaning previous results...
    rmdir /s /q "%RESULTS_DIR%"
)
if exist "%OUTPUT_DIR%" (
    echo [INFO] Cleaning previous reports...
    rmdir /s /q "%OUTPUT_DIR%"
)
goto :eof

REM Function to run tests
:run_tests
echo [INFO] Running Go tests...

REM Run tests with JSON output and pipe to our generator
go test %TEST_PATH% -v -json | go run main.go -results="%RESULTS_DIR%" -output="%OUTPUT_DIR%" -test="%TEST_PATH%" -debug="%DEBUG%"
goto :eof

REM Function to serve report
:serve_report
if "%SERVE%"=="true" (
    echo [INFO] Starting Allure server...
    allure serve "%RESULTS_DIR%"
) else (
    echo.
    echo [SUCCESS] Test execution completed!
    echo.
    echo To view the report:
    echo   allure serve %RESULTS_DIR%
    echo   or
    echo   allure open %OUTPUT_DIR%
    echo.
    echo Report files:
    echo   Results: %RESULTS_DIR%
    echo   Report:  %OUTPUT_DIR%
)
goto :eof

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :main
if "%~1"=="-h" goto :show_usage
if "%~1"=="--help" goto :show_usage
if "%~1"=="-r" (
    set RESULTS_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--results" (
    set RESULTS_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-o" (
    set OUTPUT_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--output" (
    set OUTPUT_DIR=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-t" (
    set TEST_PATH=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--test" (
    set TEST_PATH=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-d" (
    set DEBUG=true
    shift
    goto :parse_args
)
if "%~1"=="--debug" (
    set DEBUG=true
    shift
    goto :parse_args
)
if "%~1"=="-s" (
    set SERVE=true
    shift
    goto :parse_args
)
if "%~1"=="--serve" (
    set SERVE=true
    shift
    goto :parse_args
)
if "%~1"=="-c" (
    call :clean_results
    shift
    goto :parse_args
)
if "%~1"=="--clean" (
    call :clean_results
    shift
    goto :parse_args
)
echo [ERROR] Unknown option: %~1
call :show_usage
exit /b 1

REM Main execution
:main
echo [INFO] Starting Allure Report POC...

REM Check prerequisites
call :check_allure
if errorlevel 1 exit /b 1

REM Check if Go is available
go version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Go is not installed or not in PATH
    exit /b 1
)

REM Check if we're in a Go module
if not exist "go.mod" (
    echo [ERROR] Not in a Go module. Run 'go mod init' first.
    exit /b 1
)

REM Download dependencies
echo [INFO] Downloading dependencies...
go mod tidy

REM Run tests
call :run_tests

REM Serve report if requested
call :serve_report

echo.
echo [INFO] Allure Report POC completed successfully! 