
#ifndef _LIDAR_BASE_H_
#define _LIDAR_BASE_H_

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <stdint.h>
#include "mems.h"
#include "types.h"


enum ReadWriteStateIndex
{
    STATE_TCP_START = 0,
    STATE_TCP_END,
    STATE_READ_REG,
    STATE_WRITE_REG,

    // ----- MEMS  used ---- 
    STATE_FIX_REG,
    STATE_READ_NETWORK_INFO,
    STATE_WRITE_NETWORK_INFO,
    STATE_READ_INTENSITY,
    STATE_READ_VIEWING_FIELD_PARAM,
    STATE_WRITE_SPLICE_PARAMS,
    STATE_READ_REG_2,
    STATE_WRITE_REG_2,    
    STATE_WRITE_FLASH,
    // ----- end mems

    STATE_WRITE_TOP_REG,
    STATE_READ_TOP_REG,
    STATE_GET_ALL_CONFIG,
    STATE_SET_ALL_CONFIG,
    STATE_SET_SN,
    STATE_EYE_SAFE_MODE,
    //STATE_SET_MAC,
    STATE_SET_NET,
    STATE_SET_SYNC_MODE,
    STATE_SET_WAVE_MODE,
    STATE_SET_FOV,
    STATE_SET_PHASE,
    STATE_MOTOR_SPEED,
    STATE_SET_WORK_MODE,
    STATE_GET_ZERO_ANGLE,    
    STATE_GET_CHANNEL_ANGLE,
    STATE_SET_ZERO_ANGLE, 
    STATE_SET_CHANNEL_ANGLE,    

    STATE_UPDATE_TOP,
    STATE_UPDATE_TOP_BAK,
    STATE_UPDATE_BOT,
    STATE_UPDATE_BOT_BAK,
    STATE_UPDATE_APP,
    STATE_UPDATE_APP_BAK,
    STATE_UPDATE_MOTOR,
    STATE_UPDATE_CGI,

    STATE_UPDATE_REQUEST,
    STATE_UPDATE_SEND_DATA,
    STATE_UPDATE_CHECKSUM,

	STATE_SET_MODE,
    STATE_GET_MODE,

    STATE_GET_IMU,
    STATE_SET_IMU,

    STATE_SET_MOTOR_DIR,
    STATE_GET_MOTOR_DIR,

    STATE_PATCH_CONFIG_VER,
    STATE_GET_CONFIG_VER,

    STATE_REBOOT_LIDAR,
    
    STATE_DELETE_EVENT_LOG,
    STATE_RESTART_MONITOR_LOG,
    STATE_READ_EVENT_LOG,
    STATE_READ_MONITOR_LOG,
    STATE_READ_DAEMON_LOG,
    STATE_DELETE_DAEMON_LOG,

    STATE_READ_CONFIG_FILE,

    STATE_GET_SUPPLY_PARAMS,
    STATE_SET_SUPPLY_PARAMS,
    STATE_REFLECT_ENHANCE,
    STATE_SET_GPS_BAUD,
    STATE_TRAIL_FILTER_LEVERL,
    STATE_RAIN_BLOCK_DETECT_DISTANCE,
    STATE_RAIN_DETECT_SENSITIVITY,
    STATE_BLOCK_DETECT_SENSITIVITY,
    STATE_FIX_TOP_REG,
    STATE_FIX_BOT_REG,
    STATE_FIX_459_REG,

    STATE_GET_MONITOR,
    STATE_SET_MOTOR_STOP_ANGLE,
    
    STATE_TOTAL_NUM  //　must be the last element of this enum
};



class LidarBase
{
public:
    LidarBase();
    ~LidarBase();

    virtual bool writeRegData(const std::vector<uint32_t>& reg_addr,
                      const std::vector<int32_t>& reg_val,
                      std::vector<char> & send_data, uint8_t & statusIdx );
 
