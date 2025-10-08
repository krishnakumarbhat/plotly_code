Table of contents
=================

- [Links](#links)
- [Setup](#setup)
  - [Setup using Visual Studio Code](#setup_using_visual_studio_code)
- [Examples](#examples)
  - [Preparing Examples](#preparing_examples)
    - [Get example data](#get_gxample_data)
    - [Get stream definitions](#get_stream_definitions)
  - [Using Examples](#using_examples)
    - [Evaluate some data](#evaluate_some_data)
    - [Generate a Report](#generate_a_report)

Links
=====
- [Confluence](https://confluence.asux.aptiv.com/display/ASPE/Active+Safety+Performance+Evaluation)
- [Project](http://pep.usinkok.northamerica.delphiauto.net/projectdb/public/?page=project-tasks&pid=3809.1911)
- [Jira](http://jiraprod1.delphiauto.net:8080/projects/DOH)

Setup
=====
See [Confluence](https://confluence.asux.aptiv.com/display/ASPE/ASPE+Installation+Guide)

Examples
========

Preparing Examples
------------------
### Get example data
Follow the guide on [Confluence](https://confluence.asux.aptiv.com/display/ASPE/Run+examples+from+aspe.examples)

Using Examples
--------------
### Evaluate some data
- open the file ASPE0000_00_Common/aspe_package/aspe/examples/evaluation/04_F360_multi_log_evaluation.py
- Run it
- It will produce the following output files:
  - **ASPE_Examples**/Analysis_Data/multi_log_evaluation_output_2_12.pickle
  - **ASPE_Examples**/Analysis_Data/multi_log_evaluation_output_2_15.pickle
  
### Generate a Report
Make sure to execute the chapter [Evaluate some data](#evaluate_some_data) successfully first. 
- Create a folder **ASPE_Examples**/Analysis_Data/report_temp_figs_path
- open the file ASPE0000_00_Common/aspe_package/aspe/examples/evaluation/07_generate_report.py
- Run it
- It will produce:
  - lots of temporary files in **ASPE_Examples**/Analysis_Data/report_temp_figs_path
  - The report: **ASPE_Examples**/Analysis_Data/ASPE_report_2_12_2_15_compare.pdf
