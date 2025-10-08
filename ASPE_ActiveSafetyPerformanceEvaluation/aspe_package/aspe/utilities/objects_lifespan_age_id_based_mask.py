import numpy as np
import pandas as pd


def create_obj_lifespan_mask_based_on_id_age(id_array: np.ndarray, age_array: np.ndarray, max_age_diff: float = 55.0) \
        -> pd.DataFrame:
    """
    Create lifespan mask, in pandas.DataFrame form which allows to filter only relevant records from parsed data arrays
    and set unique id for objects through it lifespan.

    DataFrame form:
    row     column      unique_id
    ------------------------------
    0       0           0
    1       0           0
    2       1           0
    0       2           1
    1       2           1
    .       .           .
    .       .           .

    Function assumes that object memory slot index could be changed through it lifetime, but it ID is constant. To
    distinguish two separate objects which have same ID, age information is used. It is assumed that age jumps which
    are larger than  max_age_diff indicates new object initialization.
    :param id_array: 2d numpy ndarray with IDs, columns represents memory slots, rows represents time instances
    :param age_array: 2d numpy ndarray with age, columns represents memory slots, rows represents time instances
    :param max_age_diff: value of age jump within objects with same ID which indicates two separate objects - risk of
    using this parameter is that single scan drop will split object to two separate IDs
    :return: pandas DataFrame with 3 columns: row, column and unique_id which represents lifespan mask
    """
    ids = np.unique(id_array)
    curr_unique_id = 0
    obj_lifespan_masks_list = []
    for id in ids:
        if id == 0:
            continue

        id_mask = id_array == id
        age = age_array[id_mask]

        diff_age = np.diff(np.hstack([age[0], age]))  # age[0] -> handle objects which live from first row
        unique_id_vec = np.full(age.shape, curr_unique_id)

        age_jumps_idxs = np.where(np.abs(diff_age) > max_age_diff)
        for age_jumps_idx in age_jumps_idxs[0]:
            unique_id_vec[age_jumps_idx:] += 1
        curr_unique_id = unique_id_vec[-1] + 1

        rows, columns = np.where(id_mask)
        obj_lifespan_masks_list.append(pd.DataFrame({
            'row': rows,
            'column': columns,
            'unique_id': unique_id_vec,
        }))
    lifespan_mask = pd.concat(obj_lifespan_masks_list).reset_index(drop=True)
    return lifespan_mask


if __name__ == '__main__':
    import time

    from mdf_parser_pkg.srr5_dev_tools.mgp_module import load as load_mgp

    from aspe.utilities.flatten_someip_object_list_data import flatten_someip_signals

    # example of usage
    mgp_parsed_data_path = r"C:\logs\DEX_497\DEX_475\BN_FASETH\parse_result\20200128T155737_20200128T155757_543078_LB36408_BN_FASETH_SRR_Master-mPAD.mgp"
    parsed_data = load_mgp(mgp_parsed_data_path)
    object_list_data = parsed_data['RecogSideRadarObjectlist']['Objectlist']
    transformed = flatten_someip_signals(object_list_data)

    ids = transformed['summary_id']
    age = transformed['summary_age']

    t1 = time.time()
    lifespan_mask = create_obj_lifespan_mask_based_on_id_age(ids, age)
    t2 = time.time()
    print(f'lifespan mask creation time: {t2-t1}')
