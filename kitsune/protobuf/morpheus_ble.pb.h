/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.1 at Thu Nov  6 11:48:01 2014. */

#ifndef PB_MORPHEUS_BLE_PB_H_INCLUDED
#define PB_MORPHEUS_BLE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _ErrorType {
    ErrorType_TIME_OUT = 0,
    ErrorType_NETWORK_ERROR = 1,
    ErrorType_DEVICE_ALREADY_PAIRED = 2,
    ErrorType_INTERNAL_DATA_ERROR = 3,
    ErrorType_DEVICE_DATABASE_FULL = 4,
    ErrorType_DEVICE_NO_MEMORY = 5,
    ErrorType_INTERNAL_OPERATION_FAILED = 6,
    ErrorType_NO_ENDPOINT_IN_RANGE = 7,
    ErrorType_WLAN_CONNECTION_ERROR = 8,
    ErrorType_FAIL_TO_OBTAIN_IP = 9
} ErrorType;

typedef enum _wifi_endpoint_sec_type {
    wifi_endpoint_sec_type_SL_SCAN_SEC_TYPE_OPEN = 0,
    wifi_endpoint_sec_type_SL_SCAN_SEC_TYPE_WEP = 1,
    wifi_endpoint_sec_type_SL_SCAN_SEC_TYPE_WPA = 2,
    wifi_endpoint_sec_type_SL_SCAN_SEC_TYPE_WPA2 = 3
} wifi_endpoint_sec_type;

typedef enum _MorpheusCommand_CommandType {
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_SET_TIME = 0,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_GET_TIME = 1,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_SET_WIFI_ENDPOINT = 2,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_GET_WIFI_ENDPOINT = 3,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_SET_ALARMS = 4,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_GET_ALARMS = 5,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_SWITCH_TO_PAIRING_MODE = 6,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_SWITCH_TO_NORMAL_MODE = 7,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_START_WIFISCAN = 8,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_STOP_WIFISCAN = 9,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_GET_DEVICE_ID = 10,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_EREASE_PAIRED_PHONE = 11,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_PAIR_PILL = 12,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_ERROR = 13,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_PAIR_SENSE = 14,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_UNPAIR_PILL = 15,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_MORPHEUS_DFU_BEGIN = 16,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_PILL_DATA = 17,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_PILL_HEARTBEAT = 18,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_PILL_DFU_BEGIN = 19,
    MorpheusCommand_CommandType_MORPHEUS_COMMAND_FACTORY_RESET = 20
} MorpheusCommand_CommandType;

/* Struct definitions */
typedef struct _BatchedPillData {
    pb_callback_t pills;
} BatchedPillData;

typedef PB_BYTES_ARRAY_T(20) MorpheusCommand_PillData_motionDataEntrypted_t;

typedef struct _MorpheusCommand_PillData {
    bool has_deviceId;
    char deviceId[17];
    bool has_batteryLevel;
    int32_t batteryLevel;
    bool has_uptime;
    int32_t uptime;
    bool has_motionDataEntrypted;
    MorpheusCommand_PillData_motionDataEntrypted_t motionDataEntrypted;
    bool has_firmwareVersion;
    int32_t firmwareVersion;
} MorpheusCommand_PillData;

typedef struct _wifi_endpoint {
    pb_callback_t ssid;
    pb_callback_t bssid;
    int32_t rssi;
    wifi_endpoint_sec_type security_type;
} wifi_endpoint;

typedef struct _MorpheusCommand {
    int32_t version;
    MorpheusCommand_CommandType type;
    pb_callback_t deviceId;
    pb_callback_t accountId;
    bool has_error;
    ErrorType error;
    pb_callback_t wifiName;
    pb_callback_t wifiSSID;
    pb_callback_t wifiPassword;
    bool has_batteryLevel;
    int32_t batteryLevel;
    bool has_uptime;
    int32_t uptime;
    bool has_motionData;
    int32_t motionData;
    pb_callback_t motionDataEntrypted;
    bool has_firmwareVersion;
    int32_t firmwareVersion;
    pb_callback_t wifi_scan_result;
    bool has_security_type;
    wifi_endpoint_sec_type security_type;
    bool has_pillData;
    MorpheusCommand_PillData pillData;
} MorpheusCommand;

/* Default values for struct fields */

/* Initializer values for message structs */
#define wifi_endpoint_init_default               {{{NULL}, NULL}, {{NULL}, NULL}, 0, (wifi_endpoint_sec_type)0}
#define MorpheusCommand_init_default             {0, (MorpheusCommand_CommandType)0, {{NULL}, NULL}, {{NULL}, NULL}, false, (ErrorType)0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, false, 0, false, 0, false, 0, {{NULL}, NULL}, false, 0, {{NULL}, NULL}, false, (wifi_endpoint_sec_type)0, false, MorpheusCommand_PillData_init_default}
#define MorpheusCommand_PillData_init_default    {false, "", false, 0, false, 0, false, {0, {0}}, false, 0}
#define BatchedPillData_init_default             {{{NULL}, NULL}}
#define wifi_endpoint_init_zero                  {{{NULL}, NULL}, {{NULL}, NULL}, 0, (wifi_endpoint_sec_type)0}
#define MorpheusCommand_init_zero                {0, (MorpheusCommand_CommandType)0, {{NULL}, NULL}, {{NULL}, NULL}, false, (ErrorType)0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, false, 0, false, 0, false, 0, {{NULL}, NULL}, false, 0, {{NULL}, NULL}, false, (wifi_endpoint_sec_type)0, false, MorpheusCommand_PillData_init_zero}
#define MorpheusCommand_PillData_init_zero       {false, "", false, 0, false, 0, false, {0, {0}}, false, 0}
#define BatchedPillData_init_zero                {{{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define BatchedPillData_pills_tag                1
#define MorpheusCommand_PillData_deviceId_tag    1
#define MorpheusCommand_PillData_batteryLevel_tag 2
#define MorpheusCommand_PillData_uptime_tag      3
#define MorpheusCommand_PillData_motionDataEntrypted_tag 4
#define MorpheusCommand_PillData_firmwareVersion_tag 5
#define wifi_endpoint_ssid_tag                   1
#define wifi_endpoint_bssid_tag                  2
#define wifi_endpoint_rssi_tag                   4
#define wifi_endpoint_security_type_tag          5
#define MorpheusCommand_version_tag              1
#define MorpheusCommand_type_tag                 2
#define MorpheusCommand_deviceId_tag             3
#define MorpheusCommand_accountId_tag            4
#define MorpheusCommand_error_tag                5
#define MorpheusCommand_wifiName_tag             6
#define MorpheusCommand_wifiSSID_tag             7
#define MorpheusCommand_wifiPassword_tag         8
#define MorpheusCommand_batteryLevel_tag         9
#define MorpheusCommand_uptime_tag               10
#define MorpheusCommand_motionData_tag           11
#define MorpheusCommand_motionDataEntrypted_tag  12
#define MorpheusCommand_firmwareVersion_tag      13
#define MorpheusCommand_wifi_scan_result_tag     14
#define MorpheusCommand_security_type_tag        15
#define MorpheusCommand_pillData_tag             16

/* Struct field encoding specification for nanopb */
extern const pb_field_t wifi_endpoint_fields[5];
extern const pb_field_t MorpheusCommand_fields[17];
extern const pb_field_t MorpheusCommand_PillData_fields[6];
extern const pb_field_t BatchedPillData_fields[2];

/* Maximum encoded size of messages (where known) */
#define MorpheusCommand_PillData_size            74

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
