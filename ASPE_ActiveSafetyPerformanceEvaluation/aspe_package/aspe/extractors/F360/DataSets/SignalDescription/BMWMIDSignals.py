import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

vigem_timestamp = SignalDescription(signature='vigem_timestamp',
                                    dtype=np.float32,
                                    description='timestamp of Vigem data logger',
                                    unit='s')

publish_time_in_global_domain = SignalDescription(signature='publish_time_in_global_domain',
                                                  dtype=np.float32,
                                                  description='time when data was published',
                                                  unit='s')

system_latency = SignalDescription(signature='system_latency',
                                   dtype=np.float32,
                                   description='latency between measurement and publish time',
                                   unit='s')

ethernet_timestamp = SignalDescription(signature='ethernet_timestamp',
                                                  dtype=np.float32,
                                                  description='timestamp of ethernet data',
                                                  unit='s')

vigem_to_global_time_diff = SignalDescription(signature='vigem_to_global_time_diff',
                                              dtype=np.float32,
                                              description='difference between timestamps',
                                              unit='s')