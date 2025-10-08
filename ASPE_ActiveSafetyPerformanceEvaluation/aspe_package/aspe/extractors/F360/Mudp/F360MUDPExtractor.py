# coding=utf-8
"""
F360 Main Extractor
"""
from warnings import warn

from aspe.extractors.F360.DataSets.F360MudpExtractedData import F360MudpExtractedData
from aspe.extractors.F360.Mudp.Builders.GDSR_output.F360MudpGdsrOutputBuilder import F360MudpGdsrOutput
from aspe.extractors.F360.Mudp.Builders.MetaData.F360MudpMetaDataBuilder import F360MudpMetaDataBuilder
from aspe.extractors.F360.Mudp.Exceptions.MissingStreamException import MissingStreamException
from aspe.extractors.F360.Mudp.Exceptions.VersionNotSupportedException import VersionNotSupportedException
from aspe.extractors.F360.Mudp.Selectors.F360MudpDetectionsBuilderSelector import F360MudpDetectionsBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpExecutionTimeInfoBuilderSelector import (
    F360MudpExecutionTimeInfoBuilderSelector,
)
from aspe.extractors.F360.Mudp.Selectors.F360MudpHostBuilderSelector import F360MudpHostBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpInternalObjectsBuilderSelector import (
    F360MudpInternalObjectsBuilderSelector,
)
from aspe.extractors.F360.Mudp.Selectors.F360MudpOALObjectsBuildersSelector import F360MudpOALObjectsBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpOALObjectsHeaderBuildersSelector import (
    F360MudpOALObjectsHeaderBuilderSelector,
)
from aspe.extractors.F360.Mudp.Selectors.F360MudpObjectsBuilderSelector import F360MudpObjectsBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpSensorsBuilderSelector import F360MudpSensorsBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpStaticEnvironmentBuilderSelector import (
    F360MudpStaticEnvironmentBuilderSelector,
)
from aspe.extractors.F360.Mudp.Selectors.F360MudpTrackerInfoBuilderSelector import F360MudpTrackerInfoBuilderSelector
from aspe.extractors.F360.Mudp.Selectors.F360MudpTrailerBuilderSelector import F360MudpTrailerBuilderSelector
from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.utilities.SupportingFunctions import calc_dets_azimuth_vcs


