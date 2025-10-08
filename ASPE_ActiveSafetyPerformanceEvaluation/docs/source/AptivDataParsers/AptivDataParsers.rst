.. _Aptiv Data Parsers:

Aptiv Data Parsers
================================

This section describes general usage of Aptiv Data parsers

This documentation (will) contain following parts:

* Supporting Formats
* Usage/Interface
* Command Line Options / Parser Configuration
* How to contribute
* MUDP parser documentation
* DVL parser documentation


.. automodule:: aptivdataparsers.AptivDataParser.aptiv_data_parser
   :members:

.. automodule:: aptivdataparsers.AptivDataParser.MudpParser.mudp_parser
   :members: MudpHandler, example

.. note::
   :ref:`Full mudp_parser.py documentation can be find here! <Mudp Parser>`

.. automodule:: aptivdataparsers.AptivDataParser.DvlParser.dvl_parser
   :members: DVLFileVersion, DVLParser, example

.. note::
   :ref:`Full dvl_parser.py documentation can be find here! <Dvl Parser>`


Versions
********

1.3._
_____
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| Version    | Release    | Content                                                                                                                                        |
+============+============+================================================================================================================================================+
| 1.3.1      | 2019.11.18 | - CEA-231 Provide path to mudp stream definition folder by MUDP handler initialization method                                                  |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| 1.3.0      | 2019.11.18 | - CEA-148 Handle n-dimensional arrays in mudp logs                                                                                             |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+

1.2._
_____

+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| Version    | Release    | Content                                                                                                                                        |
+============+============+================================================================================================================================================+
| 1.2.8      | 2019.10.16 | - non functional changes                                                                                                                       |
|            |            | - updated docstrings                                                                                                                           |
|            |            | - cleaned some code parts                                                                                                                      |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| 1.2.7      | 2019.10.14 | - (reverted) deleted f_force_parse flag from .mudp parser config                                                                               |
|            |            | - (reverted) deleted warnings about stream_chunk is equal 0                                                                                    |
|            |            | - handled case when stream_chunk is equal 0 - changed some sanity checks logic to work properly                                                |
|            |            | - updated some docstrings within .mudp parser                                                                                                  |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| 1.2.6      | 2019.10.02 | - in almost all streams' headers 'stream_chunks' value was 0, this causes sanity checks failure and data was not parsed                        |
|            |            | - added flag f_force_parse to mudp parser config                                                                                               |
|            |            | - when header is corrupted and f_force_parse flag is set to False - parsing will be not done and user will be informed with warning            |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
| 1.2.5      | 2019.8.22  | - Added unknown type handling to MUDP parser                                                                                                   |
+------------+------------+------------------------------------------------------------------------------------------------------------------------------------------------+
