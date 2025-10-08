from setuptools import setup, find_packages


def req(path):
    with open(path) as f:
        return f.read().splitlines()


setup(name='radardetseval',
      version='0.0.8',
      description='Aptiv performance evaluation of radar detections',
      author='Aptiv TCK Algorithm Development - Perception F360 ST4',
      author_email='plkraasadf360st4@Aptiv.com',
      license='internal use only',
      packages=find_packages(),
      zip_safe=False,
      install_requires=req('requirements.txt'),
      include_package_data=True)
