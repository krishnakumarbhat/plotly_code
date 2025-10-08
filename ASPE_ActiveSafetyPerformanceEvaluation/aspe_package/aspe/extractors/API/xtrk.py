from pathlib import Path

from aspe.extractors.F360.DataSets.F360XtrkExtractedData import F360XtrkExtractedData
from aspe.extractors.F360.Xtrk.F360XtrkExtractor import F360XtrkExtractor
from aspe.parsers.XtrkParser.XtrkParser import XtrkParser
from aspe.utilities.SupportingFunctions import create_pickle_path, is_log_newer_than_pickle, load_from_pkl, save_to_pkl


def extract_f360_from_xtrk(xtrk_path: str,
                           save_to_file: bool = False,
                           internal_objects: bool = True,
                           host: bool = True,
                           sensors: bool = True,
                           detections: bool = False,
                           clusters: bool = False,
                           stat_env: bool = False,
                           raw_signals: bool = False,
                           force_extract: bool = False,
                           ) -> F360XtrkExtractedData:
    '''
    Extract F360 tracker core data from .xtrk log file.

    :param xtrk_path: absolute path to .xtrk log file
    :type xtrk_path: str
    :param save_to_file: flag indicating if extracted output should be saved as .pickle file. If is True .pickle
    file named log_name_f360_xtrk_extracted.pickle will be created within same folder.
    :type save_to_file: bool
    :param internal_objects: flag indicating if reduced objects should be extracted
    :type internal_objects: bool
    :param sensors: flag indicating if sensors should be extracted
    :type sensors: bool
    :param host: flag indicating if host should be extracted
    :type host: bool
    :param detections: flag indicating if detections should be extracted
    :type detections: bool
    :param raw_signals: flag indicating if raw signals should be extracted. Raw signals have same signatures as in
    stream definition - there is no translation applied. If yes extracted raw signals are placed in separate DataFrame
    within data set.
    :type raw_signals: bool
    :param force_extract: if false function tries to find already extracted .pickle file and load it. If true extraction
    process is done, even if .pickle file already exists.
    :return: extracted data in ASPE data structure form
    :type: F360XtrkExtractedData
    '''
    print(f'Started processing XTRK log file: {Path(xtrk_path).name}')
    pickle_save_path = create_pickle_path(xtrk_path)
    if Path(pickle_save_path).exists() and not force_extract and not is_log_newer_than_pickle(xtrk_path,
                                                                                              pickle_save_path):
        print('Found pickle file. Loading instead of extracting.')
        extracted = load_from_pkl(pickle_save_path)
    else:
        print(f'Parsing data ...')
        parser = XtrkParser()
        parsed = parser.parse(log_file_path=xtrk_path)

        print(f'Extracting data ...')
        extractor = F360XtrkExtractor(
            f_extract_internal_objects=internal_objects,
            f_extract_sensors=sensors,
            f_extract_host=host,
            f_extract_detections=detections,
            f_extract_clusters=clusters,
            f_extract_stat_env=stat_env,
            f_builders_extract_raw_signals=raw_signals,
        )

        extracted = extractor.extract_data(parsed)
        print('Extraction done')
        if save_to_file:
            save_to_pkl(extracted, pickle_save_path)
            print(f'Saved output to {pickle_save_path}')
    return extracted


if __name__ == '__main__':
    xtrk_path = r"PATH_TO_XTRK_FILE"

    extracted = extract_f360_from_xtrk(xtrk_path,
                                       force_extract=True,
                                       detections=True,
                                       sensors=True,
                                       clusters=True,
                                       save_to_file=True)
