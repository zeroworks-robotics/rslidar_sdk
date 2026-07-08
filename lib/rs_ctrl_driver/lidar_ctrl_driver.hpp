/******************************************************************************
 * Copyright 2021 RoboSense All rights reserved.
 * Suteng Innovation Technology Co., Ltd. www.robosense.ai

 * This software is provided to you directly by RoboSense and might
 * only be used to access RoboSense LiDAR. Any compilation,
 * modification, exploration, reproduction and redistribution are
 * restricted without RoboSense's prior consent.

 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOSENSE BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#pragma once

#include <vector>

#include "ctrl_driver_param.hpp"
#include "types.h"


using namespace std;
using  std::vector;


namespace robosense
{
namespace lidar
{

/**
 * @brief This is the RoboSense LiDAR Control driver interface class
 */
class LidarCtrlDriver
{
public:

    /**
     * @brief Constructor
     */
    LidarCtrlDriver();    

    /**
     * @brief Destructor
     */
    ~LidarCtrlDriver();

    /**
     * @brief The initialization function, used to set up parameters and instance objects,
     *        used when get/set lidars
     * @param param The custom struct RSCtrlDriverParam
     * @return If successful, return true; else return false
     */
    bool init(RSCtrlDriverParam param);
	int  setLidarType(LidarType type);

    // ver[32]  
    void getLibVer(char *ver);

    bool isConnected() const;

    bool getVersions(VersionRst &ver);


    bool getPatchVersion(u32 &ver);

    bool getConfigVersion(char ver[32]);

    /**
     * @brief set work mode of the lidar
     * @param mode work mode
     *        0 low power consumption mode, 
     *        1 normal mode
     * @return If successful, return true; else return false
     */
    bool setMode(int mode);

    /**
     * @brief set work mode of the lidar
     * @param returned value of work mode
     *        0 low power consumption mode, 
     *        1 normal mode
     * @return If successful, return true; else return false
     */
    bool getMode(int* mode);
    bool rebootLidar();
    bool setIpPort(char *ip, char *mask, u16 port);


	bool getMonitorValue(float value[64]);
    bool getWaveMode(u8 &waveMode);
    bool getTimeSyncMode(u8 &timeMode);
    bool getPhaseStatus(u8 &status);  //  status : 1 为成功

    bool ctrlEyeSafe(u8 on); // default is 1 open eye safe; 0 is close eye safe

    bool openLaser();
    bool closeLaser();
    void rebootNoLink(char *ethName);

	/*
		type：
		    NET_CMD_TOP_BIN_UPDATE     3
			NET_CMD_TOPBACKUP_UPDATE   4    
			NET_CMD_BOT_BIN_UPDATE     5   
			NET_CMD_BOTBACKUP_UPDATE   6   
			NET_CMD_LINUX_APP_UPDATE   7		
			NET_CMD_LAPPBACKUP_UPDATE  8	
			NET_CMD_CGI_UPDATE	       12 		  
			NET_CMD_MOTOR_UPDATE       22

			NET_CMD_RUBY4_UPDATE            0x020
			NET_CMD_BP4_UPDATE              0x023
			NET_CMD_PATCH_UPDATE     0x024

			NET_CMD_CONFIG_UPDATE     0x033

	    fileName: need contains string as follow:
	        helios_top_xxxxxxx.bin
	        helios_topbackup_xxxxx.bin
	        helios_bot_xxxxx.bin	        
	        helios_botbackup_xxxxx.bin     
	        helios_app_xxxxx.elf   
	        helios_appbackup_xxxxx.elf	           
	        helios_cgi_xxxxx.tar.gz	           
	        helios_mot_xxxxx.hex
	        helios_config_xxxxxx.tar.gz
	        helios_patch_xxxxxx.tar.gz
	        

	        ruby4_system_upgrade.zip
	        bp4_system_upgrade.zip
	        xxx_config.tar.gz
			xxx_patch.tar.gz

			airy use 4 update files,  3,5,7, 0x51  file name need begin with airy
			NET_CMD_TOP_BIN_UPDATE,   "airy_xx_top_xxxxxxxx_sign.bin"
			NET_CMD_BOT_BIN_UPDATE,   "airy_b1_bot_fpga_xxxxxxxx_sign.bit"
			NET_CMD_LINUX_APP_UPDATE, "airy_app_final.release_ps_24101402_mot_24101221.appimage.hs_fs"
			NET_CMD_FIX_MACHINE_REGS  "machine_config.txt"

	*/
    bool update(u8 type, char *fileName);
    bool getUpdateStatus(u8 &type, s32 &status);

