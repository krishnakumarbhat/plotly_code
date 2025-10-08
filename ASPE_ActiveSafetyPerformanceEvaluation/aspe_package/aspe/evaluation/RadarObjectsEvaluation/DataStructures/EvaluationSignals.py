import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

relevancy_flag = SignalDescription(signature='relevancy_flag',
                                   dtype=bool,
                                   description='Flag indicating if entity is relevant for further processing',
                                   unit='-')

is_associated = SignalDescription(signature='is_associated',
                                  dtype=bool,
                                  description='Flag indicating if entity is associated with any other entity,'
                                              'Note, that it is it depends on association method if not relevant '
                                              'entity can be associated to relevant one',
                                  unit='-')

binary_classification = SignalDescription(signature='binary_classification',
                                          dtype='category',
                                          description='Binary classification enumeration see BCType from: '
                                                      'aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.BinaryClassifier',
                                          unit='-')

index_ref = SignalDescription(signature='index_ref',
                              dtype=np.int16,
                              description='Data frame based index of reference entity',
                              unit='-')

index_est = SignalDescription(signature='index_est',
                              dtype=np.int16,
                              description='Data frame based index of estimated entity',
                              unit='-')

unique_id_ref = SignalDescription(signature='unique_id_ref',
                                  dtype=np.int16,
                                  description='Unique ID of reference entity, see unique_id from:'
                                              'aspe.extractors.Interfaces.SignalDescription.GeneralSignals',
                                  unit='-')

unique_id_est = SignalDescription(signature='unique_id_est',
                                  dtype=np.int16,
                                  description='Unique ID of estimated entity, see unique_id from:'
                                              'aspe.extractors.Interfaces.SignalDescription.GeneralSignals',
                                  unit='-')

relevancy_flag_ref = SignalDescription(signature='relevancy_flag_ref',
                                       dtype=bool,
                                       description='Flag indicating if reference entity is relevant for further processing',
                                       unit='-')

relevancy_flag_est = SignalDescription(signature='relevancy_flag_est',
                                       dtype=bool,
                                       description='Flag indicating if estimated entity is relevant for further processing',
                                       unit='-')
