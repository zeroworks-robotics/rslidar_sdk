

#include <sys/stat.h>
#include <iostream>
//#include <direct.h>

#pragma warning(disable:4996) 

#include "lidar_base.h"
#include "mems.h"


static uint16_t gs_logIdx = 0;



static uint16_t checkSum(const FrameHead& frame_head)
{
    uint32_t sum = 0;
    sum += frame_head.frameFlag & 0xFFFF;
    sum += (frame_head.frameFlag >> 16) & 0xFFFF;

    sum += frame_head.length & 0xFFFF;
    sum += (frame_head.length >> 16) & 0xFFFF;

    sum += frame_head.cmd & 0xFFFF;
    sum += (frame_head.cmd >> 16) & 0xFFFF;
    sum = (sum >> 16) + (sum & 0xFFFF);

    return static_cast<uint16_t>(~sum);
}

static uint16_t checkSum(uint8_t* array, int length)
{
    uint32_t sum = 0;
    int i        = 0;
    while (1 < length)
    {
        sum += (static_cast<uint32_t>((static_cast<uint16_t>(array[i]) & 0x00ff) |
                                      ((static_cast<uint16_t>(array[i + 1])) << 8))) &    0x0000ffff;
        length -= 2;
        i += 2;
    }

    if (length)
    {
        sum += static_cast<uint32_t>(array[i]);
    }

    while (sum >> 16)
    {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    return static_cast<uint16_t>(~sum);
}

static std::vector<char> frameHeadPack(const uint32_t cmd, const uint32_t length)
{
    std::vector<char> frame_head_array;
    FrameHead frame_head;

    frame_head.frameFlag = FRAME_FLAG;
    frame_head.cmd       = cmd;
    frame_head.length    = length;
    frame_head.checkSum  = checkSum(frame_head);
    frame_head_array.insert(frame_head_array.begin(), reinterpret_cast<char*>(&frame_head),
                            reinterpret_cast<char*>(&frame_head) + sizeof(frame_head));

    return std::move(frame_head_array);
}


LidarBase::LidarBase() 
{
	/*
    ++gs_logIdx;
    char file[32] = {0};
    char folderName[] = "log";
    sprintf(file, "./log/lib-log%d.txt", gs_logIdx);
    m_fp = fopen(file, "w");
    if( NULL == m_fp )
    {
        if(0 != mkdir(folderName, 777))
        {
            m_fp = fopen(file, "a+");
        }
    }
    */
}

LidarBase::~LidarBase()
{    
    if( NULL != m_fp )
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}

void LidarBase::writeLog(char *msg, uint8_t level )
{   
	if( NULL == m_fp )
	{
		return;
	}

    if(level <= m_logLevel)
    {
        fprintf(m_fp, "%s\n", msg);
        fflush(m_fp);
    }
}


void LidarBase::writeErrLog(char *msg)
{
    writeLog(msg, 1);   
}


void LidarBase::writeDbgLog(char *msg)
{
    writeLog(msg, 2); 
}


// 0 不输出任何日志
// 1 输出error 日志
// 2 或其它 输出error 和 dbg 日志
void LidarBase::ctrlLog(uint8_t level )
{
    m_logLevel = level;
}

bool LidarBase::checkRecvCmd(ReadWriteStateIndex index)
{
    if(index >= STATE_TCP_START && index < STATE_TOTAL_NUM)
    {
        return read_write_state_[index];
    }

    return false;
}

bool LidarBase::ParseRecvTcpData(std::vector<char> &data)
{
    return false;
}




bool LidarBase::writeRegData(const std::vector<uint32_t>& reg_addr,
                           const std::vector<int32_t>& reg_val,
                           std::vector<char> & send_data, uint8_t & statusIdx)
{
    return false;
}

bool LidarBase::writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val, 
                                    std::vector<char> & send_data,  uint8_t & statusIdx)
{


    return false;
}

bool LidarBase::writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                          std::vector<char> & send_data,uint8_t & statusIdx)
{
    return false;
}

bool LidarBase::getReadRegSendData(const std::vector<uint32_t>& addrs2read,
                          std::vector<int32_t>& reg_val,
                          std::vector<char> & send_data, 
                          uint8_t & statusIdx)
{
    return false;
}