class F360MUDPExtractor(IExtractor):
    """
    Class for extracting tracker F360 data.
    Main extraction method. Create and fill data container - F360ExtractedData.
    """

    def __init__(self, *,
                 f_extract_objects=True,
                 f_extract_internal_objects=True,
                 f_extract_sensors=True,
                 f_extract_host=True,
                 f_extract_trailer=False,
                 f_extract_detections=True,
                 f_extract_execution_time_info=False,
                 f_extract_oal_objects=False,
                 f_extract_gdsr_objects=False,
                 f_extract_stat_env=False,
                 f_builders_extract_raw_signals=True,
                 f_raise_exc_if_stream_missing=False,
                 f_raise_exc_if_version_not_supported=False):
        super().__init__()
        self._f_extract_objects = f_extract_objects
        self._f_extract_internal_objects = f_extract_internal_objects
        self._f_extract_sensors = f_extract_sensors
        self._f_extract_host = f_extract_host
        self._f_extract_trailer = f_extract_trailer
        self._f_extract_detections = f_extract_detections
        self._f_extract_execution_time_info = f_extract_execution_time_info
        self._f_extract_oal_objects = f_extract_oal_objects
        self._f_extract_gdsr_objects = f_extract_gdsr_objects
        self._f_extract_stat_env = f_extract_stat_env
        self._f_builders_extract_raw_signals = f_builders_extract_raw_signals
        self._f_raise_exc_if_stream_missing = f_raise_exc_if_stream_missing
        self._f_raise_exc_if_version_not_supported = f_raise_exc_if_version_not_supported

    def extract_data(self, parsed_data):
        """
        Calls builder to build F360 metadata and F360 data container.
        :return: F360ExtractedData object
        """
        extracted = F360MudpExtractedData()
        extracted.metadata = self._build_metadata(parsed_data)
        extracted.tracker_info = self._build_data_set(parsed_data, 'tracker info', F360MudpTrackerInfoBuilderSelector)
        extracted.objects = self._build_data_set(parsed_data, 'objects', F360MudpObjectsBuilderSelector,
                                                 f_create_builder=self._f_extract_objects)
        extracted.internal_objects = self._build_data_set(parsed_data, 'internal objects',
                                                          F360MudpInternalObjectsBuilderSelector,
                                                          f_create_builder=self._f_extract_internal_objects)
        extracted.sensors = self._build_data_set(parsed_data, 'sensors', F360MudpSensorsBuilderSelector,
                                                 f_create_builder=self._f_extract_sensors)
        extracted.host = self._build_data_set(parsed_data, 'host', F360MudpHostBuilderSelector,
                                              f_create_builder=self._f_extract_host)
        extracted.detections = self._build_data_set(parsed_data, 'detections', F360MudpDetectionsBuilderSelector,
                                                    f_create_builder=self._f_extract_detections)
        extracted.execution_time_info = self._build_data_set(parsed_data, 'execution time info',
                                                             F360MudpExecutionTimeInfoBuilderSelector,
                                                             f_create_builder=self._f_extract_execution_time_info)
        extracted.oal_objects = self._build_data_set(parsed_data, 'OAL objects', F360MudpOALObjectsBuilderSelector,
                                                     f_create_builder=self._f_extract_oal_objects)
        extracted.oal_objects_header = self._build_data_set(parsed_data, 'OAL objects',
                                                            F360MudpOALObjectsHeaderBuilderSelector,
                                                            f_create_builder=self._f_extract_oal_objects)
        extracted.gdsr_output = self._extract_oal_gdsr_output(parsed_data, self._f_extract_gdsr_objects)
        extracted.static_environment = self._build_data_set(parsed_data, 'static environment',
                                                            F360MudpStaticEnvironmentBuilderSelector,
                                                            f_create_builder=self._f_extract_stat_env)
        extracted.trailer = self._build_data_set(parsed_data, 'trailer', F360MudpTrailerBuilderSelector,
                                                 f_create_builder=self._f_extract_trailer)

        self.oal_cross_extraction(extracted)
        self.extract_relative_velocity(extracted)
        calc_dets_azimuth_vcs(extracted)

        return extracted

    def _build_metadata(self, parsed_data):
        metadata_builder = F360MudpMetaDataBuilder(parsed_data)
        return metadata_builder.build()

    def _build_data_set(self, parsed_data: dict, dataset_name: str, selector_class, f_create_builder: bool = True):
        data_set = None
        if f_create_builder:
            builder = self._select_and_create_builder(parsed_data, dataset_name, selector_class)
            if builder is not None:
                data_set = builder.build()
        return data_set

    def _select_and_create_builder(self, parsed_data: dict, dataset_name: str, selector_class):
        builder = None
        selector = selector_class()
        try:
            builder_class = selector.select_builder(parsed_data)
            builder = builder_class(parsed_data, f_extract_raw_signals=self._f_builders_extract_raw_signals)
        except MissingStreamException as exc:
            if self._f_raise_exc_if_stream_missing:
                raise exc
            else:
                warn(f'{exc} {dataset_name.capitalize()} will not be extracted')
        except VersionNotSupportedException as exc:
            if self._f_raise_exc_if_version_not_supported:
                raise exc
            else:
                warn(f'{exc} {dataset_name.capitalize()} will not be extracted')
        return builder

    def oal_cross_extraction(self, extracted: F360MudpExtractedData):
        if extracted.oal_objects is not None and extracted.oal_objects_header:
            rows = extracted.oal_objects.signals.loc[:, 'log_data_row'].to_numpy()
            extracted.oal_objects.signals.loc[:, 'scan_index'] = extracted.oal_objects_header.signals.iloc[rows, :][
                'scan_index'].to_numpy()
            extracted.oal_objects.signals.loc[:, 'timestamp'] = extracted.oal_objects_header.signals.iloc[rows, :][
                'timestamp'].to_numpy()

            if extracted.host is not None and extracted.host.dist_of_rear_axle_to_front_bumper is not None:
                extracted.oal_objects._transfer_to_vcs(extracted.host.dist_of_rear_axle_to_front_bumper)
            else:
                warn('Information about distance from rear axle to front bumper missing - OAL objects will be in ISO '
                     'VRACS coordinate system')

    def extract_relative_velocity(self, extracted: F360MudpExtractedData):
        if extracted.host is not None:
            host_sub_df = extracted.host.signals.set_index('scan_index').loc[:,
                          ['velocity_otg_x', 'velocity_otg_y', 'yaw_rate']]

            def calc_rel_velocities(objects_signals):
                objs_sub_df = objects_signals.loc[:,
                              ['scan_index', 'velocity_otg_x', 'velocity_otg_y', 'position_x', 'position_y']]
                objs_host_df = objs_sub_df.join(host_sub_df, on='scan_index', lsuffix='_obj', rsuffix='_host')

                host_yaw_infl_x = -objs_host_df.loc[:, 'position_y'] * objs_host_df.loc[:, 'yaw_rate']
                host_yaw_infl_y = objs_host_df.loc[:, 'position_x'] * objs_host_df.loc[:, 'yaw_rate']

                objs_vel_x_rel = (objs_host_df.loc[:, 'velocity_otg_x_obj'] -
                                  objs_host_df.loc[:, 'velocity_otg_x_host'] - host_yaw_infl_x)
                objs_vel_y_rel = (objs_host_df.loc[:, 'velocity_otg_y_obj'] -
                                  objs_host_df.loc[:, 'velocity_otg_y_host'] - host_yaw_infl_y)

                objects_signals.loc[:, 'velocity_rel_x'] = objs_vel_x_rel
                objects_signals.loc[:, 'velocity_rel_y'] = objs_vel_y_rel

            if extracted.objects is not None:
                try:
                    calc_rel_velocities(extracted.objects.signals)
                except ValueError:
                    warn('Relative velocity for reduced objects cannot be extracted')

            if extracted.internal_objects is not None:
                try:
                    calc_rel_velocities(extracted.internal_objects.signals)
                except ValueError:
                    warn('Relative velocity for internal objects cannot be extracted')
        else:
            warn('Relative velocity cannot be extracted - missing host data')

    @staticmethod
    def _get_extractor_name():
        return 'F360MUDP'

    def _extract_oal_gdsr_output(self, parsed_data, f_extract_gdsr_objects):
        output = None
        if 33 in parsed_data['parsed_data'] and f_extract_gdsr_objects:
            if 'gdsr_tracker_output' in parsed_data['parsed_data'][33]:
                output = F360MudpGdsrOutput(parsed_data, self._f_builders_extract_raw_signals).build()
        return output