    virtual bool writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val, 
                                    std::vector<char> & send_data, uint8_t & statusIdx );

    virtual bool writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                           std::vector<char> & send_data,uint8_t & statusIdx);

    virtual bool getReadRegSendData(const std::vector<uint32_t>& reg_addr,
                          std::vector<int32_t>& reg_val,
                          std::vector<char> & send_data, uint8_t & statusIdx);
                          
    virtual bool getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val);

    //获取发送反射率命令返回的结果
    virtual bool getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal);

	
	virtual bool getConfigVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) ;
	virtual bool getConfigVerResult(char ver[32]) ;
	virtual bool getPatchVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) ;
	virtual bool getPatchVerResult(u32 &ver) ;	

    virtual bool getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx) ;
    virtual bool getConfigResult(ConfigPara &para) ;
    virtual bool setConfigSendData(ConfigPara para, std::vector<char> & send_data, uint8_t & statusIdx) ;
    
    virtual bool readRegData(const uint32_t start_reg_addr,
                     const uint32_t reg_number,
                     std::vector<int32_t>& reg_val,
                     std::vector<char> & send_data);


    virtual bool ParseRecvTcpData(std::vector<char> & data);


    virtual bool getIntensityData(const std::vector<uint32_t>& addrs2read,
                          std::vector<int32_t>& calib_data,
                          std::vector<char> & send_data) ;

    virtual bool checkRecvCmd(ReadWriteStateIndex index);
    virtual bool readViewingFieldParams(std::vector<double>& param) ;

    virtual bool writeViewingFieldParams(const std::string& csv_path) ;

    virtual bool writeViewingFieldParams(const std::vector<double>& param) ;

    virtual bool setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec = 1000) ;

    virtual void getNetworkInfo(NetworkInfo& network_info) const ;

    virtual bool fixRegister(const uint32_t msec = 1000) ;

    virtual bool writeToFlash(const std::string& file_path, std::string &msg, const int update_type = 0) ;

    void writeErrLog(char *msg);    
    void writeDbgLog(char *msg);
    void ctrlLog(uint8_t level = 3);

    virtual bool update(const u8 type, std::vector<char> file_data, 
                                    std::vector<char> & send_data, uint8_t & statusIdx );


	virtual bool getUpdateStatus(u8 &type, s32 &status);  
    virtual bool sendUpdateRequest(u8 type, uint32_t len, std::vector<char> & send_data, uint8_t & statusIdx );
    virtual bool sendUpdateData(u8 type, std::vector<char> & file_data, std::vector<char> & send_data, uint8_t & statusIdx );
    virtual bool sendUpdateCheckSum(u8 type, uint32_t checkSum, std::vector<char> & send_data, uint8_t & statusIdx );
	virtual bool queryUpdateStatus(std::vector<char> & send_data, uint8_t & statusIdx );
	virtual bool rebootLidar(std::vector<char> & send_data, uint8_t & statusIdx);

	virtual bool setMode(u8 mode, std::vector<char> & send_data, uint8_t & statusIdx);
	virtual bool getMode( std::vector<char> & send_data, uint8_t & statusIdx);
	virtual bool getModeResult(uint8_t &mode);

	virtual bool setMotorDir(u8 dir, std::vector<char> & send_data, uint8_t & statusIdx);
	virtual bool getMotorDir( std::vector<char> & send_data, uint8_t & statusIdx);
	virtual bool getMotorResult(uint8_t &mode);

    // -----------------
    virtual bool  SetLidarSn(u8 sn[6], std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetLidarNetInfo(NetParam_st netInfo, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetLidarSyncMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetLidarWaveMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetLidarFov(u16 start, u16 end, std::vector<char> & send_data, uint8_t &statusIdx);

	virtual bool  SetMotorPhaseLockCfg(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetMotorSpeed(u8 level, std::vector<char> & send_data, uint8_t &statusIdx); // 1 ---300; 2---600; 3---1200

    virtual bool  SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx); // 
	

    //int SetLidarTrigerAngle(u16 angle);
    virtual bool  SetLidarWorkMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx); // 0  standby, 1 normal
    virtual bool  CtrlCodeWheelCalibrate(u8 ctrl, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  WriteTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  ReadTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  SetZeroAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx);

    virtual bool  GetZeroAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx);
    virtual bool  GetZeroAngleResult(s32 &angle);

    virtual bool  GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum);
    virtual bool  GetChannelAngleResult(std::vector<float>& angles);

    virtual bool  SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles);

   	virtual bool readEventLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool deleteEventLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool readMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool restartMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getEventLogData(std::vector<char> & recv_data);
	virtual bool getMonitorLogData(std::vector<char> & recv_data);

	virtual bool readDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool deleteDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getDaemonData(std::vector<char> & recv_data);
	
   	virtual bool readConfigFile(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getConfigFileData(std::vector<char> & recv_data);


	// airy
	virtual bool getImuParamsSendData(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getImuParamsResult(ImuParam &getParams);
	virtual bool setImuParamsSendData(ImuParam getParams, std::vector<char> & send_data, uint8_t &statusIdx);
	
	virtual bool getSupplementParamsSendData(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getSupplementParamsResult(NetInfo2 &getParams);
	virtual bool setSomeSupplementParamsSendData(NetInfoSet setParams, std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool setReflectEnhanceSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx); // 0:off	 1:on1	2:on2		  3:on3
	virtual bool setGpsBaudSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx); 

	virtual bool setTrailFilterSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx);	// [1, 7]
	virtual bool setRainBlockDetectDistanceSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx); //[0,3]	0: 30cm   1: 20cm	2: 10cm  
	virtual bool setRainDetectSensitivitySendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx);			  // [0, 2]   0: high,	1: middle 2:low 
	virtual bool setBlockDetectSensitivitySendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx);				 // [0, 2]	 0: high,  1: middle 2:low 

	virtual bool setFixTopRegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool setFixBotRegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool setFix459RegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx);


	virtual bool getMonitorSendData(std::vector<char> & send_data, uint8_t &statusIdx);
	virtual bool getMonitorResult(float value[64]);


public:
    std::array<bool, STATE_TOTAL_NUM> read_write_state_;
    char m_msg[512];

private:
    bool readNetworkInfo();
    void writeLog(char *msg, uint8_t level );


private:
    FILE *m_fp = NULL;
    uint8_t m_logLevel = 3;


};

   
#endif 

