/** \file
 * This file contains unit tests for content of f360_sensor_occlusion_info.cpp file
 */

#include "f360_sensor_occlusion_info.h"
#include "f360_sensor_occlusion_info_helpers.h"
#include "f360_calc_point_scs_position.h"
#include <algorithm>
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_sensor_occlusion_info__Update_Calibs
 *  @{
 */

 /** \brief
  * Test group of Sensor_Occlusion_Info_T class Update_Calibs method. Tests verify whether sensor calibrations are properly selected.
  */
TEST_GROUP(f360_sensor_occlusion_info__Update_Calibs)
{
   Sensor_Occlusion_Info_T sensor_occlusion_info{};
   F360_Radar_Sensor_T sensor{};
   const float32_t range_uncertainty = 0.0F;

   /** \setup
    * Set up sensor basic properties
    */
   TEST_SETUP()
   {
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.fov_min_az_rad[sensor.variable.look_id] = -0.5F;
      sensor.constant.fov_max_az_rad[sensor.variable.look_id] = 0.5F;
      sensor.constant.range_limits[sensor.variable.look_id] = 50.0F;

      sensor.constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      sensor.constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;
      sensor.constant.mounting_position.vcs_position.lateral = 2.0F;
      sensor.constant.mounting_position.vcs_position.height = 1.0F;
      sensor.constant.mounting_position.vcs_position.longitudinal = -1.0F;
   }
};

/** \purpose
 * Purpose of this test is to verify whether when sensor is valid, Update_Calibs method properly
 * selects sensor properties and saves them inside private members.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Calibs, f360_sensor_occlusion_info__Sensor_Is_Valid_Properly_Updates_Sensor_FOV)
{
   /** \precond
    * Set sensors is_valid flag as true
    */
    sensor.variable.is_valid = true;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);

   /** \result
    * Get sensor calibrations from sensor
    * Get sectors width
    * Check whether sensor min az is equal to -0.5F
    * Check whether sensor max az is equal to -0.5F
    * Check whether sensor max range is equal to 50.0F
    * Check whether sectors width is equal to 1.0F/num_sectors
    */
   const Sensor_FOV_T sensor_fov = sensor_occlusion_info.Get_Props();
   const float32_t width = sensor_occlusion_info.Get_Sectors_Width();
   const float32_t expected_sector_width = 1.0F / static_cast<float32_t>(occlusion_num_sectors);
   DOUBLES_EQUAL(-0.5F, sensor_fov.Get_Min_Az(), F360_EPSILON);
   DOUBLES_EQUAL(1.0F, sensor_fov.Get_Azimuth_Spread(), F360_EPSILON);
   DOUBLES_EQUAL(50.0F, sensor_fov.Get_Max_Range(), F360_EPSILON);
   DOUBLES_EQUAL(expected_sector_width, width, 1e-4F);
}

