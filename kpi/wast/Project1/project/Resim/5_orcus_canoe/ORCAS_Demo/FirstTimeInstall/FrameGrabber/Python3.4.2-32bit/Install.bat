@echo off
echo ###### DevSuite Python 3.4.2 32-bit installer
echo ### Please do not alter the installation folder or
echo ### you may lose 32-bit or 64-bit Python support.
echo ###
echo ### Lauching the installer - please follow the prompts
msiexec /i "bin\python-3.4.2.msi" TARGETDIR="C:\Python34(x86)"
echo ### Installation Complete
