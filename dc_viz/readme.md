# Log Viewer - Build Instructions

## Prerequisites
Install PyInstaller:
```bash
pip install pyinstaller
```

## Building

### Windows
```bash
pyinstaller log_viewer_windows.spec
```
The executable will be in `dist/log_viewer.exe`

### Linux
```bash
pyinstaller log_viewer_linux.spec
```
The executable will be in `dist/log_viewer`

## Usage
```bash
# Windows
log_viewer.exe <html_root> <video_root> [output_html]

# Linux
./log_viewer <html_root> <video_root> [output_html]

# Example
log_viewer db/html db/video out/viewer.html
```

## Notes
- Both spec files create single-file executables (--onefile mode)
- The `html_build` module is included as a hidden import
- Windows version keeps console window open for output
- Linux version uses strip=True for smaller binary size
- UPX compression is enabled for both platforms
