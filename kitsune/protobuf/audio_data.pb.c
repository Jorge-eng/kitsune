/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 at Thu Nov 13 16:25:55 2014. */

#include "audio_data.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t audio_data_fields[7] = {
    PB_FIELD(  1, BYTES   , OPTIONAL, CALLBACK, FIRST, audio_data, features, features, 0),
    PB_FIELD(  2, INT32   , OPTIONAL, STATIC  , OTHER, audio_data, feature_version, features, 0),
    PB_FIELD(  3, INT32   , OPTIONAL, STATIC  , OTHER, audio_data, samples, feature_version, 0),
    PB_FIELD(  4, INT32   , OPTIONAL, STATIC  , OTHER, audio_data, rate, samples, 0),
    PB_FIELD(  5, INT32   , OPTIONAL, STATIC  , OTHER, audio_data, audio_version, rate, 0),
    PB_FIELD(  6, BYTES   , OPTIONAL, CALLBACK, OTHER, audio_data, audio, audio_version, 0),
    PB_LAST_FIELD
};


