/********************************************************************
 *
 *  File name: comm.h
 *
 *  Module:    Communications header for the G&H project.
 *
 *  Summary:   This header contains the definitions common to all
 *             processors within the G&H product.
 *
 *******************************************************************/

// Message Commands
enum commCmd {
    COMM_CMD_CLOSE_FET,          // command switch to close FET
    COMM_CMD_OPEN_FET,           // command switch to open FET
    COMM_CMD_ARM,                // command switch to arm detonator
    COMM_CMD_FIRE,               // command switch to fire detonator
    COMM_CMD_DISARM,             // command switch to disarm detonator
    COMM_CMD_SWITCH_READY,       // switch announces its presence on the string
    COMM_CMD_QUIET_SWITCH,       // command switch to stop sending ready message
    COMM_CMD_NEW_CRC,            // command switch to prep for a new CRC
    COMM_CMD_CLOSE_FET_GO_QUIET, // command switch to close FET and stop responding to all commands
    COMM_CMD_QUERY_SWITCH_VER,   // request switch to return software version number
    COMM_CMD_QUERY_LIFE,         // request number of switch firings since manufacture
    COMM_CMD_ARM_TOOL,           // command switch to arm setting tool
    COMM_CMD_FIRE_TOOL,          // command switch to fire setting tool
    COMM_CMD_DISARM_TOOL,        // command switch to disarm setting tool
    COMM_CMD_COMM_ERROR,         // control panel received serial message with an error
    COMM_CMD_NO_COMM,            // comm with the switches not possible
    COMM_CMD_QUERY_PANEL_VER,    // request panel to return software version number
    COMM_CMD_QUERY_STATUS,       // request panel status, turn relay on/off
    COMM_CMD_POWER_GUNSTRING,    // request power to gun string on/off
    COMM_CMD_ADC_MEAS,           // current and voltage measurements (request and data)
    COMM_CMD_GUNSTRING_ERROR,    // control panel detected serious fault in gunstring (powered off - test box only)
    COMM_CMD_PWRSUPPLY_V_PROFILE,// programmable power supply firing voltage profile
    COMM_CMD_PWRSUPPLY_C_PROFILE,// programmable power supply firing current profile
    COMM_CMD_END
};

// Payload Length Definitions
// Where payload lengths are different depending on who is transmitting,
// _RQST are lengths of messages transmitted by the PC or Test Box Display processor
// _RESP are lengths of messages transmitted by the Controller
#define PAYLOAD_LEN_GENERIC_SWITCH_MSG     1
#define PAYLOAD_LEN_CLOSE_FET              1
#define PAYLOAD_LEN_OPEN_FET               1
#define PAYLOAD_LEN_ARM                    1
#define PAYLOAD_LEN_FIRE                   1
#define PAYLOAD_LEN_DISARM                 1
#define PAYLOAD_LEN_SWITCH_READY           1
#define PAYLOAD_LEN_QUIET_SWITCH           1
#define PAYLOAD_LEN_NEW_CRC_RQST           2
#define PAYLOAD_LEN_NEW_CRC_RESP           1
#define PAYLOAD_LEN_CLOSE_FET_GO_QUIET     1
#define PAYLOAD_LEN_QUERY_SWITCH_VER_RQST  1
#define PAYLOAD_LEN_QUERY_SWITCH_VER_RESP  2
#define PAYLOAD_LEN_QUERY_LIFE_RQST        1
#define PAYLOAD_LEN_QUERY_LIFE_RESP        2
#define PAYLOAD_LEN_ARM_TOOL               1
#define PAYLOAD_LEN_FIRE_TOOL              1
#define PAYLOAD_LEN_DISARM_TOOL            1
#define PAYLOAD_LEN_COMM_ERROR             1
#define PAYLOAD_LEN_NO_COMM                0
#define PAYLOAD_LEN_QUERY_PANEL_VER_RQST   0
#define PAYLOAD_LEN_QUERY_PANEL_VER_RESP   3
#define PAYLOAD_LEN_QUERY_STATUS_RQST      0
#define PAYLOAD_LEN_QUERY_STATUS_RESP      2
#define PAYLOAD_LEN_POWER_GUNSTRING        1
#define PAYLOAD_LEN_ADC_MEAS_RQST          1
#define PAYLOAD_LEN_ADC_MEAS_RESP          4
#define PAYLOAD_LEN_GUNSTRING_ERROR        0
#define PAYLOAD_LEN_PWR_V_PROFILE_RQST     6
#define PAYLOAD_LEN_PWR_C_PROFILE_RQST     6
#define PAYLOAD_LEN_PWR_PROFILE_RESP       1

#define PAYLOAD_LEN_MAX                    6


// COMM_CMD_COMM_ERROR payload definitions
#define COMM_CMD_CHKSUM_ERROR              1
#define COMM_CMD_LEN_ERROR                 2

// COMM_CMD_POWER_GUNSTRING payload definitions
#define WIRELINE_PWR_DISABLE               0
#define WIRELINE_PWR_ENABLE                1

// COMM_CMD_PWRSUPPLY_PROFILE payload definitions
#define PROFILE_VOLTAGE                    1
#define PROFILE_CURRENT                    2


