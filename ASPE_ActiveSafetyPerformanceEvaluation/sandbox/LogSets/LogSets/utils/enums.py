from enum import Enum


class ObjectClass(Enum):
    Object_Undetermined = 0
    Car = 1
    Motorcycle = 2
    Truck = 3
    Pedestrian = 4
    Pole = 5
    Tree = 6
    Animal = 7
    General_on_road_object_detected = 8
    Bicycle = 9
    Vehicle_unidentified = 10