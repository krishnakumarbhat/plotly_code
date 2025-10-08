.. ASPE documentation master file, created by
   sphinx-quickstart on Tue Aug 13 17:26:51 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Documentation for the Active Safety Performance Evaluation (ASPE)
******************************************************************
.. toctree::
   :maxdepth: 2
   :caption: Contents:

ASPE Introduction
===================
..
   .. automodule:: app
   :members:

This is an Active Safety Performance Evaluation (ASPE) project documentation:

ASPE is a set of libraries used by Active Safety teams in Aptiv. It started as a library for a performance evaluation for tracker development. Now we are working to split it to modules which can be use by different teams in multi domain problems.

Overview on How to Project is divided
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Currently ASPE is divided into following modules:

1. Data Parsers
    - Used for data read and conversion from different file format to a python readable format
2. Data Extractors
    - Used for data conversion to shared format user friendly format
3. Data Synchronization
    - Used to synchronize data from different sources
4. Tracker Performance Evaluation
    - Module to get performance metrics from tracker algorithms

Overview on How to Run this API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Either install a Python IDE or create a Python virtual environment to install the packages required
2. Install packages required
3. Run example notebook or follow tutorial for usage introduction

Setup procedure
~~~~~~~~~~~~~~~~
1. Configure project environment (Either A. Install Pycharm OR B. Create a Virtual Environment)
    A. Install Pycharm (www.jetbrains.com/pycharm/download/)::

            pip install -r requirements.txt

    B. Create a Python Virtual Environment
        - Install virtualenv::

            sudo pip install virtualenv

        - Create virtialenv::

            virtualenv -p python3 <name of virtualenv>

        - Install requirements::

            pip install -r requirements.txt

PASTA Data Parsers
=====================
Data Parsers is a module used to convert aptiv log files into a python readable format

Currently we support:
   1. MUDP
   2. DVL

.. toctree::

   AptivDataParsers/AptivDataParsers


PASTA Data Extractors
=====================
..
   .. automodule:: main
   :members:

PASTA Synchronization Tool
==========================
..
   .. automodule:: main
   :members:

PASTA Performance Evaluation
============================
..
   .. automodule:: main
   :members:

PASTA How to contribute
=======================
..
   .. automodule:: main
   :members:



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

