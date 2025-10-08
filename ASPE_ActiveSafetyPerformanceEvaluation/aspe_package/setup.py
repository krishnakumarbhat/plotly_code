from setuptools import setup, find_packages


def req(path):
    with open(path) as f:
        return [line for line in f if '--' not in line]


setup(name='aspe',
      version='1.1.7',
      description='Aptiv tools for performance evaluation of logged data',
      author='Aptiv TCK Algorithm Development - Perception F360 ST4',
      author_email='plkraasadf360st4@Aptiv.com',
      license='internal use only',
      packages=find_packages(),
      include_package_data=True,
      zip_safe=False,
      install_requires=req('requirements.txt'),
      )
