
#ifndef _RUBY4_COM_H_
#define _RUBY4_COM_H_

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <stdint.h>
#include <string.h>


#include "ruby4.h"
#include "lidar_base.h"




class Ruby4Com : public LidarBase
{
public:
    Ruby4Com();
    ~Ruby4Com();

    bool ParseRecvTcpData(std::vector<char> & data) override;


    bool writeRegData(const std::vector<uint32_t>& reg_addr,
                      const std::vector<int32_t>& reg_val,
                      std::vector<char> & send_data,
                      uint8_t & statusIdx ) override;

    bool writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val,
                    std::vector<char> & send_data, uint8_t & statusIdx ) override;

    bool writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                   std::vector<char> & send_data,uint8_t & statusIdx) override;

    bool getReadRegSendData(const std::vector<uint32_t>& reg_addr, std::vector<int32_t>& reg_val,
                            std::vector<char> & send_data, uint8_t & statusIdx) override;
    bool getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val) override;
        
    //获取发送反射率命令返回的结果
    bool getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal) override;

    bool readRegData(const uint32_t start_reg_addr,
                     const uint32_t reg_number,
                     std::vector<int32_t>& reg_val,
                     std::vector<char> & send_data) override;

    bool getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx) override;
    bool getConfigResult(ConfigPara &para) override;

    bool getPatchVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) ;
	bool getPatchVerResult(u32 &ver) ;
	
	bool getConfigVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) ;
	bool getConfigVerResult(char ver[32]) ;

    
    bool SetLidarSn(u8 sn[6], std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetLidarNetInfo(NetParam_st netInfo, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetLidarSyncMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetLidarWaveMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetLidarFov(u16 start, u16 end, std::vector<char> & send_data, uint8_t &statusIdx) override;

    bool  SetMotorPhaseLockCfg(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetMotorSpeed(u8 level, std::vector<char> & send_data, uint8_t &statusIdx) override; // 1 ---300; 2---600; 3---1200
    //int SetLidarTrigerAngle(u16 angle);
    bool  SetLidarWorkMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx) override; // 0  standby, 1 normal
    bool  CtrlCodeWheelCalibrate(u8 ctrl, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  WriteTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  ReadTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx) override;
    bool  SetZeroAngle(s32 angle, std::vector<char> & send_data, uint8_t &statusIdx) ;
    bool  GetZeroAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx);
    bool  GetZeroAngleResult(s32 &angle); 

    bool  SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles);
    bool  GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum);
    bool  GetChannelAngleResult(std::vector<float>& angles);

    bool  SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx); // 

    bool queryUpdateStatus(std::vector<char> & send_data, uint8_t & statusIdx );
	bool getUpdateStatus(u8 &type, s32 &status);
	
    bool sendUpdateRequest(u8 type, uint32_t len, std::vector<char> & send_data, uint8_t & statusIdx );
    bool sendUpdateData(u8 type, std::vector<char> & file_data, std::vector<char> & send_data, uint8_t & statusIdx );
    bool sendUpdateCheckSum(u8 type, uint32_t checkSum, std::vector<char> & send_data, uint8_t & statusIdx );

    bool setMode(u8 mode, std::vector<char> & send_data, uint8_t & statusIdx);
	bool rebootLidar(std::vector<char> & send_data, uint8_t & statusIdx);
	bool getMode( std::vector<char> & send_data, uint8_t & statusIdx);
	bool getModeResult(uint8_t &mode);

   	bool readEventLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool deleteEventLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool readMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool restartMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool getEventLogData(std::vector<char> & recv_data);
	bool getMonitorLogData(std::vector<char> & recv_data);

	bool readDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool deleteDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx);
	bool getDaemonData(std::vector<char> & recv_data);
	
   	bool readConfigFile(std::vector<char> & send_data, uint8_t &statusIdx);
	bool getConfigFileData(std::vector<char> & recv_data);

    
private:
    bool judeRegAddr(uint32_t start_reg_addr, uint32_t reg_number, uint8_t &retCmdClass);
    s32 handleBotControlCmd(char *package, uint16_t dataLen);
	s32 handleOtherControlCmd(char *package, uint16_t dataLen);
    s32 handleTopControlCmd(char *package, uint16_t dataLen);
    s32 handleMotorControlCmd(char *package, uint16_t dataLen);
    s32 handleFirmwareUpdateCmd(char *package, uint16_t dataLen);
    s32 handleGetInfoCmd(char *package, uint16_t dataLen);
    s32 handleParamConfigCmd(char *package, uint16_t dataLen);
    bool judeRecvPackCrc(std::vector<char> recvData);


private:
    char reg_back_data_[2048];  // 包括DataLength, cmd, response + data
    char collect_back_data_[2048];
    char write_flash_back_data_[2048];

    uint32_t m_topRegLow = 0x1000;
    uint32_t m_topRegHi  = 0x4000;

    uint32_t m_botRegLow = 0x83c00000;
    uint32_t m_botRegHi  = 0x83c30000;

    const uint32_t m_minPackBytes = 11;

    u8  m_updateType = 0;  // 见 types.h   NET_CMD_TOP_BIN_UPDATE
    s32 m_updateStatus = 0005; // 最后一字节表示进度, 倒数第二字节表示是否出错。
    
};

   
#endif  //

