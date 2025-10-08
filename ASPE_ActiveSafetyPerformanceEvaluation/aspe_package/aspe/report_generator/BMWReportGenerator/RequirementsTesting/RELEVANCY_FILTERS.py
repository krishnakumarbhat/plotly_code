from aspe.evaluation.RadarObjectsEvaluation.Flags import SignalInBoundsFlag, IsMovableFlag, DistanceToCSOriginBelowThrFlag

DS_FILTERS = {
    'DS_01': {'est':          [SignalInBoundsFlag('position_y', min_value=-15.0, max_value=15.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_02': {'est':          [SignalInBoundsFlag('position_y', min_value=-15.0, max_value=15.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_03': {'est':          [SignalInBoundsFlag('position_y', min_value=-15.0, max_value=15.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_04': {'est':          [SignalInBoundsFlag('position_y', min_value=-15.0, max_value=15.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_05': {'est':          [SignalInBoundsFlag('position_y', min_value=-15.0, max_value=15.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_06': {'est':          [SignalInBoundsFlag('position_y', min_value=-90.0, max_value=90.0),
                               SignalInBoundsFlag('position_x', min_value=-100.0, max_value=100.0),
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=70.0)],
              'direction':    'parallel',
              'straightness': 'straight'},

    'DS_07': {'est':          [
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=40.0)],
              'direction':    'crossing',
              'straightness': 'straight'},

    'DS_09': {'est':          [
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=90.0)],
              'direction':    'crossing',
              'straightness': 'straight'},

    'DS_10': {'est':          [
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=90.0)],
              'direction':    'crossing',
              'straightness': 'straight'},

    'DS_11': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_12': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_13': {'est':          [
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=50.0)],
              'direction':    'crossing',
              'straightness': 'curve'},


    'DS_14': {'est':          [SignalInBoundsFlag('position_x', min_value=-2.0),
                               IsMovableFlag()],
              'ref':          [DistanceToCSOriginBelowThrFlag(thr=70.0)],
              'direction':    'crossing',
              'straightness': 'curve'},

    'DS_15': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_16': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_17': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'curve'},

    'DS_18': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'curve'},

    'DS_19': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'curve'},

    'DS_20': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_21': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_22': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_23': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_24': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_25': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_26': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_27': {'ref': [],
              'est': []},

    'DS_28': {'est':    [
        IsMovableFlag()],
        'ref':         [DistanceToCSOriginBelowThrFlag(thr=70.0)],
        'direction':    'parallel',
        'straightness': 'curve'},

    'DS_29': {'est':    [
        IsMovableFlag()],
        'ref':         [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_30': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_31': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_32': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_33': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_34': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_35': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_36': {'est':    [
        IsMovableFlag()],
        'ref':          [DistanceToCSOriginBelowThrFlag(thr=120.0)],
        'direction':    'crossing',
        'straightness': 'straight'},

    'DS_37': {'est':    [
        IsMovableFlag()],
        'ref':          [DistanceToCSOriginBelowThrFlag(thr=120.0)],
        'direction':    'crossing',
        'straightness': 'straight'},

    'DS_38': {'est':    [
        IsMovableFlag()],
        'ref':          [DistanceToCSOriginBelowThrFlag(thr=120.0)],
        'direction':    'crossing',
        'straightness': 'straight'},

    'DS_39': {'est':    [
        IsMovableFlag()],
        'ref':         [DistanceToCSOriginBelowThrFlag(thr=70.0)],
        'direction':    'crossing',
        'straightness': 'curve'},

    'DS_40': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_y', min_value=-90.0, max_value=70.0)],
        'direction':    'crossing',
        'straightness': 'straight'},

    'DS_41': {'ref': [],
              'est': []},

    'DS_42': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_43': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_y', min_value=-90.0, max_value=70.0)],
        'direction':    'crossing',
        'straightness': 'straight'},

    'DS_44': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

    'DS_45': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_46': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_47': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},
    'DS_48': {'est':    [
        IsMovableFlag()],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},


    'DS_50': {'est':    [],
        'ref':          [SignalInBoundsFlag('position_x', min_value=-90.0, max_value=70.0)],
        'direction':    'parallel',
        'straightness': 'straight'},

}
