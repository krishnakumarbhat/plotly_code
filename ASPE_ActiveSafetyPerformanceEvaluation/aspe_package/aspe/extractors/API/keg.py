from pathlib import Path

from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.extractors.ENV.Keg.ENVKegExtractor import ENVKegExtractor
from aspe.parsers.KegParser.keg_parser import KegParser
from aspe.utilities.SupportingFunctions import create_pickle_path, load_from_pkl, save_to_pkl


def parse_keg(keg_log_path: str) -> dict:
    """Parse keg file.
        :param keg_log_path: path to leg log file.
        :return: dictionary containing the parsed data.
    """
    return KegParser().parse(keg_log_path)


def extract_env_from_keg(
        keg_path: str,
        raw_signals: bool = False,
        save_to_file: bool = False,
        force_extract: bool = False,
) -> ENVExtractedData:
    """Parse and extract Environment data from a keg file.

     If a previously saved pickle file with extracted data already exists, by default, it will be loaded instead of
     parsing and extracting from the keg file. This behavior can be changed by setting `force_extract` to True.

     :param keg_path: path to the keg file.
     :param raw_signals: flag indicating if raw signals should be extracted. No translation is applied to raw
         signals. If set to true, extracted raw signals are placed in separate DataFrame within the extracted data.
        Defaults to False.
     :param save_to_file: flag indicating if extracted output should be saved as a pickle file. If True, a pickle
         file will be created within same folder. Defaults to False.
     :param force_extract: forces extraction even if a previously saved pickle file with extracted data already exists.
         Defaults to False.
     :return: extracted data in ASPE data structure form.
     """
    pickle_save_path = create_pickle_path(keg_path)
    if Path(pickle_save_path).is_file() and not force_extract:
        print("Found pickle file. Loading instead of extracting...")
        extracted = load_from_pkl(pickle_save_path)
        print("Loaded successfully.")
    else:
        print("Parsing data ...")
        parser = KegParser()
        parsed = parser.parse(keg_path)

        print(f'Extracting data ...')
        extractor = ENVKegExtractor(f_extract_raw_signals=raw_signals)
        extracted = extractor.extract_data(parsed)
        print("Extraction done.")

        if save_to_file:
            save_to_pkl(extracted, pickle_save_path)
            print(f"Saved extracted data to {pickle_save_path}")

    return extracted