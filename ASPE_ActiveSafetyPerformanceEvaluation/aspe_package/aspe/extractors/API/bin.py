import os
from dataclasses import fields
from pathlib import Path

from aspe.extractors.GDSR.gdsr_bin_extractor import GdsrBinExtractor, GdsrParsedData
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.ReferenceExtractor.dSpace.dspace_bin_extractor import DSpaceBinExtractor
from aspe.parsers.BinParser.bin_parser import BinParser
from aspe.utilities.SupportingFunctions import (
    create_log_path,
    create_pickle_path,
    get_log_list_from_path,
    load_from_pkl,
    save_to_pkl,
)


def parse_bin(bin_log_path: str) -> dict:
    """Parse bin file.

    :param bin_log_path: path to bin log file.
    :return: dictionary containing the parsed data.
    """
    return BinParser().parse(bin_log_path)


def extract_gdsr_from_bin(
    root_dir: str,
    log_name: str,
    extract_raw_signals: bool = False,
    save_to_file: bool = False,
    force_extract: bool = False,
) -> ExtractedData:
    """Parse and extract GDSR data from a bin file.

    If a previously saved pickle file with extracted data already exists, by default, it will be loaded instead of
    parsing and extracting from the bin file. This behavior can be changed by setting `force_extract` to True.

    :param root_dir: path to the directory that contains the bin files written during GDSR execution.
    :param log_name: name of the log file used for evaluation. It's assumed that the bin file follows the format:
        `{log_name}_{type}.bin`. e.g. for a bin file with the filename: `TestLog_TrackerOutput.bin`; it's expected that
        `log_name` is set to `TestLog`.
    :param extract_raw_signals: flag indicating if raw signals should be extracted. No translation is applied to raw
        signals. If True extracted raw signals are placed in separate DataFrame within the extracted data. Defaults to
        False.
    :param save_to_file: flag indicating if extracted output should be saved as a pickle file. If True, a pickle
        file will be created within same folder. Defaults to False.
    :param force_extract: forces extraction even if a previously saved pickle file with extracted data already exists.
        Defaults to False.
    :return: extracted data in ASPE data structure form.
    """
    print(f"Started processing GDSR data from log: {log_name}")
    tracker_out_path = os.path.join(root_dir, f"{log_name}_TrackerOutput.bin")
    radar_params_path = os.path.join(root_dir, f"{log_name}_RadarParams.bin")
    print(f"Using tracker output bin file: {tracker_out_path}")
    print(f"Using radar parameters bin file: {radar_params_path}")

    pickle_save_path = create_pickle_path(tracker_out_path)
    if Path(pickle_save_path).is_file() and not force_extract:
        print("Found pickle file. Loading instead of extracting...")
        extracted = load_from_pkl(pickle_save_path)
        print("Loaded successfully.")
    else:
        print("Parsing data ...")
        parsed_tracker_out = parse_bin(tracker_out_path)
        parsed_radar_params = parse_bin(radar_params_path)
        gdsr_parsed_data = GdsrParsedData(parsed_tracker_out, parsed_radar_params)
        print("Parsing done.")

        print("Extracing data ...")
        extractor = GdsrBinExtractor(f_extract_raw_signals=extract_raw_signals)
        extracted = extractor.extract_data(gdsr_parsed_data)
        print("Extraction done.")

        if save_to_file:
            save_to_pkl(extracted, pickle_save_path)
            print(f"Saved extracted data to {pickle_save_path}")

    return extracted


def extract_dspace_from_bin(
    root_dir: str,
    log_name: str,
    extract_raw_signals: bool = False,
    save_to_file: bool = False,
    force_extract: bool = False,
) -> ExtractedData:
    """Parse and extract dSpace data from a bin file.

    If a previously saved pickle file with extracted data already exists, by default, it will be loaded instead of
    parsing and extracting from the bin file. This behavior can be changed by setting `force_extract` to True.

    :param root_dir: path to the directory that contains the dSpace ground-truth bin file.
    :param log_name: name of the log file used for evaluation. It's assumed that the bin file follows the format:
        `{log_name}_{type}.bin`. E.g. For a bin file with the filename: `TestLog_OSIGTInput.bin`; it's expected that
        `log_name` is set to `TestLog`.
    :param extract_raw_signals: flag indicating if raw signals should be extracted. No translation is applied to raw
        signals. If yes, the extracted raw signals are placed in a separate DataFrame within the extracted data.
        Defaults to False.
    :param save_to_file: flag indicating if extracted output should be saved as a pickle file. If True, a pickle
        file will be created within same folder. Defaults to False.
    :param force_extract: forces extraction even if a previously saved pickle file with extracted data already exists.
        Defaults to False.
    :return: extracted data in ASPE data structure form.
    """
    print(f"Started processing dSpace ground-truth from log: {log_name}")
    dspace_path = os.path.join(root_dir, f"{log_name}_OSIGTInput.bin")
    print(f"Using dSpace ground-truth bin file: {dspace_path}")

    pickle_save_path = create_pickle_path(dspace_path)
    if Path(pickle_save_path).is_file() and not force_extract:
        print("Found pickle file. Loading instead of extracting.")
        extracted = load_from_pkl(pickle_save_path)
        print("Loaded successfully.")
    else:
        print("Parsing data ...")
        parsed = parse_bin(dspace_path)
        print("Parsing done.")

        print("Extracing data ...")
        extractor = DSpaceBinExtractor(f_extract_raw_signals=extract_raw_signals)
        extracted = extractor.extract_data(parsed)
        print("Extraction done.")

        if save_to_file:
            save_to_pkl(extracted, pickle_save_path)
            print(f"Saved extracted data to {pickle_save_path}")

    return extracted
