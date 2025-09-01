@echo off
echo Building Docker image...
docker build -t resim-html-report .

echo.
echo Converting Docker image to Singularity...
singularity build resim-html-report.sif docker-daemon://resim-html-report:latest

echo.
if %ERRORLEVEL% EQU 0 (
    echo Build completed successfully!
    echo Singularity image created as: resim-html-report.sif
) else (
    echo Build failed! Please check the error messages above.
)

pause