	bool setMotorDir(u8 dir);  // 0 正向， 1反向, just for helios and 0350
	bool getMotorDir(u8 &dir);

	bool getConfigParams(ConfigPara &params);
	
	// only 0320 helios can use setConfigParams() to set all params
	bool setConfigParams(ConfigPara params);


	// ruby4, airy should use lots of setParams functions as following

	//  helios            0x00: strongest; 0x01: last ; 0x02: first  0x03: dual 
	//  ruby4 and ariy B1 0x00: strongest; 0x01: first; 0x02: last   0x03: dual 
	bool setWaveMode(u8 waveMode);
	
	// 0x00：GPS同步    0x01：E2E L4同步    0x02：P2P L4同步    0x03：GPTP同步 0x04:  E2E L2同步
    bool setTimeSyncMode(u8 timeMode); 
    
    bool setLockPhase(u16 angle); // [0, 360]
    
    bool setLidarNetInfo(NetParam_st netInfo);
    bool setLidarFov(u16 start, u16 end);
    bool setMotorPhaseLockCfg(u16 angle);
    bool setMotorSpeed(u8 level); // 0 --0; 1 ---300; 2---600; 3---1200
    bool setLidarWorkMode(u8 mode); // 0  standby, 1 normal

	// --end of ruby4, airy should use lots of setParams functions as following

	// for airy funcs only
	bool setImuParams(ImuParam setParams);
	bool getImuParams(ImuParam &getParams);
	bool getSupplementParams(NetInfo2 &getParams, NetInfoSet &setParams);
	bool setSomeSupplementParams(NetInfoSet setParams);
	bool setReflectEnhance(u8 level); // 0:off   1:on1  2:on2         3:on3

/*   level    baud
	 0x03	  9600		  
	 0x04	 14400		  
	 0x05	 19200		  
	 0x06	 38400		  
	 0x07	 43200		  
	 0x08	 57600		  
	 0x09	 76800		  
	 0x0A	 115200 	  
	 0x0B	 128000 	  
	 0x0C	 230400 	  
	 0x0D	 256000 	  
	 0x0E	 460800 	  
	 0x0F	 921600 	  
	 0x10	 1382400  
	 */
	bool setGpsBaud(u8 level); 

	bool setTrailFilter(u8 level);  // [1, 7]
	bool setRainBlockDetectDistance(u8 level); //[0,3]  0: 30cm   1: 20cm   2: 10cm  
	bool setRainDetectSensitivity(u8 level);              // [0, 2]   0: high,  1: middle 2:low 
	bool setBlockDetectSensitivity(u8 level);                // [0, 2]   0: high,  1: middle 2:low 
	bool setMotorStopAngle(u16 angle);

 	// --end of airy funcs only

	

	bool readConfigToFile(char *fileName);    
	bool readEventLog(char *fileName);
	bool deleteEventLog();
	bool readMonitorLog(char *fileName);
	bool restartMonitorLog();
	bool readDaemonLog(char *fileName);
	bool deleteDaemonLog();

    bool writeReg(u32 addr, u32 data);
    bool readReg(u32 addr, u32 &value);	

    
    bool getZeroAngle(s32 &angle);    
    bool setZeroAngle(s32 angle);
    bool getChannelAngle(std::vector<float>& angles, u16 inAngleNum);
    bool setChannelAngle(std::vector<float>        angles);

	
    /**
     * @brief The uninitialization function, used to free objects,
     */
    void uninit();

private:

    bool disconnect();
    void *m_com; 
    RSCtrlDriverParam m_param;
};

}  // namespace lidar
}  // namespace robosense
