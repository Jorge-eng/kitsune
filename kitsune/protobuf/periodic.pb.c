/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 at Mon Jan 23 14:16:44 2017. */

#include "periodic.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t periodic_data_fields[24] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, periodic_data, unix_time, unix_time, 0),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light, unix_time, 0),
    PB_FIELD(  3, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, temperature, light, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, humidity, temperature, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust, humidity, 0),
    PB_FIELD(  8, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light_variability, dust, 0),
    PB_FIELD(  9, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light_tonality, light_variability, 0),
    PB_FIELD( 13, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_variability, light_tonality, 0),
    PB_FIELD( 14, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_max, dust_variability, 0),
    PB_FIELD( 15, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_min, dust_max, 0),
    PB_FIELD( 16, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, wave_count, dust_min, 0),
    PB_FIELD( 17, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, hold_count, wave_count, 0),
    PB_FIELD( 18, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, audio_num_disturbances, hold_count, 0),
    PB_FIELD( 19, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, audio_peak_disturbance_energy_db, audio_num_disturbances, 0),
    PB_FIELD( 20, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, audio_peak_background_energy_db, audio_peak_disturbance_energy_db, 0),
    PB_FIELD( 21, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, audio_peak_energy_db, audio_peak_background_energy_db, 0),
    PB_FIELD( 22, UINT32  , OPTIONAL, STATIC  , OTHER, periodic_data, pressure, audio_peak_energy_db, 0),
    PB_FIELD( 23, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, tvoc, pressure, 0),
    PB_FIELD( 24, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, co2, tvoc, 0),
    PB_FIELD( 25, MESSAGE , OPTIONAL, STATIC  , OTHER, periodic_data, light_sensor, co2, &periodic_data_light_data_fields),
    PB_FIELD( 26, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light_duration_ms, light_sensor, 0),
    PB_FIELD( 27, MESSAGE , OPTIONAL, STATIC  , OTHER, periodic_data, keyword_stats, light_duration_ms, &KeywordStats_fields),
    PB_FIELD( 28, MESSAGE , OPTIONAL, STATIC  , OTHER, periodic_data, tvoc_fw_data, keyword_stats, &periodic_data_tvoc_fw_fields),
    PB_LAST_FIELD
};

const pb_field_t periodic_data_light_data_fields[8] = {
    PB_FIELD(  1, INT32   , REQUIRED, STATIC  , FIRST, periodic_data_light_data, r, r, 0),
    PB_FIELD(  2, INT32   , REQUIRED, STATIC  , OTHER, periodic_data_light_data, g, r, 0),
    PB_FIELD(  3, INT32   , REQUIRED, STATIC  , OTHER, periodic_data_light_data, b, g, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_light_data, infrared, b, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_light_data, clear, infrared, 0),
    PB_FIELD(  6, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_light_data, lux_count, clear, 0),
    PB_FIELD(  7, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_light_data, uv_count, lux_count, 0),
    PB_LAST_FIELD
};

const pb_field_t periodic_data_tvoc_fw_fields[5] = {
    PB_FIELD(  1, BOOL    , OPTIONAL, STATIC  , FIRST, periodic_data_tvoc_fw, update_tvoc_fw, update_tvoc_fw, 0),
    PB_FIELD(  2, UINT32  , OPTIONAL, STATIC  , OTHER, periodic_data_tvoc_fw, tvoc_app_fw_version, update_tvoc_fw, 0),
    PB_FIELD(  3, UINT32  , OPTIONAL, STATIC  , OTHER, periodic_data_tvoc_fw, tvoc_boot_fw_version, tvoc_app_fw_version, 0),
    PB_FIELD(  4, UINT32  , OPTIONAL, STATIC  , OTHER, periodic_data_tvoc_fw, tvoc_hw_version, tvoc_boot_fw_version, 0),
    PB_LAST_FIELD
};

const pb_field_t batched_periodic_data_fields[9] = {
    PB_FIELD(  1, MESSAGE , REPEATED, CALLBACK, FIRST, batched_periodic_data, data, data, &periodic_data_fields),
    PB_FIELD(  2, STRING  , REQUIRED, CALLBACK, OTHER, batched_periodic_data, device_id, data, 0),
    PB_FIELD(  3, INT32   , REQUIRED, STATIC  , OTHER, batched_periodic_data, firmware_version, device_id, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, batched_periodic_data, uptime_in_second, firmware_version, 0),
    PB_FIELD(  7, STRING  , OPTIONAL, STATIC  , OTHER, batched_periodic_data, connected_ssid, uptime_in_second, 0),
    PB_FIELD(  9, STRING  , OPTIONAL, STATIC  , OTHER, batched_periodic_data, ring_time_ack, connected_ssid, 0),
    PB_FIELD( 10, MESSAGE , REPEATED, CALLBACK, OTHER, batched_periodic_data, scan, ring_time_ack, &batched_periodic_data_wifi_access_point_fields),
    PB_FIELD( 11, INT32   , OPTIONAL, STATIC  , OTHER, batched_periodic_data, messages_in_queue, scan, 0),
    PB_LAST_FIELD
};

const pb_field_t batched_periodic_data_wifi_access_point_fields[4] = {
    PB_FIELD(  1, STRING  , OPTIONAL, STATIC  , FIRST, batched_periodic_data_wifi_access_point, ssid, ssid, 0),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, batched_periodic_data_wifi_access_point, rssi, ssid, 0),
    PB_FIELD(  3, ENUM    , OPTIONAL, STATIC  , OTHER, batched_periodic_data_wifi_access_point, antenna, rssi, 0),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(periodic_data, light_sensor) < 65536 && pb_membersize(periodic_data, keyword_stats) < 65536 && pb_membersize(periodic_data, tvoc_fw_data) < 65536 && pb_membersize(batched_periodic_data, data) < 65536 && pb_membersize(batched_periodic_data, scan) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_periodic_data_periodic_data_light_data_periodic_data_tvoc_fw_batched_periodic_data_batched_periodic_data_wifi_access_point)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(periodic_data, light_sensor) < 256 && pb_membersize(periodic_data, keyword_stats) < 256 && pb_membersize(periodic_data, tvoc_fw_data) < 256 && pb_membersize(batched_periodic_data, data) < 256 && pb_membersize(batched_periodic_data, scan) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_periodic_data_periodic_data_light_data_periodic_data_tvoc_fw_batched_periodic_data_batched_periodic_data_wifi_access_point)
#endif


