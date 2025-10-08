@echo off
:: Script automatically sets up python virtual environment with all necessary packages.
:: In some cases corporate firewall is blocking pip from downloading packages.
:: The easiest workaround is to connect to internet through different network.

:: Set python 3.7 directory
set python_3p7_dir=

if defined python_3p7_dir (
   set "PATH=%python_3p7_dir%;%PATH%"
   python -m venv venv
   call venv\Scripts\activate
   python -m pip install --upgrade pip
   python -m pip install -r requirements.txt
   call deactivate
) else (
   echo "Python 3.7.x directory path not set in bat file"
)
pause
