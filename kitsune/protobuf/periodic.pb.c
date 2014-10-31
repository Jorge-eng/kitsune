/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 at Tue Oct 28 16:47:22 2014. */

#include "periodic.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t periodic_data_fields[16] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, periodic_data, unix_time, unix_time, 0),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light, unix_time, 0),
    PB_FIELD(  3, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, temperature, light, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, humidity, temperature, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust, humidity, 0),
    PB_FIELD(  6, STRING  , OPTIONAL, CALLBACK, OTHER, periodic_data, name, dust, 0),
    PB_FIELD(  7, BYTES   , OPTIONAL, CALLBACK, OTHER, periodic_data, mac, name, 0),
    PB_FIELD(  8, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light_variability, mac, 0),
    PB_FIELD(  9, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, light_tonality, light_variability, 0),
    PB_FIELD( 10, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, firmware_version, light_tonality, 0),
    PB_FIELD( 11, MESSAGE , REPEATED, CALLBACK, OTHER, periodic_data, pills, firmware_version, &periodic_data_pill_data_fields),
    PB_FIELD( 12, STRING  , OPTIONAL, CALLBACK, OTHER, periodic_data, device_id, pills, 0),
    PB_FIELD( 13, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_variability, device_id, 0),
    PB_FIELD( 14, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_max, dust_variability, 0),
    PB_FIELD( 15, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data, dust_min, dust_max, 0),
    PB_LAST_FIELD
};

const pb_field_t periodic_data_pill_data_fields[7] = {
    PB_FIELD(  1, INT32   , OPTIONAL, STATIC  , FIRST, periodic_data_pill_data, firmwareVersion, firmwareVersion, 0),
    PB_FIELD(  2, BYTES   , OPTIONAL, CALLBACK, OTHER, periodic_data_pill_data, motionDataEncrypted, firmwareVersion, 0),
    PB_FIELD(  3, STRING  , OPTIONAL, CALLBACK, OTHER, periodic_data_pill_data, deviceId, motionDataEncrypted, 0),
    PB_FIELD(  9, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_pill_data, batteryLevel, deviceId, 0),
    PB_FIELD( 10, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_pill_data, uptime, batteryLevel, 0),
    PB_FIELD( 11, INT32   , OPTIONAL, STATIC  , OTHER, periodic_data_pill_data, motionData, uptime, 0),
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
PB_STATIC_ASSERT((pb_membersize(periodic_data, pills) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_periodic_data_periodic_data_pill_data)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(periodic_data, pills) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_periodic_data_periodic_data_pill_data)
#endif


