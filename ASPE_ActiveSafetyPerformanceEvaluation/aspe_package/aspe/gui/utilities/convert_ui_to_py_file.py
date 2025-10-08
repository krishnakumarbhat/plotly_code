import subprocess

if __name__ == "__main__":
    ui_path = r"C:\wkspaces\aspe_gui\views\uis\drawer_labels_settings_widget.ui"
    output_py_path = r"C:\wkspaces\aspe_gui\views\uis\drawer_labels_settings_widget.py"

    subprocess.call(f"pyuic5 {ui_path} -o {output_py_path}")  # noqa: S603