/** \purpose
 * Purpose of this test is to verify whether when sensor is invalid, Update_Calibs method does
 * not modify sensor fov.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Calibs, f360_sensor_occlusion_info__Sensor_Is_Invalid_Does_Not_Update_FOV)
{
   /** \precond
    * Set sensor is_valid flag as true
    */
   sensor.variable.is_valid = false;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);

   /** \result
    * Get sensor calibrations from sensor
    * Get sectors width
    * Check whether sensor min az is equal to 0.0F
    * Check whether sensor max az is equal to 0.0F
    * Check whether sensor max range is equal to 0.0F
    * Check whether sectors width is equal to 0.0F
    */
   const Sensor_FOV_T sensor_fov = sensor_occlusion_info.Get_Props();
   const float32_t width = sensor_occlusion_info.Get_Sectors_Width();
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Min_Az(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Azimuth_Spread(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Max_Range(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, width, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether when sensor calibrations are invalid (max az is below min az), further sensor updates
 * are not done.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Calibs, f360_sensor_occlusion_info__Sensor_Azimuth_Limits_Invalid_No_Further_Updates)
{
   /** \precond
    * Set sensor is_valid flag as true
    * Inverses sensor max fov and min fov
    */
   sensor.variable.is_valid = true;
   std::swap(sensor.constant.fov_min_az_rad[sensor.variable.look_id], sensor.constant.fov_max_az_rad[sensor.variable.look_id]);

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);

   /** \result
    * Get sensor calibrations from sensor
    * Get sectors width
    * Check whether sensor min az is equal to 0.5F
    * Check whether sensor max az is equal to -0.5F
    * Check whether sensor max range is equal to 50.0F
    * Check whether sectors width is equal to 0.0F
    */
   const Sensor_FOV_T sensor_fov = sensor_occlusion_info.Get_Props();
   const float32_t width = sensor_occlusion_info.Get_Sectors_Width();
   DOUBLES_EQUAL(0.5F, sensor_fov.Get_Min_Az(), F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, sensor_fov.Get_Azimuth_Spread(), F360_EPSILON);
   DOUBLES_EQUAL(50.0F, sensor_fov.Get_Max_Range(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, width, F360_EPSILON);
}
/** @}*/

/** \defgroup  f360_sensor_occlusion_info__reset
 *  @{
 */

 /** \brief
  * Test group of Sensor_Occlusion_Info_T class Reset method. Tests verify whether all fields are properly reset.
  */
TEST_GROUP(f360_sensor_occlusion_info__reset)
{
   Sensor_Occlusion_Info_T sensor_occlusion_info{};
   F360_Radar_Sensor_T sensor{};
   const float32_t range_uncertainty = 0.0F;

   /** \setup
    * Set up sensor basic properties
    */
   TEST_SETUP()
   {
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.fov_min_az_rad[sensor.variable.look_id] = -0.5F;
      sensor.constant.fov_max_az_rad[sensor.variable.look_id] = 0.5F;
      sensor.constant.range_limits[sensor.variable.look_id] = 50.0F;

      sensor.constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      sensor.constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;
      sensor.constant.mounting_position.vcs_position.lateral = 2.0F;
      sensor.constant.mounting_position.vcs_position.height = 1.0F;
      sensor.constant.mounting_position.vcs_position.longitudinal = -1.0F;
   }
};

/** \purpose
 * Purpose of this test is to verify whether reset method resets internal Sensor_Occlsion_Info_T fields.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__reset, f360_sensor_occlusion_info__reset_works_as_expected)
{
   /** \precond
    * Set sensor is_valid flag as true
    * Call Update_Calibs() method
    */
   sensor.variable.is_valid = true;
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);

   /** \action
    * Call Reset method
    */
   sensor_occlusion_info.Reset();

   /** \result
    * Get sensor calibrations from sensor
    * Get sectors width
    * Get sectors
    * Check whether sensor min az is equal to 0.0F
    * Check whether sensor max az is equal to 0.0F
    * Check whether sensor max range is equal to 0.0F
    * Check whether sectors width is equal to 0.0F
    * Check wheter all sectors ranges are equal to 0.0F
    */
   const Sensor_FOV_T sensor_fov = sensor_occlusion_info.Get_Props();
   const float32_t width = sensor_occlusion_info.Get_Sectors_Width();
   const Occlusion_Sector_T(&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Min_Az(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Azimuth_Spread(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, sensor_fov.Get_Max_Range(), F360_EPSILON);
   DOUBLES_EQUAL(0.0F, width, F360_EPSILON);
   for (unsigned int i = 0; i < occlusion_num_sectors; i++)
   {
      DOUBLES_EQUAL(0.0F, sectors[i].Get_Range(), F360_EPSILON);
   }
}
/** @}*/

/** \defgroup  f360_sensor_occlusion_info__Update_Occlusion
 *  @{
 */

 /** \brief
  * Test group of Sensor_Occlusion_Info_T class Update_Occlusion_With_Object_Visible_Edges method.
  * Tests verify whether sensor occlusion information is properly updated.
  */
TEST_GROUP(f360_sensor_occlusion_info__Update_Occlusion)
{
   Sensor_Occlusion_Info_T sensor_occlusion_info{};
   F360_Radar_Sensor_T sensor{};
   VCS_Track_Visible_Edges_T vcs_visible_edges{};
   const float32_t max_range = 100.0F;
   const float32_t range_uncertainty = 0.0F;

   /** \setup
    * Set up sensor basic properties
    * Set up two vcs visible edges
    * First edge has to be valid
    * Second edge has to be invalid
    * Update sensor calibrations
    */
   TEST_SETUP()
   {
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.fov_min_az_rad[sensor.variable.look_id] = -1.5F;
      sensor.constant.fov_max_az_rad[sensor.variable.look_id] = 1.5F;
      sensor.constant.range_limits[sensor.variable.look_id] = max_range;

      sensor.constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      sensor.constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;
      sensor.constant.mounting_position.vcs_position.lateral = 2.0F;
      sensor.constant.mounting_position.vcs_position.height = 1.0F;
      sensor.constant.mounting_position.vcs_position.longitudinal = -1.0F;

      sensor.variable.is_valid = true;

      vcs_visible_edges.first_edge.f_valid = true;

      vcs_visible_edges.first_edge.point_begin.y = -4.0F;
      vcs_visible_edges.first_edge.point_begin.x = 4.0F;

      vcs_visible_edges.first_edge.point_end.y = 4.0F;
      vcs_visible_edges.first_edge.point_end.x = 4.0F;


      vcs_visible_edges.second_edge.f_valid = false;

      vcs_visible_edges.second_edge.point_begin.y = -4.0F;
      vcs_visible_edges.second_edge.point_begin.x = 2.0F;

      vcs_visible_edges.second_edge.point_end.y = 4.0F;
      vcs_visible_edges.second_edge.point_end.x = 2.0F;

      sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);
   }
};

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is updated with edge that is in FOV of sensor.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Visible_Edges_Are_Used_To_Update_Occlusion_Information)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Calculate azimuth of first and last sector
    * Calculate index of first and last sector
    * Calculate number of sectors that should be updated
    */
   const SCS_Position_T scs_pos_start = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_begin.x, vcs_visible_edges.first_edge.point_begin.y, sensor_occlusion_info.Get_Mounting_Position());
   const SCS_Position_T scs_pos_end = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_end.x, vcs_visible_edges.first_edge.point_end.y, sensor_occlusion_info.Get_Mounting_Position());

   const float32_t azim_start = scs_pos_start.polar.azimuth;
   const float32_t azim_end = scs_pos_end.polar.azimuth;

   const int32_t starting_sector_idx = Calc_Starting_Sector_IDx(azim_start, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());
   const int32_t ending_sector_idx = Calc_Ending_Sector_IDx(azim_end, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());

   const int32_t num_sectors_to_update = ending_sector_idx - starting_sector_idx;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is greater or equal to (num_sectors_to_update - 2)
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[starting_sector_idx], &sectors[ending_sector_idx + 1], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_TRUE(num_updated_sectors >= (num_sectors_to_update - 2));
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is not updated when sensor calibrations are not valid
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Occlusion_Not_Updated_If_Sensor_Calibrations_Are_Not_Valid)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Set sensor is_valid flag to false
    * Call Update_Calibs
    */
   sensor.variable.is_valid = false;
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);


   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is equal to 0
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[0], &sectors[occlusion_num_sectors], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_EQUAL(0, num_updated_sectors);
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is updated with edge that left point is not in FOV but right one is in FOV.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Occlusion_Updated_With_Edge_That_First_Point_Is_Out_Of_FOV)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Modify position of first point of first edge to be placed outside of sensor FOV
    * Calculate azimuth of first and last sector
    * Calculate index of first and last sector
    * Calculate number of sectors that should be updated
    */
   vcs_visible_edges.first_edge.point_begin.y = -100.0F;

   const SCS_Position_T scs_pos_start = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_begin.x, vcs_visible_edges.first_edge.point_begin.y, sensor_occlusion_info.Get_Mounting_Position());
   const SCS_Position_T scs_pos_end = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_end.x, vcs_visible_edges.first_edge.point_end.y, sensor_occlusion_info.Get_Mounting_Position());

   const float32_t azim_start = scs_pos_start.polar.azimuth;
   const float32_t azim_end = scs_pos_end.polar.azimuth;

   const int32_t starting_sector_idx = Calc_Starting_Sector_IDx(azim_start, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());
   const int32_t ending_sector_idx = Calc_Ending_Sector_IDx(azim_end, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());

   const int32_t num_sectors_to_update = ending_sector_idx - starting_sector_idx;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is greater or equal to (num_sectors_to_update - 1)
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[starting_sector_idx], &sectors[ending_sector_idx + 1], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_TRUE(num_updated_sectors >= (num_sectors_to_update - 1));
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is updated with edge that right point is in FOV but right one is not in FOV.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Occlusion_Updated_With_Edge_That_Second_Point_Is_Out_Of_FOV)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Modify position of second point of first edge to be placed outside of sensor FOV
    * Calculate azimuth of first and last sector
    * Calculate index of first and last sector
    * Calculate number of sectors that should be updated
    */
   vcs_visible_edges.first_edge.point_end.y = 100.0F;

   const SCS_Position_T scs_pos_start = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_begin.x, vcs_visible_edges.first_edge.point_begin.y, sensor_occlusion_info.Get_Mounting_Position());
   const SCS_Position_T scs_pos_end = Calc_Point_SCS_Position(vcs_visible_edges.first_edge.point_end.x, vcs_visible_edges.first_edge.point_end.y, sensor_occlusion_info.Get_Mounting_Position());

   const float32_t azim_start = scs_pos_start.polar.azimuth;
   const float32_t azim_end = scs_pos_end.polar.azimuth;

   const int32_t starting_sector_idx = Calc_Starting_Sector_IDx(azim_start, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());
   const int32_t ending_sector_idx = Calc_Ending_Sector_IDx(azim_end, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());

   const int32_t num_sectors_to_update = ending_sector_idx - starting_sector_idx;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is greater or equal to (num_sectors_to_update - 1)
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[starting_sector_idx], &sectors[ending_sector_idx + 1], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_TRUE(num_updated_sectors >= (num_sectors_to_update - 1));
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is not updated when edge is out of FOV.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Occlusion_Not_Updated_When_Edge_Is_Out_Of_FOV)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Modify lateral positions of first edge so it is not in FOV of sensor
    * Calculate azimuth of first and last sector
    * Calculate index of first and last sector
    * Calculate number of sectors that should be updated
    */
   vcs_visible_edges.first_edge.point_begin.y = -100.0F;
   vcs_visible_edges.first_edge.point_end.y = -80.0F;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is equal to 0
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[0], &sectors[occlusion_num_sectors], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_EQUAL(0, num_updated_sectors);
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is updated with second edge that is in FOV of sensor.
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Second_Edge_Is_Used_To_Update_Occlusion)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Set first edge as invalid
    * Set second edge as valid
    * Calculate azimuth of first and last sector
    * Calculate index of first and last sector
    * Calculate number of sectors that should be updated
    */
   vcs_visible_edges.first_edge.f_valid = false;
   vcs_visible_edges.second_edge.f_valid = true;

   const SCS_Position_T scs_pos_start = Calc_Point_SCS_Position(vcs_visible_edges.second_edge.point_begin.x, vcs_visible_edges.second_edge.point_begin.y, sensor_occlusion_info.Get_Mounting_Position());
   const SCS_Position_T scs_pos_end = Calc_Point_SCS_Position(vcs_visible_edges.second_edge.point_end.x, vcs_visible_edges.second_edge.point_end.y, sensor_occlusion_info.Get_Mounting_Position());

   const float32_t azim_start = scs_pos_start.polar.azimuth;
   const float32_t azim_end = scs_pos_end.polar.azimuth;

   const int32_t starting_sector_idx = Calc_Starting_Sector_IDx(azim_start, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());
   const int32_t ending_sector_idx = Calc_Ending_Sector_IDx(azim_end, sensor_occlusion_info.Get_Props().Get_Min_Az(), sensor_occlusion_info.Get_Sectors_Width());

   const int32_t num_sectors_to_update = ending_sector_idx - starting_sector_idx;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check whether number of sectors with updated range is greater or equal to (num_sectors_to_update - 2)
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   const int32_t num_updated_sectors = std::count_if(&sectors[starting_sector_idx], &sectors[ending_sector_idx + 1], [&](const Occlusion_Sector_T& sector) {
      return sector.Get_Range() < max_range;
   });
   CHECK_TRUE(num_updated_sectors >= (num_sectors_to_update - 2));
}

