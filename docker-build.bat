@echo off
REM ============================================================================
REM Docker and Singularity Build Script for Windows
REM ResimHTMLReport - KPI Server & Interactive Plot Applications
REM ============================================================================
REM Usage: docker-build.bat [command]

setlocal enabledelayedexpansion

set COMMAND=%1

if "%COMMAND%"=="" (
    echo ============================================================================
    echo Docker and Singularity Build Script
    echo ============================================================================
    echo.
    echo Usage: docker-build.bat [command]
    echo.
    echo Docker Commands:
    echo   build              - Build all Docker images
    echo   build-kpi          - Build KPI Server Docker image only
    echo   build-interactive  - Build Interactive Plot Docker image only
    echo   run-kpi            - Run KPI Server in ZMQ mode (port 5555)
    echo   run-kpi-json       - Run KPI Server in JSON batch mode
    echo   run-interactive    - Run Interactive Plot container
    echo   run-generator      - Run KPI Dummy HDF Generator
    echo   up                 - Start all services with docker-compose
    echo   down               - Stop all docker-compose services
    echo   logs               - Show logs from running containers
    echo   clean              - Remove all containers and images
    echo.
    echo Singularity Commands (requires WSL2 or Linux):
    echo   singularity-build  - Build Singularity images from definition files
    echo   docker-to-sif      - Convert Docker images to Singularity .sif
    echo.
    exit /b 1
)

REM ============================================================================
REM Docker Commands
REM ============================================================================

if "%COMMAND%"=="build" (
    echo ============================================================================
    echo Building all Docker images...
    echo ============================================================================
    docker-compose build
    echo.
    echo Done! Images built:
    docker images ^| findstr /i "kpi-server interactiveplot"
    goto :eof
)

if "%COMMAND%"=="build-kpi" (
    echo ============================================================================
    echo Building KPI Server Docker image...
    echo ============================================================================
    docker build -f Dockerfile.kpi -t kpi-server:latest .
    echo.
    echo Done! KPI Server image: kpi-server:latest
    goto :eof
)

if "%COMMAND%"=="build-interactive" (
    echo ============================================================================
    echo Building Interactive Plot Docker image...
    echo ============================================================================
    docker build -f Dockerfile.interactiveplot -t interactiveplot:latest .
    echo.
    echo Done! Interactive Plot image: interactiveplot:latest
    goto :eof
)

if "%COMMAND%"=="run-kpi" (
    echo ============================================================================
    echo Running KPI Server in ZMQ mode on port 5555...
    echo ============================================================================
    echo Press Ctrl+C to stop the server
    echo.
    docker run -it --rm ^
        --name kpi-server ^
        -p 5555:5555 ^
        -v "%cd%\hdf_DB:/app/hdf_DB" ^
        -v "%cd%\html:/app/html" ^
        -v "%cd%\logs:/app/logs" ^
        kpi-server:latest ^
        python -m KPI.kpi_server zmq 5555
    goto :eof
)

if "%COMMAND%"=="run-kpi-json" (
    echo ============================================================================
    echo Running KPI Server in JSON batch mode...
    echo ============================================================================
    docker run -it --rm ^
        --name kpi-json-processor ^
        -v "%cd%\hdf_DB:/app/hdf_DB" ^
        -v "%cd%\html:/app/html" ^
        -v "%cd%\KPIPlot.json:/app/KPIPlot.json:ro" ^
        kpi-server:latest ^
        python -m KPI.kpi_server KPIPlot.json /app/html
    goto :eof
)

if "%COMMAND%"=="run-interactive" (
    echo ============================================================================
    echo Running Interactive Plot container...
    echo ============================================================================
    echo Make sure KPI server is running first (run-kpi command)
    echo.
    docker run -it --rm ^
        --name interactiveplot ^
        --network kpi-zmq-network ^
        -e KPI_SERVER_HOST=kpi-server ^
        -e KPI_SERVER_PORT=5555 ^
        -v "%cd%\hdf_DB:/app/data" ^
        -v "%cd%\html:/app/html" ^
        -v "%cd%\plots:/app/plots" ^
        interactiveplot:latest %2 %3 %4 %5
    goto :eof
)

if "%COMMAND%"=="run-generator" (
    echo ============================================================================
    echo Running KPI Dummy HDF Generator...
    echo ============================================================================
    docker run -it --rm ^
        --name kpi-generator ^
        -v "%cd%\hdf_DB:/app/hdf_DB" ^
        kpi-server:latest ^
        python -m KPI.a_persistence_layer.kpi_dummy_hdf_generator
    echo.
    echo Done! Test HDF files generated in hdf_DB folder.
    goto :eof
)

if "%COMMAND%"=="up" (
    echo ============================================================================
    echo Starting all services with docker-compose...
    echo ============================================================================
    docker-compose up -d
    echo.
    echo Services started:
    docker-compose ps
    echo.
    echo KPI Server ZMQ endpoint: tcp://localhost:5555
    goto :eof
)

if "%COMMAND%"=="down" (
    echo ============================================================================
    echo Stopping all docker-compose services...
    echo ============================================================================
    docker-compose down
    echo Done!
    goto :eof
)

if "%COMMAND%"=="logs" (
    echo ============================================================================
    echo Showing container logs (Ctrl+C to exit)...
    echo ============================================================================
    docker-compose logs -f
    goto :eof
)

if "%COMMAND%"=="clean" (
    echo ============================================================================
    echo Cleaning Docker resources...
    echo ============================================================================
    echo Stopping and removing all containers...
    docker-compose down --rmi all --volumes --remove-orphans
    echo Removing dangling images...
    docker image prune -f
    echo Done!
    goto :eof
)

REM ============================================================================
REM Singularity Commands (via WSL2)
REM ============================================================================

if "%COMMAND%"=="singularity-build" (
    echo ============================================================================
    echo Building Singularity images via WSL2...
    echo ============================================================================
    echo Note: This requires Singularity installed in WSL2
    echo.
    wsl bash -c "cd /mnt/c/git/10030156_03_Analysis_Framework_Suite/ResimulationTool/SOURCE/ApplicationProjects/ResimHTMLReport/Interactiveplot/plotly && sudo singularity build kpi_server.sif singularity_kpi.def"
    wsl bash -c "cd /mnt/c/git/10030156_03_Analysis_Framework_Suite/ResimulationTool/SOURCE/ApplicationProjects/ResimHTMLReport/Interactiveplot/plotly && sudo singularity build interactiveplot.sif singularity_interactiveplot.def"
    echo.
    echo Done! Singularity images created.
    goto :eof
)

if "%COMMAND%"=="docker-to-sif" (
    echo ============================================================================
    echo Converting Docker images to Singularity via WSL2...
    echo ============================================================================
    echo Note: This requires Singularity installed in WSL2
    echo.
    echo Step 1: Building Docker images first...
    docker-compose build
    echo.
    echo Step 2: Converting to Singularity...
    wsl bash -c "singularity build kpi_server.sif docker-daemon://kpi-server:latest"
    wsl bash -c "singularity build interactiveplot.sif docker-daemon://interactiveplot:latest"
    echo.
    echo Done! Singularity .sif files created.
    goto :eof
)

echo Unknown command: %COMMAND%
echo Run 'docker-build.bat' without arguments to see available commands.
exit /b 1
