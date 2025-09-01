@echo off
REM Batch file to build ResimHTMLReport.py with Nuitka

REM Make sure Nuitka is installed and in your PATH
REM Also ensure UPX is installed and in your PATH if you want UPX compression

nuitka ^
  --standalone ^
  --enable-plugin=upx ^
  --enable-plugin=pyside6 ^
  --include-data-dir=InteractivePlot/a_config_layer=InteractivePlot/a_config_layer ^
  --include-data-dir=InteractivePlot/b_persistence_layer=InteractivePlot/b_persistence_layer ^
  --include-data-dir=InteractivePlot/c_data_storage=InteractivePlot/c_data_storage ^
  --include-data-dir=InteractivePlot/d_business_layer=InteractivePlot/d_business_layer ^
  --include-data-dir=InteractivePlot/e_presentation_layer=InteractivePlot/e_presentation_layer ^
  --include-data-dir=KPI/a_business_layer=KPI/a_business_layer ^
  --include-data-dir=KPI/b_presentation_layer=KPI/b_presentation_layer ^
  --noinclude-setuptools-mode=nofollow ^
  --noinclude-pytest-mode=nofollow ^
  --plugin-enable=anti-bloat ^
  --nofollow-import-to=setuptools_scm ^
  --include-module=lxml ^
  --include-module=h5py ^
  --include-module=plotly ^
  --include-module=InteractivePlot.d_business_layer.data_cal ^
  --include-module=InteractivePlot.d_business_layer ^
  --include-module=pandas ^
  --include-module=numpy ^
  --include-module=pytz ^
  --include-module=six ^
  --include-module=datashader ^
  --include-module=win32api ^
  --include-module=win32con ^
  --include-module=pywintypes ^
  --output-dir=dist ^
  ResimHTMLReport.py

IF %ERRORLEVEL% NEQ 0 (
    echo Build failed with error %ERRORLEVEL%.
    pause
    exit /b %ERRORLEVEL%
) ELSE (
    echo Build completed successfully.
    pause
)