/** \purpose
 * Purpose of this test is to verify sensor occlusion information is accumulated from both edges
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__Update_Occlusion, f360_sensor_occlusion_info__Both_Edges_Are_Used_To_Update_Occlusion)
{
   /** \precond
    * All basic input is set in TEST_SETUP()
    * Set second edge as valid
    * Modify beginning point of first edge to be placed beyond minimal azimuth of sensor FOV
    * Modify ending point of second edge to be placed beyond maximal azimuth of sensor FOV
    */
   vcs_visible_edges.second_edge.f_valid = true;

   vcs_visible_edges.first_edge.point_begin.y = -100.0F;
   vcs_visible_edges.second_edge.point_end.y = 100.0F;

   /** \action
    * Call Update_Calibs() method
    */
   sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);

   /** \result
    * Get occlusion sectors
    * Check wheter all sectors were updated
    */
   const Occlusion_Sector_T (&sectors)[occlusion_num_sectors] = sensor_occlusion_info.Get_Occlusion_Sectors();
   for (unsigned int i = 0; i < occlusion_num_sectors; i++)
   {
      const float32_t sector_range = sectors[i].Get_Range();
      CHECK_TRUE(sector_range < max_range);
   }
}
/** @}*/



/** \defgroup  f360_sensor_occlusion_info__determine_occlusion_status
 *  @{
 */

 /** \brief
  * Test group of Determine_Occlusion_Status class Reset method. Tests verify whether point occlusion status is properly determined
  */
