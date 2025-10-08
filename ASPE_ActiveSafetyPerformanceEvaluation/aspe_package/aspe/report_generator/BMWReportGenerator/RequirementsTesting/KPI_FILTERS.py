from aspe.evaluation.RadarObjectsEvaluation.Flags import DistanceToCSOriginBetweenThrFlag, \
    SignalInBoundsFlag, DistanceToCSOriginBelowThrFlag

KPI_FILTERS = dict(
    {
        'x-position of reference point':
            {
                '1-sigma':
                    {'0.2-12': [DistanceToCSOriginBetweenThrFlag(thr_low=0.2, thr_high=12)],
                     '12-40':  [DistanceToCSOriginBetweenThrFlag(thr_low=12, thr_high=40)],
                     '40-100': [DistanceToCSOriginBetweenThrFlag(thr_low=40, thr_high=100)],
                     }
            },

        'y-position of reference point':
            {
                '1-sigma':
                    {
                        '0.2-12': [DistanceToCSOriginBetweenThrFlag(thr_low=0.2, thr_high=12)],
                        '12-40':  [DistanceToCSOriginBetweenThrFlag(thr_low=12, thr_high=40)],
                        '40-100': [DistanceToCSOriginBetweenThrFlag(thr_low=40, thr_high=100)],
                    }
            },

        'vx abs':
            {
                '1-sigma':
                    {
                        'ax>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                    thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]},
                'max error':
                    {
                        'ax>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]
                    },
            },

        'vy abs':
            {
                '1-sigma':
                    {
                        'ay>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                      pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                     pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]},
                'max error':
                    {
                        'ay>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                      pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                     pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)],
                    }
            },
        'vx rel':
            {
                '1-sigma':
                    {
                        'ax>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                    pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]},
                'max error':
                    {
                        'ax>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                      pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                     pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)],
                    }
            },

        'vy rel':
            {
                '1-sigma':
                    {
                        'ay>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                      pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                     pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]},
                'max error':
                    {
                        'ay>2 a<3': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=0, max_value=2),
                                     DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                      pos_y_signal_name='acceleration_otg_y',
                                                                      thr_low=0, thr_high=3)],
                        'a>3 a<6': [DistanceToCSOriginBetweenThrFlag(pos_x_signal_name='acceleration_otg_x',
                                                                     pos_y_signal_name='acceleration_otg_y',
                                                                     thr_low=3, thr_high=6)]
                    }
            },

        #
        # 'velocity_long':
        #     {'acc_long_2': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=0, max_value=2),
        #                     DistanceToCSOriginBelowThrFlag(pos_x_signal_name='acceleration_otg_x', pos_y_signal_name='acceleration_otg_y',
        #                                                    thr=3)],
        #      'acc_long_6': [SignalInBoundsFlag(signal_name='acceleration_otg_y', min_value=2, max_value=6)]},
        #
        # 'velocity_lat':
        #     {'acc_lat_2': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=0, max_value=2),
        #                    DistanceToCSOriginBelowThrFlag(pos_x_signal_name='acceleration_otg_x', pos_y_signal_name='acceleration_otg_y',
        #                                                   thr=3)],
        #      'acc_lat_6': [SignalInBoundsFlag(signal_name='acceleration_otg_x', min_value=2, max_value=6)]},

        'length':
            {
                '1-sigma':
                    {
                        '0.2-2':  [DistanceToCSOriginBetweenThrFlag(thr_low=0.2, thr_high=2)],
                        '2-12':   [DistanceToCSOriginBetweenThrFlag(thr_low=2, thr_high=12)],
                        '12-40':  [DistanceToCSOriginBetweenThrFlag(thr_low=12, thr_high=40)],
                        '40-100': [DistanceToCSOriginBetweenThrFlag(thr_low=40, thr_high=100)],
                    },
            },
        'width':
            {
                '1-sigma':
                    {
                        '0.2-2':  [DistanceToCSOriginBetweenThrFlag(thr_low=0.2, thr_high=2)],
                        '2-12':   [DistanceToCSOriginBetweenThrFlag(thr_low=2, thr_high=12)],
                        '12-40':  [DistanceToCSOriginBetweenThrFlag(thr_low=12, thr_high=40)],
                        '40-100': [DistanceToCSOriginBetweenThrFlag(thr_low=40, thr_high=100)],
                    },
            },
    },
)
pass
