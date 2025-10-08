from pathlib import Path

from srr5_dev_tools import pkl_module

from aspe.extractors.ReferenceExtractor.SDB.SDB_Extractor import SDB_Extractor


def download_pandora_log_from_sdb(
        log_id: str,
        save_dir: Path = None,
        load_cached: bool = True,
):
    """
    Download log data from Scenario Data Base using PSACInterface class
    :param log_id: id of log to download
    :param save_dir: directory where downloaded log should be saved as .pickle file,
        if not provided - data will be not saved
    :param load_cached: if True function first tries to find already downloaded data saved into .pickle file and load it
    :return: list of SDB samples
    """
    if load_cached and save_dir is not None:
        cached_path = save_dir / f'log_{log_id}_sdb_raw.pickle'
        if cached_path.exists():
            print(f'Found already downloaded data, loading {cached_path}')
            return pkl_module.load(cached_path)
    raise Exception(f'No SDB data found with id: {log_id}')


def extract_pandora_log_from_sdb(
        estimated_data_path: str,
        set_id: str,
        stationary_threshold: float,
        load_cache: bool = True,
        extract_sdb_cache_dir: Path = None,
        label_sdb_cache_dir: Path = None,
):
    """
    Download given log data from SDB and transform it to ASPE structure.
    :param log_id: id of log to download
    :param extract_sdb_cache_dir: directory where downloaded log and extracted data should be saved as .pickle file,
        if not provided - data will be not saved
    :param load_cache: if True function first tries to find already extracted data saved into .pickle file and load it
    :param stationary_threshold: threshold value below which consider object stationary
    :return:
    """
    estimated_data_path = Path(estimated_data_path)
    if extract_sdb_cache_dir is None:
        extract_sdb_cache_dir = estimated_data_path.parents[1] / 'ASP_output' / 'SDB_cache'
    extract_sdb_cache_dir.mkdir(parents=True, exist_ok=True)
    if label_sdb_cache_dir is None:
        label_sdb_cache_dir = estimated_data_path.parents[1] / 'ASP_output' / 'SDB_cache'
    label_sdb_cache_dir.mkdir(parents=True, exist_ok=True)
    if load_cache and extract_sdb_cache_dir is not None:
        cache_path = extract_sdb_cache_dir / f'log_{set_id}_sdb_extracted.pickle'
        if cache_path.exists():
            print(f"Loading already extracted data from : {cache_path}")
            extracted = pkl_module.load(cache_path)
            return extracted
    print(f"Extracting data ... ")
    sdb_raw_data = download_pandora_log_from_sdb(
        set_id,
        label_sdb_cache_dir,
        load_cache,
    )
    extracted = SDB_Extractor(stationary_threshold=stationary_threshold).extract_data(sdb_raw_data)
    if extract_sdb_cache_dir is not None:
        cache_path = extract_sdb_cache_dir / f'log_{set_id}_sdb_extracted.pickle'
        print(f'Saving output to: {cache_path}')
        pkl_module.save(cache_path, extracted)
    return extracted
