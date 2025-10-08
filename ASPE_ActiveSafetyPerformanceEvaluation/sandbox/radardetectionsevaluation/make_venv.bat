python -m venv .venv_pe

call .venv_pe\scripts\activate

pip install ../../aptivdataparsers
pip install ../../aptivdataextractors
pip install ../../aptivperformanceevaluation
pip install -r requirements_dev.txt
call deactivate
pause