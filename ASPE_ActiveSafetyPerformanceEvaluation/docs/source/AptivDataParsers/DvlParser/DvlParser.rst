.. _Dvl Parser:

Aptiv Data Parsers
==================

This section describes Aptiv Dvl Data parsers

This documentation (will) contain following parts:

* File format description
* Module architecture
* Usage
* How to contribute

File format description
-----------------------

DVL is a binary format that contains CAN network data stream.

To decode DVL you need to provide signals definition, which are stored in .dbc files.

.. note::
   `More info about DVL can be found on Aptiv Confluence Page <http://confluenceprod1.delphiauto.net:8090/display/TASADT/DVL>`_


.. automodule:: aptivdataparsers.AptivDataParser.DvlParser.dvl_parser
   :members: DVLFileVersion, DVLParser, DvlPackage, DVLExtractionMethods

