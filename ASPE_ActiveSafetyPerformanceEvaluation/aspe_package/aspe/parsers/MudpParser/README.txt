To use MUDP parser you need to add MUDP_STREAM_DEFINITIONS_PATH to your system or user environment variables. It should point to the folder with MUPD definitions
i.e.
MUDP_STREAM_DEFINITIONS_PATH = C:\Repos\F360\F360Core\F360TrackerPC_SRR_ESR_vs2015\zResimSupport\stream_definitions

# RUN
Input arguments:
    1) path to folder with mudp to process (no quotation marks)
    2) list of stream to decode (python form)
python MudpParser.py C:\path_to_folder_with_mudp [3,6]