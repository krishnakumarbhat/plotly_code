import pandas as pd


def filter_only_cars(df: pd.DataFrame):
    df.query("object_class == 'ObjectClass.CAR'", inplace=True)


def filter_only_trucks(df: pd.DataFrame):
    df.query("object_class == 'ObjectClass.TRUCK'", inplace=True)


def filter_only_same_direction(df: pd.DataFrame):
    df.query('velocity_otg_x > 0.0', inplace=True)


def filter_only_opposite_direction(df: pd.DataFrame):
    df.query('velocity_otg_x < 0.0', inplace=True)


def filter_only_moving(df: pd.DataFrame):
    df.query("movement_status == 'MovementStatus.MOVING'", inplace=True)
