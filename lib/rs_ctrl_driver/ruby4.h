#ifndef _TYPES_RUBY4_H_
#define _TYPES_RUBY4_H_

#include "types.h"

#pragma pack(1)
typedef struct modules_upgrade_firmware_info
{
    uint8_t type;
    uint8_t action;
    uint8_t progress;
} __attribute__((packed)) ModulesUpgradeFirmwareInfo_st;
#pragma pack()



/***** Config information class define *****/
typedef enum
{
    PARAM_SETTING_CLASS = 0x10,
    PARAM_INFO_GET_CLASS = 0x20,
    FIRMWARE_UPDATE_CLASS = 0x30,
    MOTOR_CONTORL_CLASS = 0x40,
    TOP_CONTORL_CLASS = 0x50,
    BOT_CONTORL_CLASS = 0x60,
    OTHER_CTRL_CLASS = 0XE0,
} ConfigClass_et;

/***** Parameter setting class cmd define, CMD:0x10 *****/
typedef enum
{
    SET_SN_CMD = 0x01,
    SET_NET_CFG_CMD,
    SET_TIME_SYNC_MODE_CMD,
    SET_WAVE_MODE_CMD,
    SET_MOTOR_FOV_CMD,
    SET_PHASE_LOCKED_CMD,
    SET_MOTOR_SPEED_CMD,
    SET_TRIGGER_START_ANGLE,
    SET_WORK_MODE,
    SET_DE_RAIN = 0x0A,    
    SET_PS_CMD = 0X0C,
    SET_SUPPLY_PARAMS = 0x0D,
    SET_REFLECT_ENHANCE = 0X0E,
    SET_GPS_BAUD,
    SET_MOTOR_DIR,    
    SET_IMU_PARAM = 0x11,
    SET_TRAIL_FILTER_LEVERL,
    SET_POINT_CLOUD_MODE,
    SET_RAIN_BLOCK_DETECT_DISTANCE = 0X15,
    SET_RAIN_DETECT_SENSITIVITY,
    SET_BLOCK_DETECT_SENSITIVITY,	
    SET_ALL_CONFIG_CMD = 0xFF

} SetParamCmd_et;

/***** Get Parameter Information class cmd define, CMD:0x20 *****/
typedef enum
{
    GET_PARAM_INFO_CMD = 0x01,
    GET_CONFIG_VER_CMD = 0x04,
} GetParamCmd_et;

/***** Update class cmd define, CMD:0x30 *****/
typedef enum
{
    TOP_FIRMWARE_UPDATE_CMD = 0x01,
    STATUS_FIRMWARE_UPDATE_CMD = 0x02,
    PATCH_FIRMWARE_UPDATE_CMD  = 0x03,
    CONFIG_FIRMWARE_UPDATE_CMD = 0x04,
} FirmwareUpdateCmd_et;

/***** Motor Contorl Class cmd define, CMD:0x40 *****/
typedef enum
{
    MOTOR_DATA_SEND_CMD = 0x01,
    MOTOR_DATA_RECV_CMD,
    CODE_WHEEL_CALIBRATION_CMD,
    GET_MOTOR_PRARAM_CMD
} MotorControlCmd_et;

/***** Top Contorl Class cmd define, CMD:0x50 *****/
typedef enum
{
    READ_TOP_REG_CMD = 0x01,
    WRITE_TOP_REG_CMD,
    WRITE_TOP_FLASH_CMD,
    READ_TOP_FLASH_CMD,
    UPDATE_TOP_PARAM_FLASH_CMD,
    NET_ACK_EYES_SAFE_CMD,    //‰∫∫ÁúºÂ    CONTINUOUS_READ_TOP_REG_CMD,
    CONTINUOUS_WRITE_TOP_REG_CMD,
    TOP_GET_INTENSITY_CMD           //?çÂ∞Ñ?á?áÂÆö
} TopControlCmd_et;

/***** Bot Contorl Class cmd define, CMD:0x50 *****/
typedef enum
{
    READ_BOT_REG_CMD = 0x01,
    WRITE_BOT_REG_CMD,
    UPDATE_BOT_PARAM_FLASH_CMD,
    SET_ZERO_ANGLE_CMD,
    GET_ZERO_ANGLE_CMD,
    CONTINUOUS_READ_BOT_REG_CMD,
    CONTINUOUS_WRITE_BOT_REG_CMD
} BotControlCmd_et;

/***** Response Code *****/
typedef enum
{
    PROCESS_SUCCESS = 0x00,
    CMD_NOT_SUPPORT,
    PARAM_ERROR,
    DATA_LENGTH_ERROR,
    DATA_FORMAT_ERROR,
    CHECKSUM_FAILED,
    OTHER_ERROR,
    TIME_OUT
} ErrorCode_et;







#endif
