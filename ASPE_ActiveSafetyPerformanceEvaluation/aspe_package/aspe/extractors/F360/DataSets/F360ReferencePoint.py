from aspe.extractors.F360.Enums.f360_reference_point import F360ReferencePointEnum


class F360ReferencePoint:
    F360ReferencePointMap = {
        F360ReferencePointEnum.CENTER: (0.5, 0.5),
        F360ReferencePointEnum.FRONT_LEFT: (0, 1),
        F360ReferencePointEnum.FRONT: (0.5, 1),
        F360ReferencePointEnum.FRONT_RIGHT: (1, 1),
        F360ReferencePointEnum.RIGHT: (1, 0.5),
        F360ReferencePointEnum.REAR_RIGHT: (1, 0),
        F360ReferencePointEnum.REAR: (0.5, 0),
        F360ReferencePointEnum.REAR_LEFT: (0, 0),
        F360ReferencePointEnum.LEFT: (0, 0.5),
    }

    @staticmethod
    def get_reference_point_map_items():
        return F360ReferencePoint.F360ReferencePointMap.items()
