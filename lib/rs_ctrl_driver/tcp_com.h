
#ifndef _TCP_COM_H_
#define _TCP_COM_H_

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <stdint.h>
#include <boost/asio.hpp>

#include "types.h"
#include "mems.h"
#include "lidar_base.h"
#include "mems_com.h"
#include "ruby4_com.h"
#include "helios_com.h"
#include "airy_com.h"


class TcpCom
{
public:
    explicit TcpCom();
    explicit TcpCom(TcpCom&&)      = delete;
    explicit TcpCom(const TcpCom&) = delete;
    TcpCom& operator=(TcpCom&&) = delete;
    TcpCom& operator=(const TcpCom&) = delete;
    ~TcpCom();

    bool connect(const std::string& ip, const uint16_t msop_port, const uint32_t sec = 5);
    bool disconnect();
    bool isConnected() const;
    bool sendData(const std::vector<char>& send_data, const ReadWriteStateIndex index, uint32_t msec = 5000);

    int SetLidarType(LidarType type);
 
    bool writeRegData(const std::vector<uint32_t>& reg_addr,
                      const std::vector<int32_t>& reg_val,
                      const uint32_t msec = 300);
   
    bool writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val, const uint32_t msec = 300);

    bool readRegData(const std::vector<uint32_t>& reg_addr, std::vector<int32_t>& reg_val, const uint32_t msec = 300);

    bool readRegData(const uint32_t start_reg_addr,
                     const uint32_t reg_number,
                     std::vector<int32_t>& reg_val,
                     const uint32_t msec = 300);

    bool writeCalibrateReflectData(std::vector<uint32_t>& addrs2read,std::vector<uint32_t>& reg_val,
                                   std::vector<uint32_t>& vecRetRegVal, const uint32_t msec);

    bool GetLidarConfig(ConfigPara &para);
    bool SetLidarConfig(ConfigPara para);

	bool GetPatchVersion(u32 &ver);
    bool GetConfigVersion(char ver[32]);              

    bool getIntensityData(const std::vector<uint32_t>& addrs2read,
                          std::vector<int32_t>& calib_data,
                          const uint32_t msec = 300) ;


    bool readViewingFieldParams(std::vector<double>& param) ;

    bool writeViewingFieldParams(const std::string& csv_path) ;

    bool writeViewingFieldParams(const std::vector<double>& param) ;

    bool setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec = 1000) ;

    void getNetworkInfo(NetworkInfo& network_info) const ;

    bool fixRegister(const uint32_t msec = 1000) ;

    bool writeToFlash(const std::string& file_path, std::string &msg, const int update_type = 0) ;

	bool update(u8 type, std::vector<char> file_data);
	bool getUpdateStatus(u8 &type, s32 &status);
	bool fixRegsCfg(char *fileName);


	// 0  sleep mode, low power consumption mode,  1 normal mode
	bool setMode(u8 mode);
	bool getMode(u8 &mode);
	bool rebootLidar();
	void rebootNoLink(char *ethName);

	bool setMotorDir(u8 dir);
	bool getMotorDir(u8 &dir);

    // ---------- ruby4 只能单独设置 每个雷达的参数
    int SetLidarSn(u8 sn[6]);
    int SetLidarNetInfo(NetParam_st netInfo);
    int SetLidarSyncMode(u8 mode);
    int SetLidarWaveMode(u8 mode);
    int SetLidarFov(u16 start, u16 end);

    int SetMotorPhaseLockCfg(u16 angle);
    int SetMotorSpeed(u8 level); // 1 ---300; 2---600; 3---1200
    //int SetLidarTrigerAngle(u16 angle);
    
    int SetLidarWorkMode(u8 mode); // 0  standby, 1 normal
    int CtrlCodeWheelCalibrate(u8 ctrl);
    int WriteTopFlash(u32 startAddr, char *file);
    int ReadTopFlash(u32 startAddr, char *file);
    int CtrlEyeSafe(u16 on);
    bool SetZeroAngle(s32 angle);
    bool GetZeroAngle(s32 &angle);    
    bool GetChannelAngle(std::vector<float>& angles, u16 inAngleNum);
    bool SetChannelAngle(std::vector<float>  angles);

	bool readConfigFile(std::vector<char> & data);
   	bool readEventLog(std::vector<char> & data);
	bool deleteEventLog();
	bool readMonitorLog(std::vector<char> & data);
	bool restartMonitorLog();
	bool readDaemonLog(std::vector<char> & data);
	bool deleteDaemonLog();


	// airy 专用的
	bool setImuParams(ImuParam setParams);
	bool getImuParams(ImuParam &getParams);
	
	bool getSupplementParams(NetInfo2 &getParams);
	bool setSomeSupplementParams(NetInfoSet setParams);
	bool setReflectEnhance(u8 level); // 0:off	 1:on1	2:on2		  3:on3
	bool setGpsBaud(u8 level); 

	bool setTrailFilter(u8 level);	// [1, 7]
	bool setRainBlockDetectDistance(u8 level); //[0,3]	0: 30cm   1: 20cm	2: 10cm  
	bool setRainDetectSensitivity(u8 level);			  // [0, 2]   0: high,	1: middle 2:low 
	bool setBlockDetectSensitivity(u8 level);				 // [0, 2]	 0: high,  1: middle 2:low 

	bool getMonitorValue(float value[64]);
    bool SetMotorStopAngle(u16 angle);
	

public:
    std::vector<char> read_data_buffer_;
    std::array<bool, STATE_TOTAL_NUM> read_write_state_;


private:

    void slotReadTCPData();
    bool readNetworkInfo();
    void checkDeadline();
    uint32_t hexStrToUInt(const char *str) ;

private:
    std::shared_ptr<LidarBase> m_usedCom = nullptr;

    std::unique_ptr<boost::asio::ip::tcp::socket> ptr_socket_;
    std::unique_ptr<boost::asio::deadline_timer> ptr_deadline_timer_;
    std::unique_ptr<boost::asio::io_service> ptr_io_service_;

    std::shared_ptr<std::thread> ptr_thread_;
    std::atomic<bool> flag_thread_run_;
    std::string ip_;
    uint16_t msop_port_;

    char reg_back_data_[4096];
    char collect_back_data_[4096];
    char write_flash_back_data_[256];

    bool m_netOK = false;
    LidarType m_lidarType = RS_HELIOS;

    FILE *m_fp;


};

   
#endif  // _TCP_COM_H_

