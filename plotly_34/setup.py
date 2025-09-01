from setuptools import setup, find_packages

setup(
    name="interactive_plot",
    version="0.1",
    packages=find_packages(),
    install_requires=[
        'h5py>=3.9.0',
        'numpy>=1.24.3',
    ],
)