TEST_GROUP(f360_sensor_occlusion_info__determine_occlusion_status)
{
   Sensor_Occlusion_Info_T sensor_occlusion_info{};
   F360_Radar_Sensor_T sensor{};
   VCS_Track_Visible_Edges_T vcs_visible_edges{};
   float32_t vcs_long_posn{};
   float32_t vcs_lat_posn{};
   const float32_t range_uncertainty = 0.0F;

   /** \setup
    * Set up sensor basic properties
    * Set up two vcs visible edges
    * First edge has to be valid
    * Second edge has to be invalid
    * Update sensor calibrations
    * Update sensor occlusion
    */
   TEST_SETUP()
   {
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.fov_min_az_rad[sensor.variable.look_id] = -1.5F;
      sensor.constant.fov_max_az_rad[sensor.variable.look_id] = 1.5F;
      sensor.constant.range_limits[sensor.variable.look_id] = 100.0F;

      sensor.constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      sensor.constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;
      sensor.constant.mounting_position.vcs_position.lateral = 2.0F;
      sensor.constant.mounting_position.vcs_position.height = 1.0F;
      sensor.constant.mounting_position.vcs_position.longitudinal = -1.0F;

      sensor.variable.is_valid = true;

      vcs_visible_edges.first_edge.f_valid = true;

      vcs_visible_edges.first_edge.point_begin.y = -4.0F;
      vcs_visible_edges.first_edge.point_begin.x = 4.0F;

      vcs_visible_edges.first_edge.point_end.y = 4.0F;
      vcs_visible_edges.first_edge.point_end.x = 4.0F;


      vcs_visible_edges.second_edge.f_valid = true;

      vcs_visible_edges.second_edge.point_begin.y = -4.0F;
      vcs_visible_edges.second_edge.point_begin.x = 2.0F;

      vcs_visible_edges.second_edge.point_end.y = 4.0F;
      vcs_visible_edges.second_edge.point_end.x = 2.0F;

      sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);
      sensor_occlusion_info.Update_Occlusion(vcs_visible_edges);
   }
};