bool LidarBase::SetLidarSn(u8 sn[6], std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::SetLidarNetInfo(NetParam_st netInfo, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::SetLidarSyncMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::SetLidarWaveMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::SetLidarFov(u16 start, u16 end, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::SetMotorPhaseLockCfg(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::SetMotorSpeed(u8 level, std::vector<char> & send_data, uint8_t &statusIdx) // 1 ---300; 2---600; 3---1200
{
    return false;
}

bool  LidarBase::SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


//int SetLidarTrigerAngle(u16 angle);
bool LidarBase::SetLidarWorkMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)  // 0  standby, 1 normal
{
    return false;
}


bool LidarBase::CtrlCodeWheelCalibrate(u8 ctrl, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::WriteTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::ReadTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::SetZeroAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool LidarBase::GetZeroAngleSendData( std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool LidarBase::GetZeroAngleResult(s32 &angle)
{
    return false;
}

bool LidarBase::GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum)
{
    return false;
}

bool LidarBase::GetChannelAngleResult(std::vector<float>& angles)
{
    return false;
}

bool  LidarBase::SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles)
{
	return false;
}



bool LidarBase::readConfigFile(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::getConfigFileData(std::vector<char> & recv_data)
{
	return false;
}

bool LidarBase::getImuParamsSendData(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::getImuParamsResult(ImuParam &getParams)
{
	return false;
}

bool LidarBase::setImuParamsSendData(ImuParam getParams, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::readEventLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::deleteEventLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::readMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::restartMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx)

{
	return false;
}

bool LidarBase::getEventLogData(std::vector<char> & recv_data)
{
	return false;
}

bool LidarBase::getMonitorLogData(std::vector<char> & recv_data)
{
	return false;
}

bool LidarBase::readDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}
bool LidarBase::deleteDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::getDaemonData(std::vector<char> & recv_data)
{
	return false;
}

bool LidarBase::getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val)
{
    return false;
}

bool LidarBase::getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal)
{
    return false;
}

bool LidarBase::readRegData(const uint32_t start_reg_addr,
                          const uint32_t reg_number,
                          std::vector<int32_t>& reg_val,
                          std::vector<char> & send_data)
{
    return false;

}

bool LidarBase::setConfigSendData(ConfigPara para, std::vector<char> & send_data, uint8_t & statusIdx)
{
    return false;
}


bool LidarBase::getPatchVerSendData(std::vector<char> & send_data, uint8_t & statusIdx)
{
    return false;

}
bool LidarBase::getPatchVerResult(u32 &para) 
{
    return false;

}

bool LidarBase::getConfigVerSendData(std::vector<char> & send_data, uint8_t & statusIdx)
{
    return false;

}
bool LidarBase::getConfigVerResult(char ver[32]) 
{
    return false;

}


bool LidarBase::getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx)
{
    return false;

}
bool LidarBase::getConfigResult(ConfigPara &para) 
{
    return false;

}

void LidarBase::getNetworkInfo(NetworkInfo& network_info) const
{
    //network_info = net_work_info_;
}



bool LidarBase::readNetworkInfo()
{

        return false;

}

bool LidarBase::readViewingFieldParams(std::vector<double>& param)
{
    return false;
}

bool LidarBase::writeViewingFieldParams(const std::string& csv_path)
{    
    return false;
}

bool LidarBase::writeViewingFieldParams(const std::vector<double>& param)
{
    return false;
}

bool LidarBase::fixRegister(const uint32_t msec_10)
{
    return false;
}

bool LidarBase::getIntensityData(const std::vector<uint32_t>& addrs2read,
                               std::vector<int32_t>& calib_data,
                               std::vector<char> & send_data)
{
    return false;

}

bool LidarBase::writeToFlash(const std::string& file_path, std::string& msg, const int update_type)
{
    return false;
}


bool LidarBase::setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec )
{
    return false;

}


bool LidarBase::update(const u8 type, std::vector<char> file_data, 
								std::vector<char> & send_data, uint8_t & statusIdx )
{
	return false;
}


bool LidarBase::sendUpdateRequest(u8 type, uint32_t len, std::vector<char> & send_data, uint8_t & statusIdx )
{
    return false;
}


bool LidarBase::sendUpdateData(u8 type, std::vector<char> & file_data, std::vector<char> & send_data, uint8_t & statusIdx )
{
    return false;
}

bool LidarBase::sendUpdateCheckSum(u8 type, uint32_t checkSum, std::vector<char> & send_data, uint8_t & statusIdx )
{
    return false;
}

bool LidarBase::queryUpdateStatus(std::vector<char> & send_data, uint8_t & statusIdx )
{
	return false;
}

bool LidarBase::rebootLidar(std::vector<char> & send_data, uint8_t & statusIdx)
{
	return false;
}

bool LidarBase::getUpdateStatus(u8 &type, s32 &status)
{
	return false;
}

bool LidarBase::setMode(u8 mode, std::vector<char> & send_data, uint8_t & statusIdx)
{
	return false;
}

bool LidarBase::getMode( std::vector<char> & send_data, uint8_t & statusIdx)
{
	return false;
}

bool LidarBase::getModeResult(uint8_t &mode)
{
	return false;
}


bool LidarBase::setMotorDir(u8 mode, std::vector<char> & send_data, uint8_t & statusIdx)
{
	return false;
}

bool LidarBase::getMotorDir( std::vector<char> & send_data, uint8_t & statusIdx)
{
	return false;
}

bool LidarBase::getMotorResult(uint8_t &mode)
{
	return false;
}

bool LidarBase::getSupplementParamsSendData(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


bool LidarBase::getSupplementParamsResult(NetInfo2 &getParams)
{
	return false;
}

bool LidarBase::setSomeSupplementParamsSendData(NetInfoSet setParams, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setReflectEnhanceSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setGpsBaudSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


bool LidarBase::setTrailFilterSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setRainBlockDetectDistanceSendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setRainDetectSensitivitySendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setBlockDetectSensitivitySendData(u8 level, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setFixTopRegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


bool LidarBase::setFixBotRegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::setFix459RegSendData(std::vector<char> input_data, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


bool LidarBase::getMonitorSendData(std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

bool LidarBase::getMonitorResult(float value[64])
{
	return false;
}


