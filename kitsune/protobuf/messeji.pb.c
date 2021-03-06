/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.1 at Wed Oct 19 14:20:19 2016. */

#include "messeji.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t ReceiveMessageRequest_fields[3] = {
    PB_FIELD(  1, STRING  , REQUIRED, CALLBACK, FIRST, ReceiveMessageRequest, sense_id, sense_id, 0),
    PB_FIELD(  2, INT64   , REPEATED, CALLBACK, OTHER, ReceiveMessageRequest, message_read_id, sense_id, 0),
    PB_LAST_FIELD
};

const pb_field_t Message_fields[9] = {
    PB_FIELD(  1, STRING  , OPTIONAL, CALLBACK, FIRST, Message, sender_id, sender_id, 0),
    PB_FIELD(  2, INT64   , REQUIRED, STATIC  , OTHER, Message, order, sender_id, 0),
    PB_FIELD(  3, INT64   , OPTIONAL, STATIC  , OTHER, Message, message_id, order, 0),
    PB_FIELD(  4, ENUM    , REQUIRED, STATIC  , OTHER, Message, type, message_id, 0),
    PB_FIELD(  5, MESSAGE , OPTIONAL, STATIC  , OTHER, Message, play_audio, type, &PlayAudio_fields),
    PB_FIELD(  6, MESSAGE , OPTIONAL, STATIC  , OTHER, Message, stop_audio, play_audio, &StopAudio_fields),
    PB_FIELD(  7, MESSAGE , OPTIONAL, STATIC  , OTHER, Message, volume, stop_audio, &Volume_fields),
    PB_FIELD(  8, MESSAGE , OPTIONAL, STATIC  , OTHER, Message, voice_control, volume, &VoiceControl_fields),
    PB_LAST_FIELD
};

const pb_field_t BatchMessage_fields[2] = {
    PB_FIELD(  1, MESSAGE , REPEATED, CALLBACK, FIRST, BatchMessage, message, message, &Message_fields),
    PB_LAST_FIELD
};

const pb_field_t MessageStatus_fields[3] = {
    PB_FIELD(  1, INT64   , REQUIRED, STATIC  , FIRST, MessageStatus, message_id, message_id, 0),
    PB_FIELD(  2, ENUM    , REQUIRED, STATIC  , OTHER, MessageStatus, state, message_id, 0),
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
PB_STATIC_ASSERT((pb_membersize(Message, play_audio) < 65536 && pb_membersize(Message, stop_audio) < 65536 && pb_membersize(Message, volume) < 65536 && pb_membersize(Message, voice_control) < 65536 && pb_membersize(BatchMessage, message) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_ReceiveMessageRequest_Message_BatchMessage_MessageStatus)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Message, play_audio) < 256 && pb_membersize(Message, stop_audio) < 256 && pb_membersize(Message, volume) < 256 && pb_membersize(Message, voice_control) < 256 && pb_membersize(BatchMessage, message) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_ReceiveMessageRequest_Message_BatchMessage_MessageStatus)
#endif