/** \purpose
 * Purpose of this test is to verify whether when point is visible - function returns OCCLUSION_STATUS_VISIBLE 
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__determine_occlusion_status, f360_sensor_occlusion_info__point_is_visible_returns_occlusion_status_visible)
{
   /** \precond
    * Set up position of point to make it be visible
    */
   vcs_lat_posn = -0.3F;
   vcs_long_posn = 1.0F;

   /** \action
    * Call tested method
    */
   const Occlusion_Status_T occl_status = sensor_occlusion_info.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check wheter occl_status is equal to OCCLUSION_STATUS_VISIBLE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_VISIBLE, occl_status);
}

/** \purpose
 * Purpose of this test is to verify whether when point is on edeg of visibility - function returns OCCLUSION_STATUS_ON_EDGE
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__determine_occlusion_status, f360_sensor_occlusion_info__point_is_on_edge_of_visibility_returns_occlusion_status_on_edge)
{
   /** \precond
    * Set up position of point to make it be on edge of visibility
    */
   vcs_lat_posn = -0.3F;
   vcs_long_posn = 1.96F;

   /** \action
    * Call tested method
    */
   const Occlusion_Status_T occl_status = sensor_occlusion_info.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check wheter occl_status is equal to OCCLUSION_STATUS_ON_EDGE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_ON_EDGE, occl_status);
}

