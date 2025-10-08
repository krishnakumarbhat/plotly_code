# coding=utf-8
"""
Supporting modules for objects extraction
Help with conversion from numpy arrays to single objects format
"""
import numpy as np
import pandas as pd


class ObjectLifespanStatusBasedCreator:
    """
    Module for creating lifespan mask for object extraction based on status signal.
    """
    @staticmethod
    def create_lifespan(object_status: np.ndarray, obj_status_new_val: int, obj_status_invalid_val: int):
        """
        Main function which returns proper formed DataFrame which represents mask to be used in object extraction.
        Output DataFrame contains columns: 'row', 'column', 'unique_id'. Single row represents single object within
        one time instance.
        :param object_status: array of objects status, columns: objects slots, rows: scan indexes
        :param obj_status_new_val: value which represents new status within objects_status array
        :param obj_status_invalid_val: value which represents invalid status within objects_status array
        :return:
        """
        first_instance_mask = ObjectLifespanStatusBasedCreator._create_first_obj_instance_mask(object_status,
                                                                                               obj_status_new_val,
                                                                                               obj_status_invalid_val)
        last_instance_mask = ObjectLifespanStatusBasedCreator._create_last_obj_instance_mask(object_status,
                                                                                             obj_status_new_val,
                                                                                             obj_status_invalid_val)
        ObjectLifespanStatusBasedCreator._check_if_columns_are_consistent(first_instance_mask, last_instance_mask)
        first_last_mask = ObjectLifespanStatusBasedCreator._concat_first_last_instance_masks(first_instance_mask,
                                                                                             last_instance_mask)
        return ObjectLifespanStatusBasedCreator._transform_first_last_mask_to_lifespan_mask(first_last_mask)

    @staticmethod
    def _create_first_obj_instance_mask(object_status: np.ndarray, obj_status_new_val: int,
                                        obj_status_invalid_val: int):
        """
        Create DataFrame with columns: 'first_row', 'column'. Each row contains information about place where object
        was initialized. Logic is based on objects status array. To mark cell as 'new object was born' logic sum of 2
        conditions must be met:
        - object_status is STATUS_NEW
        OR
        - current object_status is not INVALID and previous object_status is INVALID
        :param object_status: array of objects status, columns: objects slots, rows: scan indexes
        :param obj_status_new_val: value which represents new status within object_status array
        :param obj_status_invalid_val: value which represents invalid status within object_status array
        :return: DataFrame with columns: 'first_row', 'column'
        """
        zero_row = np.full((1, object_status.shape[1]), obj_status_invalid_val)
        prev_status = np.vstack([zero_row, object_status[:-1]])
        status_new_mask = object_status == obj_status_new_val
        prev_inv_curr_non_inv = np.logical_and(prev_status == obj_status_invalid_val,
                                               object_status != obj_status_invalid_val)
        first_instance_mask = np.logical_or(status_new_mask, prev_inv_curr_non_inv)
        first_instance_mask_df = pd.DataFrame(np.argwhere(first_instance_mask), columns=['first_row', 'column'])
        first_instance_mask_df.sort_values(by=['column', 'first_row'], inplace=True)
        first_instance_mask_df.reset_index(inplace=True, drop=True)
        return first_instance_mask_df

    @staticmethod
    def _create_last_obj_instance_mask(object_status: np.ndarray, obj_status_new_val: int,
                                       obj_status_invalid_val: int):
        """
        Create DataFrame with columns: 'last_row', 'column'. Each row contains information about place where object
        was killed. Logic base on objects status array. Cell where objects are killed should fulfill conditions:
        - current time instance status is not invalid
        - next time instance status is invalid or new
        :param object_status: array of objects status, columns: objects slots, rows: scan indexes
        :param obj_status_new_val: value which represents new status within object_status array
        :param obj_status_invalid_val: value which represents invalid status within object_status array
        :return: DataFrame with columns: 'last_row', 'column'
        """
        end_row = np.full((1, object_status.shape[1]), obj_status_invalid_val)
        next_status = np.vstack((object_status[1:, :], end_row))
        curr_status_non_invalid = object_status != obj_status_invalid_val
        next_status_invalid_or_new = np.logical_or(next_status == obj_status_invalid_val,
                                                   next_status == obj_status_new_val)
        last_instance_mask = np.logical_and(curr_status_non_invalid, next_status_invalid_or_new)
        last_instance_mask_df = pd.DataFrame(np.argwhere(last_instance_mask), columns=['last_row', 'column'])
        last_instance_mask_df.sort_values(by=['column', 'last_row'], inplace=True)
        last_instance_mask_df.reset_index(inplace=True, drop=True)
        return last_instance_mask_df

    @staticmethod
    def _check_if_columns_are_consistent(first_instance_mask: pd.DataFrame, last_instance_mask: pd.DataFrame):
        """
        Check before merging DataFrames with first and last instances information. In both DataFrames 'column' data
        should be the same. If not - something went wrong and logic should be checked.
        :param first_instance_mask: DataFrame with 'first_row', 'column' columns
        :param last_instance_mask: DataFrame with 'last_row', 'column' columns
        :return:
        """
        columns_are_consistent = np.all(last_instance_mask['column'] == first_instance_mask['column'])
        if not columns_are_consistent:
            raise ValueError('Columns for objects init and killed time are not the same')

    @staticmethod
    def _concat_first_last_instance_masks(first_instance_mask, last_instance_mask):
        """
        Concatenate DataFrames with first and last instance information. Single row represents single object within it's
        whole lifetime.
        :param first_instance_mask: DataFrame with 'first_row', 'column' columns
        :param last_instance_mask: DataFrame with 'last_row', 'column' columns
        :return: DataFrame with columns 'first_row', 'last_row', 'column'
        """
        first_last_obj_instance_mask = pd.concat([first_instance_mask['first_row'],
                                                  last_instance_mask[['last_row', 'column']]], axis=1)
        return first_last_obj_instance_mask

    @staticmethod
    def _transform_first_last_mask_to_lifespan_mask(init_kill_mask: pd.DataFrame):
        # noinspection PyProtectedMember
        """
        Function to transform DataFrame with information about objects init and kill index (where single row is single
        object) to row - column - unique_id array where single row is single object within single time instance.
        For example input DataFrame:

        init_row    killed_row  column
        ------------------------------
        1           3           0
        5           6           2

        function should return DataFrame:

        row         column      unique_id
        ---------------------------------
        1           0           0
        2           0           0
        3           0           0
        5           2           1
        6           2           1

        :param init_kill_mask: DataFrame with columns 'first_row', 'last_row', 'column', signle row contains
        information about single object thorugh it's whole lifetime
        :return: DataFrame with columns 'row', 'column', 'unique_obj_id', single row represents single obj within single
        time instance
        >>> init_kill_mask = pd.DataFrame(np.array(([1, 3, 0],[5, 6, 2])), columns=['first_row', 'last_row', 'column'])
        >>> ObjectLifespanStatusBasedCreator._transform_first_last_mask_to_lifespan_mask(init_kill_mask)
           row  column  unique_obj_id
        0    1       0              0
        1    2       0              0
        2    3       0              0
        3    5       2              1
        4    6       2              1
        """

        """
        Get values from DataFrame as numpy array. Calculate how many rows are in the data - single row represents single
        object. Initialize empty list which should be appended by single object lifespan matrix.
        """
        init_kill_mask.sort_values(by=['first_row', 'column'], inplace=True, ignore_index=True)
        init_kill_mask_vals = init_kill_mask.values
        rows_len = init_kill_mask_vals.shape[0]
        lifespan_masks_list = []

        last_id = init_kill_mask['column'].max() + 1 #column 'column' represents ID-1

        kill_mask = init_kill_mask.copy(deep=True)
        kill_mask.sort_values(by=['last_row', 'first_row'], inplace=True, ignore_index=True)
        kill_mask['uids'] = [*range(last_id + 1, rows_len + last_id + 1)]
        """
        Loop over rows in init_kill_mask_vals - this represents loop over objects. For each object prepare 
        lifespan_matrix as numpy array which columns are: rows - column - unique_id
        """
        for row_idx in range(0, rows_len):
            row = init_kill_mask_vals[row_idx, :]
            obj_init_idx = row[0]
            obj_killed_idx = row[1]
            column = row[2]

            unique_id = ObjectLifespanStatusBasedCreator._find_unique_id(init_kill_mask, kill_mask,
                                                                         last_id, row_idx)
            """
            If object init_idx = 5, and obj_killed_idx = 10 then obj_lifespan_rows should be :
            ndarray([5, 6, 7, 8, 9, 10])
            """
            obj_lifespan_rows = np.arange(obj_init_idx, obj_killed_idx + 1)
            obj_life_len = obj_lifespan_rows.shape[0]
            obj_lifespan_columns = np.full(obj_life_len, column)
            obj_uniqe_id = np.full(obj_life_len, unique_id)
            """
            Stack 3 vectors to single matrix and append to list of matrices.
            """
            single_row_mask = np.vstack((obj_lifespan_rows, obj_lifespan_columns, obj_uniqe_id)).T
            lifespan_masks_list.append(single_row_mask)
        """
        At the end connect matrixes from single objects to one big matrix, and convert it to DataFrame with proper 
        columns names. 
        """
        if len(lifespan_masks_list) > 0:
            return pd.DataFrame(np.concatenate(lifespan_masks_list, axis=0),
                                columns=['row', 'column', 'unique_obj_id'])
        else:
            return pd.DataFrame(columns=['row', 'column', 'unique_obj_id'])  # return empty dataframe
        

    @staticmethod
    def _find_unique_id(init_kill_mask: pd.DataFrame, kill_mask: pd.DataFrame, last_id: int, row_idx: int):
        unique_id = -1
        if row_idx < last_id:
            unique_id = row_idx + 1
        else:
            id = init_kill_mask['column'][row_idx]
            unique_id_row = kill_mask.loc[(kill_mask['column'] == id)]
            unique_id = unique_id_row.iloc[0]['uids']
            kill_mask.drop(unique_id_row.index[0], inplace=True) 

        return unique_id