/** \purpose
 * Purpose of this test is to verify whether when point is occluded - function returns OCCLUSION_STATUS_OCCLUDED
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__determine_occlusion_status, f360_sensor_occlusion_info__point_is_occluded_returns_occlusion_status_occluded)
{
   /** \precond
    * Set up position of point to make it be occluded
    */
   vcs_lat_posn = -0.3F;
   vcs_long_posn = 3.0F;

   /** \action
    * Call tested method
    */
   const Occlusion_Status_T occl_status = sensor_occlusion_info.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check wheter occl_status is equal to OCCLUSION_STATUS_OCCLUDED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_OCCLUDED, occl_status);
}

/** \purpose
 * Purpose of this test is to verify whether when point is not in sensor FOV - function returns OCCLUSION_STATUS_UNDEFINED
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__determine_occlusion_status, f360_sensor_occlusion_info__point_not_in_sensor_fov)
{
   /** \precond
    * Set up position of point to make it be outside sensor FOV
    */
   vcs_lat_posn = -100.0F;
   vcs_long_posn = 3.0F;

   /** \action
    * Call tested method
    */
   const Occlusion_Status_T occl_status = sensor_occlusion_info.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check wheter occl_status is equal to OCCLUSION_STATUS_UNDEFINED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occl_status);
}

/** \purpose
 * Purpose of this test is to verify whether when sensor is not valid - function returns OCCLUSION_STATUS_UNDEFINED
 * \req
 * NA.
 */
TEST(f360_sensor_occlusion_info__determine_occlusion_status, f360_sensor_occlusion_info__sensor_not_valid)
{
   /** \precond
    * Set up position of point to make it be inside sensor FOV
    * Call reset method
    * Set up sensor is_valid flag as false
    * Call Update_Calibs
    */
   vcs_lat_posn = 2.0F;
   vcs_long_posn = 3.0F;
   sensor_occlusion_info.Reset();
   sensor.variable.is_valid = false;
   sensor_occlusion_info.Update_Calibs(sensor, range_uncertainty);

   /** \action
    * Call tested method
    */
   const Occlusion_Status_T occl_status = sensor_occlusion_info.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check wheter occl_status is equal to OCCLUSION_STATUS_UNDEFINED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occl_status);
}
/** @}*/
