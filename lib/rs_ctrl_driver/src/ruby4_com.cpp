
#include <iostream>

#include "ruby4_com.h"

#define TASK_STATUS_UPDATE         1
#define TASK_OK                    0
#define TASK_ERROR                -1
#define TASK_PARA_ERROR           -2

//#pragma warning(disable:4996)

static std::vector<char> frameHeadPack( uint8_t cmdClass, uint8_t cmdIdx, uint16_t length)
{
    std::vector<char> frame_head_array;
    uint8_t buf[64] = {0xFF, 0xFF, 0x00,};

    // 本来memcpy就能搞定的事， APP那边非得搞u16ToHex()函数转换为大端, 恼火
    buf[3] = length >> 8;      // length high
    buf[4] = length & 0xFF;    // length low
    buf[5] = cmdClass;
    buf[6] = cmdIdx;

    frame_head_array.insert(frame_head_array.begin(), &buf[0], &buf[7]);

    return std::move(frame_head_array);
}

static void frameTailPack( std::vector<char> &sendData)
{
    uint8_t ch = 0xFE;
    sendData.insert(sendData.end(), 1, (char)ch);


    uint32_t size = sendData.size();
    uint16_t sum = 0;
    for(int k = 2; k < size; ++k)
    {
        sum += (uint8_t)sendData[k];
    }

    uint8_t buf[64] = {0};
    buf[0] = sum >> 8;      // length high
    buf[1] = sum & 0xFF;    // length low

    sendData.insert(sendData.end(), &buf[0], &buf[2]);

}


//
bool Ruby4Com::judeRecvPackCrc(std::vector<char> recvData)
{
    uint32_t size = recvData.size();
    uint16_t sum = 0, len;
    len = ( (uint8_t)recvData[3]<<8 ) | (uint8_t)recvData[4];
    // len + 8 为的包长度

    if( (len+8) > size )
    {
        sprintf(m_msg, "Err: at fun %s, recvData size is 0x%x <  len:  0x%x + 8", __func__, recvData.size(), len);
        writeErrLog(m_msg);
        return false;
    }
    
/*
    for(int k = 0;  k < size; ++k)
    {
        sprintf(m_msg, "at fun %s(), 0x%x", __func__, (u8)recvData[k]);
        writeDbgLog(m_msg);
    }
*/


    for(int k = 2; k < (5 + len + 1) && k < size; ++k)
    {
        sum += (uint8_t)recvData[k];

        sprintf(m_msg, "k = %d, data is 0x%x, sum is 0x%x", k, (u8)recvData[k], sum);
        writeDbgLog(m_msg);
    }

    uint16_t recvCrc = ( (uint8_t)recvData[len+6]<<8 )  | (uint8_t)recvData[len+7];
    if(recvCrc != sum)
    {
        sprintf(m_msg, "Err: at fun %s, recvCrc 0x%x !=  calcCrc:  0x%x ", __func__, recvCrc, sum);
        writeErrLog(m_msg);
        return false;
    }
    else
    {
        sprintf(m_msg, "at fun %s(), OK ",  __func__);
        writeDbgLog(m_msg);
        return true;
    }

}

Ruby4Com::Ruby4Com()
{
   writeDbgLog("Ruby4Com Init().");
}

Ruby4Com::~Ruby4Com()
{

}


s32 Ruby4Com::handleParamConfigCmd(char *package, uint16_t dataLen)
{
    u8 buf[1024] = {0};
    u16 len = 0;
    s32 ret = TASK_ERROR;
    
    sprintf(m_msg, "at fun %s, , cmd type package[6] = 0x%x.", __func__, package[6]);
    writeDbgLog(m_msg);

    len = 0;
    memset(buf, 0, sizeof(buf));
    buf[len++] = PARAM_SETTING_CLASS;
    buf[len++] = package[0];

    switch (package[6])
    {
    case SET_SN_CMD:
        read_write_state_[STATE_SET_SN] = true;
        break;
    case SET_NET_CFG_CMD:        
        read_write_state_[STATE_SET_NET] = true;
        break;

    case SET_TIME_SYNC_MODE_CMD:
        //ret = cfgTimeSyncMode(buffer[1]);
        read_write_state_[STATE_SET_SYNC_MODE] = true;
        
        break;
    case SET_WAVE_MODE_CMD:
        //ret = cfgWaveMode(buffer[1]);
        
        read_write_state_[STATE_SET_WAVE_MODE] = true;
        break;
    case SET_MOTOR_FOV_CMD:
        read_write_state_[STATE_SET_PHASE] = true;  
        break;
        
    case SET_PHASE_LOCKED_CMD:
		read_write_state_[STATE_SET_PHASE] = true;      
        break;
    case SET_MOTOR_SPEED_CMD:
		read_write_state_[STATE_MOTOR_SPEED] = true; 
        break;
    case SET_ALL_CONFIG_CMD:
        break;
    case SET_WORK_MODE:    	
    	read_write_state_[STATE_SET_MODE] = true;
        read_write_state_[STATE_SET_WORK_MODE] = true;
    }

    if (ret != TASK_OK)
    {
        buf[len++] = OTHER_ERROR;
    }
    else
    {
        buf[len++] = PROCESS_SUCCESS;
    }

    return ret;
}


s32 Ruby4Com::handleGetInfoCmd(char *package, uint16_t dataLen)
{

    s32 ret = TASK_ERROR;

    switch(package[6])
    {
    case GET_PARAM_INFO_CMD:
        read_write_state_[STATE_GET_ALL_CONFIG] = true;

	case 0x03:
		read_write_state_[STATE_GET_MODE] = true;

	case GET_CONFIG_VER_CMD:
		read_write_state_[STATE_GET_CONFIG_VER] = true;

    default:
        break;
    }

    return ret;
}


s32 Ruby4Com::handleFirmwareUpdateCmd(char *package, uint16_t dataLen)
{
    s32 ret = TASK_OK;
    char validTag = package[7];
    u8 cmdType = (u8)package[8];
    char percentTag = package[8];
    ModulesUpgradeFirmwareInfo_st info;

    //  7 对用户是7, 8,        0x0c,       0x0d,
    //NET_CMD_LINUX_APP_UPDATE      NET_CMD_CGI_BACK_UPDATE  
    // 0,         1,           2,      3,         4
    // info_app,info_appbac,info_cgi,info_cgibac,info_bot,info_botbac,info_top,info_topbac,info_toppra,info_mot,info_motbac,info_others;
    u8 typeIdx[32] = {7, 8, 0xc, 0xd, 5, 6, 3, 4 ,2, 0x16, 0x17, 0x18};

    switch(package[6])
    {
    case PATCH_FIRMWARE_UPDATE_CMD:
        if(0 == validTag)
        {
            if(0xC1 == cmdType)
            {
                read_write_state_[STATE_UPDATE_REQUEST] = true;
            }
            else if (0xC2 == cmdType)
            {
                read_write_state_[STATE_UPDATE_SEND_DATA] = true;
            }
            else if (0xC3 == cmdType)
            {
                read_write_state_[STATE_UPDATE_CHECKSUM] = true;
                
            }
        }
        
		m_updateType = NET_CMD_PATCH_UPDATE;  // 见 types.h	 
		m_updateStatus = 0x0300 + 100; // 最后一字节表示进度, 倒数第二字节表示是否出错。
        break;

    case CONFIG_FIRMWARE_UPDATE_CMD:
        if(0 == validTag)
        {
            if(0xC1 == cmdType)
            {
                read_write_state_[STATE_UPDATE_REQUEST] = true;
            }
            else if (0xC2 == cmdType)
            {
                read_write_state_[STATE_UPDATE_SEND_DATA] = true;
            }
            else if (0xC3 == cmdType)
            {
                read_write_state_[STATE_UPDATE_CHECKSUM] = true;
                
            }
        }
        
		m_updateType = NET_CMD_CONFIG_UPDATE;  // 见 types.h	 
		m_updateStatus = 0x0300 + 100; // 最后一字节表示进度, 倒数第二字节表示是否出错。
    	break;
    	
    case TOP_FIRMWARE_UPDATE_CMD:
        if(0 == validTag)
        {
            if(0xC1 == cmdType)
            {
                read_write_state_[STATE_UPDATE_REQUEST] = true;
            }
            else if (0xC2 == cmdType)
            {
                read_write_state_[STATE_UPDATE_SEND_DATA] = true;
            }
            else if (0xC3 == cmdType)
            {
                read_write_state_[STATE_UPDATE_CHECKSUM] = true;
            }
        }
        
        break;
    case STATUS_FIRMWARE_UPDATE_CMD:
        if(percentTag == 0x03)
        {
           m_updateStatus = (m_updateStatus | 0x0300) + 100;
           
           printf("upgrade finished!\n");
           return ret;
           

        }
        else if(percentTag == 0x0a)
        {
           m_updateStatus = m_updateStatus | 0x1000;
           printf("upgrade failed.\n");
          
        }
        else if(percentTag == 0x0b)
        {
           m_updateStatus = m_updateStatus | 0x2000;
           printf("bad data format\n");
          
        }

        for(int k = 0; k < 12; ++k)
        {
            memcpy(&info, &package[9+k*3], 3);
            if(0x0C == info.action)
            {
                printf("bad signature, k = %d, typeIdx = %d\n", k, typeIdx[k]);
                m_updateType = typeIdx[k];
                m_updateStatus = m_updateStatus | 0x4000;
                break;
            }
            else if (0x0D == info.action)
            {
                printf("bad  version, k = %d, typeIdx = %d\n", k, typeIdx[k]);
                m_updateType = typeIdx[k];
                m_updateStatus = m_updateStatus | 0x8000;
            }
            else if (2 == info.action)
            {
                m_updateType = typeIdx[k];
                m_updateStatus = (m_updateStatus & 0xff00) + info.progress;
                break;
            }
            
        }

            
        break;

 
    }
    return ret;
    
}

s32 Ruby4Com::handleMotorControlCmd(char *package, uint16_t dataLen)
{
    s32 ret = TASK_ERROR;

    switch(package[6])
    {
    case MOTOR_DATA_SEND_CMD:

        break;
    case MOTOR_DATA_RECV_CMD:

        break;
    case CODE_WHEEL_CALIBRATION_CMD:

        break;
    case GET_MOTOR_PRARAM_CMD:

        break;
    }
    return ret;
}



s32 Ruby4Com::handleTopControlCmd(char *package, uint16_t dataLen)
{
    s32 ret = TASK_ERROR;

    switch(package[6])
    {
    case READ_TOP_REG_CMD:
        //ret = handleReadTopRegisterCmd(&buffer[1]);
        read_write_state_[STATE_READ_TOP_REG] =true;
        break;
    case WRITE_TOP_REG_CMD:
        read_write_state_[STATE_WRITE_TOP_REG] =true;
        //ret = handleWriteTopRegisterCmd(&buffer[1]);
        break;
    case READ_TOP_FLASH_CMD:
        break;
    case WRITE_TOP_FLASH_CMD:
        break;
    case UPDATE_TOP_PARAM_FLASH_CMD:
        break;
    case TOP_GET_INTENSITY_CMD:
        read_write_state_[STATE_READ_INTENSITY] = true;
    case NET_ACK_EYES_SAFE_CMD:
        read_write_state_[STATE_EYE_SAFE_MODE] = true;
    }
    return ret;
}



s32 Ruby4Com::handleOtherControlCmd(char *package, uint16_t dataLen)
{
	s32 ret = TASK_ERROR;

	switch(package[6])
	{
	case 0x04:
		read_write_state_[STATE_REBOOT_LIDAR] = true;
		break;

	case 0X05: // 3.4.7.5 读取基础配置软件信息, 数据结构体
		//read_write_state_[] = true;
		break;

	case 0x06: // 3.4.7.6 读取基础配置json文件内容		
		read_write_state_[STATE_READ_CONFIG_FILE] = true;
		break;

	case 0x07: // 3.4.7.7 读取基础配置文件版本
		break;

	case 0x08: // 3.4.7.8 读取Event Log文件内容
		read_write_state_[STATE_READ_EVENT_LOG] = true;
		
		break;

	case 0x09: // 3.4.7.9 读取Monitor Log文件内容
		read_write_state_[STATE_READ_MONITOR_LOG] = true;
		break;

	case 0x0A: //3.4.7.10 删除当前Event Log文件	
		read_write_state_[STATE_DELETE_EVENT_LOG] = true;
		break;

	case 0x0B: // 3.4.7.11 重启Monitor
		read_write_state_[STATE_RESTART_MONITOR_LOG] = true;
		break;

	case 0x0C: // 3.4.7.12 读取Daemon Log文件内容 
		read_write_state_[STATE_READ_DAEMON_LOG] = true;
		break;

	case 0x0D: // 3.4.7.13 删除当前Daemon Log文件
		read_write_state_[STATE_DELETE_DAEMON_LOG] = true;
		break;

			
	}
	return ret;
}


s32 Ruby4Com::handleBotControlCmd(char *package, uint16_t dataLen)
{
    s32 ret = TASK_ERROR;

    sprintf(m_msg, "at fun %s, , cmd type package[6] = 0x%x.", __func__, package[6]);
    writeDbgLog(m_msg);

    switch(package[6])
    {
    case READ_BOT_REG_CMD:

        read_write_state_[STATE_READ_REG] = true;
        break;

    case WRITE_BOT_REG_CMD:
        read_write_state_[STATE_WRITE_REG] = true;
        break;

    case UPDATE_BOT_PARAM_FLASH_CMD:
        break;

    case SET_ZERO_ANGLE_CMD:

        break;

    case GET_ZERO_ANGLE_CMD:
        break;
    }
    return ret;
}



bool Ruby4Com::ParseRecvTcpData(std::vector<char> & read_data_buffer_)
{
    int32_t ret = 0;
    bool rst = true;

    sprintf(m_msg, "at fun %s, recv data size is 0x%x", __func__, read_data_buffer_.size());
    writeDbgLog(m_msg);

    while ( static_cast<uint64_t>(read_data_buffer_.size()) >= m_minPackBytes )
    {

        char* data = read_data_buffer_.data();

        while ( ( (uint8_t)data[0] != 0xFF ) ||  ( (uint8_t)data[1] != 0xFF ) ||  ( (uint8_t)data[2] != 0x01 ))
        {
            read_data_buffer_.erase(read_data_buffer_.begin());

            if (read_data_buffer_.size() >= m_minPackBytes )
            {
                data = read_data_buffer_.data();
            }
            else
            {
                sprintf(m_msg, "at fun %s, recv data size is 0x%x < minPack 0x%x", __func__, read_data_buffer_.size(), m_minPackBytes);
                writeErrLog(m_msg);
                return false;
            }
        }


        uint16_t len = ( (uint8_t)data[3]<<8 ) | (uint8_t)data[4]; // 包里解析出来的长度= 3 + 真实数据
        uint16_t size = read_data_buffer_.size();
        uint16_t dataLen = len - 3; // 实实在在的 真实数据

        if (size < (len + 8) || len < 3 || len > 2000) // 8 is = stx_2 + type_1 + len_2 + etx_1 + crc_2
        {
            sprintf(m_msg, "at fun %s, recv data size is 0x%x <  len:  0x%x + 8", __func__, read_data_buffer_.size(), len);
            writeErrLog(m_msg);
            return false;
        }
        else
        {

            if ( !judeRecvPackCrc(read_data_buffer_) )
            {
                read_data_buffer_.erase( read_data_buffer_.begin(),  read_data_buffer_.begin() + len + 8 );
                sprintf(m_msg, "Err: at fun %s, judeRecvPackCrc() failed.", __func__);
                writeErrLog(m_msg);
                return false;
            }

            char *package = read_data_buffer_.data();
            // reg_back_data_  包括DataLength, cmd, response + data
            memcpy(reg_back_data_, package + 3, 5 + dataLen);  

            switch ((u8)read_data_buffer_[5] )
            {
                case PARAM_SETTING_CLASS:
                    ret = handleParamConfigCmd(package, dataLen);
                    break;
                case PARAM_INFO_GET_CLASS:
                    ret = handleGetInfoCmd(package, dataLen);
                    break;
                case FIRMWARE_UPDATE_CLASS:
                    ret = handleFirmwareUpdateCmd(package, dataLen);
                    break;
                case MOTOR_CONTORL_CLASS:
                    ret = handleMotorControlCmd(package, dataLen);
                    break;
                case TOP_CONTORL_CLASS:
                    ret = handleTopControlCmd(package, dataLen);
                    break;
                case BOT_CONTORL_CLASS:
                    ret = handleBotControlCmd(package, dataLen);
                    break;
				case OTHER_CTRL_CLASS:
					ret = handleOtherControlCmd(package, dataLen);
					break;

                default:
                    rst = false;
                    break;

            }

            read_data_buffer_.erase( read_data_buffer_.begin(),  read_data_buffer_.begin() + len + 8 );

        }
    }

    return rst;

}


bool Ruby4Com::judeRegAddr(uint32_t start_reg_addr, uint32_t reg_number, uint8_t &retCmdClass)
{
    retCmdClass = 0;

    if (reg_number > 60 )  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }

    // 简单点只判断第一个，连续多个地址类型应当相同, 都为底板或都为主板Reg
    if(   (start_reg_addr >= m_topRegLow) && (start_reg_addr <= m_topRegHi)
       && ( (start_reg_addr + reg_number) < m_topRegHi )  )
    {
        retCmdClass = TOP_CONTORL_CLASS;
    }

    if(   (start_reg_addr >= m_botRegLow) && (start_reg_addr <= m_botRegHi)
       && ( (start_reg_addr + reg_number*4) < m_botRegHi )  )
    {
        retCmdClass = BOT_CONTORL_CLASS;
    }

    if(0 == retCmdClass)
    {
        return false;
    }

    return true;

}


// 简单点只判断第一个，多个地址类型应当相同, 都为底板或都为主板Reg
bool Ruby4Com::writeRegData(const std::vector<uint32_t>& reg_addr,
                           const std::vector<int32_t>& reg_val,
                           std::vector<char> & send_data,
                           uint8_t & statusIdx )
{
    uint8_t cmdClass = 0, buf[8] = {0};
    uint32_t addrSize = reg_addr.size();

    if ( addrSize != reg_val.size() || addrSize > 60 || 0 == addrSize )  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }

    if( ! judeRegAddr(reg_addr[0], reg_addr.size(), cmdClass) )
    {
        return false;
    }

    addrSize = 1; // app not support  addrSize > 1　at this time
    buf[0] = addrSize & 0xFF;
    buf[1] = addrSize >> 8;

    // WRITE_BOT_REG_CMD = WRITE_TOP_REG_CMD = 2;
    send_data = frameHeadPack(cmdClass, WRITE_BOT_REG_CMD, 2 + 8*addrSize + 2); // 先按单个寄存器




    for (int i = 0; i < addrSize; ++i)
    {
        /*if (reg_addr[i] > MAX_REG_ADDR || reg_addr[i] < MIN_REG_ADDR)
        {
            return false;
        }*/
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])) + sizeof(uint32_t));

        send_data.insert(send_data.end(), buf, buf+2) ;

        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<int32_t*>(&reg_val[i])),
                         reinterpret_cast<char*>(const_cast<int32_t*>(&reg_val[i])) + sizeof(int32_t));
    }

    frameTailPack(send_data);

    if(TOP_CONTORL_CLASS == cmdClass)
    {
        statusIdx = STATE_WRITE_TOP_REG;
    }
    else
    {
        statusIdx = STATE_WRITE_REG;
    }

    read_write_state_[statusIdx] = false;

    return true;
}

bool Ruby4Com::writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val,
                        std::vector<char> & send_data, uint8_t & statusIdx )
{
    uint8_t cmdClass = 0, buf[8] = {0};
    uint32_t addrSize = reg_val.size();

    if ( addrSize > 60 || 0 == addrSize )  // NOTE more than 60 will lead to unexpected error
    {
       return false;
    }

    if( ! judeRegAddr(start_reg_addr, addrSize, cmdClass) )
    {
       return false;
    }

    // WRITE_BOT_REG_CMD = WRITE_TOP_REG_CMD = 2;
    send_data = frameHeadPack(cmdClass, WRITE_BOT_REG_CMD, 2 + 8*addrSize); // 先按单个寄存器


    buf[0] = addrSize >> 8;
    buf[1] = addrSize & 0xFF;


    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)) + sizeof(uint32_t));

    send_data.insert(send_data.end(), buf, buf+2) ;

    // 这个地方还需要修改， 把数据也放进去

    for (int i = 0; i < addrSize; ++i)
    {
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_val[i]),
                         reinterpret_cast<char*>(&reg_val[i]) + sizeof(uint32_t));
    }

    frameTailPack(send_data);

    if(TOP_CONTORL_CLASS == cmdClass)
    {
        statusIdx = STATE_WRITE_TOP_REG;
    }
    else
    {
        statusIdx = STATE_WRITE_REG;
    }

    read_write_state_[statusIdx] = false;

}

//反射率标定
bool Ruby4Com::writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                         std::vector<char> & send_data,uint8_t & statusIdx)
{
    uint8_t cmdClass = 0, buf[8] = {0};
    uint32_t addrSize = reg_addr.size();

    if ( addrSize > 224 || 0 == addrSize )  // NOTE more than 60 will lead to unexpected error
    {
       return false;
    }

//    if( ! judeRegAddr(reg_addr[0], reg_addr.size(), cmdClass) )
//    {
//       return false;
//    }
    cmdClass = TOP_CONTORL_CLASS;
    //2(两个字节长度的cmd) + 2(两个字节长度的寄存器个数)+ 8 * addrSize(实际的数据长度)
    send_data = frameHeadPack(cmdClass, TOP_GET_INTENSITY_CMD, 2 + 2 + 8 * addrSize);

    //uint16_t regNum = addrSize;
    // memcpy(buf, &regNum, 2);
    buf[0] = addrSize & 0xFF;
    buf[1] = addrSize >> 8;
    send_data.insert(send_data.end(), buf, buf+2) ;

    for (uint32_t i = 0; i < addrSize; ++i)
    {
        /*if (reg_addr[i] > MAX_REG_ADDR || reg_addr[i] < MIN_REG_ADDR)
        {
            return false;
        }*/
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])) + sizeof(uint32_t));
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_val[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_val[i])) + sizeof(uint32_t));
    }

    frameTailPack(send_data);

    statusIdx = STATE_READ_INTENSITY;
    read_write_state_[STATE_READ_INTENSITY] = false;

    return true;

}

bool Ruby4Com::getReadRegSendData(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val,
                                  std::vector<char> & send_data, uint8_t & statusIdx)
{
    uint8_t cmdClass = 0, buf[8] = {0};
    uint32_t addrSize = addrs2read.size();

    if (  addrSize > 60 || 0 == addrSize )  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }

    if( ! judeRegAddr(addrs2read[0], addrSize, cmdClass) )
    {
        return false;
    }

    addrSize = 1; // app not support  addrSize > 1　at this time
    buf[0] = addrSize & 0xFF;
    buf[1] = addrSize >> 8;

    // READ_BOT_REG_CMD = READ_TOP_REG_CMD = 2;
    send_data = frameHeadPack(cmdClass, READ_BOT_REG_CMD, 2 + 4 + 2); // 先按单个 read 寄存器


    for (int i = 0; i < addrSize; ++i)
    {
        /*if (reg_addr[i] > MAX_REG_ADDR || reg_addr[i] < MIN_REG_ADDR)
        {
            return false;
        }*/
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&addrs2read[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&addrs2read[i])) + sizeof(uint32_t));

        send_data.insert(send_data.end(), buf, buf+2) ;

    }

    frameTailPack(send_data);

    if(TOP_CONTORL_CLASS == cmdClass)
    {
        statusIdx = STATE_READ_TOP_REG;
    }
    else
    {
        statusIdx = STATE_READ_REG;
    }

    read_write_state_[statusIdx] = false;

    return true;

}


bool Ruby4Com::getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val)
{

    if(0 != reg_back_data_[4]) // 从长度开始
    {
        sprintf(m_msg, "Err: ReadReg() addr is 0x%x, response code is 0x%x", addrs2read[0], reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    int addr_size = addrs2read.size();
    reg_val.resize(addr_size);
    for (int i = 0; i < addr_size; ++i) // 7 = Cmd_2 + response_1 + addr_4
    {
    	// bp4, 0350有返回地址, 新协议
        memcpy(&reg_val[i], reg_back_data_ + 9 + 4 * i, sizeof(int32_t));

        // ruby APP中没把地址返回, 老协议
        //memcpy(&reg_val[i], reg_back_data_ + 5 + 4 * i, sizeof(int32_t));
    }

    sprintf(m_msg, "at fun %s, size = 0x%x. val = 0x%x", __func__, addr_size, (u32)reg_val[0]);
    writeDbgLog(m_msg);

    return true;

}
//                                                   2       2     1         2      4     4
//获取发送反射率命令返回的结果  reg_back_data_[]  包括DataLength, cmd, response  regnum, addr, data
bool Ruby4Com::getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal)
{
    //返回数据包格式：  2B 寄存器长度 + 回读寄存ADDR（4B) + 回读寄存Value（4B) + 回读寄存ADDR（4B) + 回读寄存Value（4B) +.....
    //接收的寄存器长度

    if(0 != reg_back_data_[4]) // response code
    {
        sprintf(m_msg, "Err:  response code is  0x%x", reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint32_t packLen = ( ((uint8_t )reg_back_data_[0]) <<8) + ((uint8_t )reg_back_data_[1]);

    uint16_t readDataLenTemp = 0; // regNum
    memcpy(&readDataLenTemp, reg_back_data_ + 5, 2);

    vecRetRegVal.resize(readDataLenTemp);
    addrs2read.resize(readDataLenTemp);
    for (int i = 0; i < readDataLenTemp; ++i)
    {
        // APP中把地址返回,地址返回到界面,打印地址信息
        memcpy(&addrs2read[i], reg_back_data_ + (7 + 8 * i), sizeof(int32_t));
        // APP中把地址返回,目前只取值，地址先不返回到界面
        memcpy(&vecRetRegVal[i], reg_back_data_ + (11 + 8 * i), sizeof(int32_t));   //2Bit的数据长度，每条数据包括:ADDR（4B) + Value（4B)
    }
    return true;
}


bool Ruby4Com::readRegData(const uint32_t start_reg_addr,
                          const uint32_t reg_number,
                          std::vector<int32_t>& reg_val,
                          std::vector<char> & send_data)
{
    if (start_reg_addr > MAX_REG_ADDR || (start_reg_addr + sizeof(uint32_t) * reg_number) > MAX_REG_ADDR ||
        start_reg_addr < MIN_REG_ADDR || reg_number > 60)  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }
    //send_data = frameHeadPack(NET_CMD_READ_REGISTER_2, sizeof(uint32_t) * 2);
    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)) + sizeof(uint32_t));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_number)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_number)) + sizeof(uint32_t));

    //if (sendData(send_data, STATE_READ_REG_2, msec_10))
    {
        reg_val.resize(reg_number);
        for (int i = 0; i < reg_number; ++i)  // 7 = Cmd_2 + response_1 + addr_4
        {
            memcpy(&reg_val[i], reg_back_data_  + 7 + 4 * i, sizeof(int32_t));
        }
        return true;
    }
    //else
    {
        return false;
    }
}

bool Ruby4Com::SetLidarSn(u8 sn[6], std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_SN_CMD, 8);
    send_data.insert(send_data.end(), sn, sn+6) ;

    frameTailPack(send_data);

    statusIdx = STATE_SET_SN;
    read_write_state_[statusIdx] = false;
    return true;
}


bool Ruby4Com::SetLidarNetInfo(NetParam_st netInfo, std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_NET_CFG_CMD, 2 + sizeof(netInfo) );
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&netInfo),
                     reinterpret_cast<char*>(&netInfo) + sizeof(netInfo));


    frameTailPack(send_data);

    statusIdx = STATE_SET_NET;
    read_write_state_[statusIdx] = false;
    return true;

}


bool Ruby4Com::SetLidarSyncMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)
{
	u8 buf[8];
    memcpy(buf, &mode, 1);
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_TIME_SYNC_MODE_CMD, 3);
    send_data.insert(send_data.end(), buf, buf +1);
    frameTailPack(send_data);
    statusIdx = STATE_SET_SYNC_MODE;
    read_write_state_[statusIdx] = false;
    return true;
}

bool Ruby4Com::SetLidarWaveMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)
{
	u8 buf[8];
    memcpy(buf, &mode, 1);
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_WAVE_MODE_CMD, 3);
    send_data.insert(send_data.end(), buf, buf +1);
    frameTailPack(send_data);
    statusIdx = STATE_SET_WAVE_MODE;
    read_write_state_[statusIdx] = false;
    return true;

}


bool Ruby4Com::SetLidarFov(u16 start, u16 end, std::vector<char> & send_data, uint8_t &statusIdx)
{
    u8 buf[128];
    memcpy(buf, &start, sizeof(start));
    memcpy(&buf[2], &end, sizeof(end));
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_MOTOR_FOV_CMD, 6);
    send_data.insert(send_data.end(), buf, buf +4);
    frameTailPack(send_data);
    statusIdx = STATE_SET_FOV;
    read_write_state_[statusIdx] = false;
    return true;

}


bool Ruby4Com::SetMotorPhaseLockCfg(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{

    u8 buf[128];
    memcpy(buf, &angle, sizeof(angle));
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_PHASE_LOCKED_CMD, 4);
    send_data.insert(send_data.end(), buf, buf +2);
    frameTailPack(send_data);
    statusIdx = STATE_SET_PHASE;
    read_write_state_[statusIdx] = false;
    return true;

}

bool Ruby4Com::SetMotorSpeed(u8 level, std::vector<char> & send_data, uint8_t &statusIdx) // 1 ---300; 2---600; 3---1200
{
    u8 buf[8];
    memcpy(buf, &level, 1);
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_MOTOR_SPEED_CMD, 3);
    send_data.insert(send_data.end(), buf, buf + 1);
    frameTailPack(send_data);
    statusIdx = STATE_MOTOR_SPEED;
    read_write_state_[statusIdx] = false;
    return true;

}

//int SetLidarTrigerAngle(u16 angle);
//设置雷达工作模式
bool Ruby4Com::SetLidarWorkMode(u8 mode, std::vector<char> & send_data, uint8_t &statusIdx)  // 0  standby, 1 normal
{
    u8 buf[8];
    memcpy(buf, &mode, 1);
    send_data = frameHeadPack(PARAM_SETTING_CLASS, SET_WORK_MODE, 3);
    send_data.insert(send_data.end(), buf, buf+1);
    frameTailPack(send_data);
    statusIdx = STATE_SET_WORK_MODE;
    read_write_state_[statusIdx] = false;
    return true;
}


bool Ruby4Com::CtrlCodeWheelCalibrate(u8 ctrl, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool Ruby4Com::WriteTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool Ruby4Com::ReadTopFlash(u32 startAddr, char *file, std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}


bool Ruby4Com::CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx)
{
    u8 buf[8];
    memcpy(buf, &on, 1);
    send_data = frameHeadPack(TOP_CONTORL_CLASS, NET_ACK_EYES_SAFE_CMD, 3);
    send_data.insert(send_data.end(), buf, buf + 1);
    frameTailPack(send_data);
    statusIdx = STATE_EYE_SAFE_MODE;
    read_write_state_[statusIdx] = false;
    return true;
}

bool Ruby4Com::SetZeroAngle(s32 angle, std::vector<char> & send_data, uint8_t &statusIdx) 
{
    return false;
}


bool Ruby4Com::GetZeroAngleSendData( std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;
}

bool  Ruby4Com::GetZeroAngleResult(s32 &angle)
{
    return false;
}

bool  Ruby4Com::SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles)
{
    return false;
}

bool  Ruby4Com::GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum)
{
	return false;
}

bool  Ruby4Com::GetChannelAngleResult(std::vector<float>& angles)
{
	return false;
}



bool Ruby4Com::getPatchVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) 
{
    send_data = frameHeadPack(PARAM_INFO_GET_CLASS, GET_CONFIG_VER_CMD, 2);

    frameTailPack(send_data);

    statusIdx = STATE_GET_CONFIG_VER;
    read_write_state_[statusIdx] = false;
    return true;

}

bool Ruby4Com::getPatchVerResult(u32 &ver)
{
    if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t dataLen = 4; // 

    memcpy(&ver, &reg_back_data_[5], dataLen);

    return true;

}


bool Ruby4Com::getConfigVerSendData(std::vector<char> & send_data, uint8_t & statusIdx) 
{
    send_data = frameHeadPack(PARAM_INFO_GET_CLASS, GET_CONFIG_VER_CMD, 2);

    frameTailPack(send_data);

    statusIdx = STATE_GET_CONFIG_VER;
    read_write_state_[statusIdx] = false;
    return true;

}

bool Ruby4Com::getConfigVerResult(char ver[32])
{
    if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t dataLen = 4; // 

    memcpy(ver, &reg_back_data_[5], dataLen);

    return true;

}


bool Ruby4Com::getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx)
{
    send_data = frameHeadPack(PARAM_INFO_GET_CLASS, GET_PARAM_INFO_CMD, 2);

    frameTailPack(send_data);

    statusIdx = STATE_GET_ALL_CONFIG;
    read_write_state_[statusIdx] = false;
    return true;
}

bool Ruby4Com::getConfigResult(ConfigPara &para)
{

    if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t len = ( (uint8_t)reg_back_data_[3]<<8 ) | (uint8_t)reg_back_data_[4]; // 包里解析出来的长度= 3 + 真实数据
    uint16_t dataLen = len - 3; // 实实在在的 真实数据

    //config.insert(config.end(),  (&reg_back_data_[5]),  (&reg_back_data_[5]) + dataLen);
    memcpy(&para.r4info, &reg_back_data_[5], dataLen);

    return true;
}


bool Ruby4Com::sendUpdateRequest(u8 type, uint32_t len, 
                                    std::vector<char> & send_data, uint8_t & statusIdx )
{   
	m_updateType = 0;
	m_updateStatus = 0;	

    u8 buf[8];
    buf[0] = 0xC1;
    memcpy(&buf[1], &len, 4);
    if(NET_CMD_PATCH_UPDATE == type)
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x03, 7);
    }
    else
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x01, 7);
    }
    send_data.insert(send_data.end(), buf, buf +5);
    frameTailPack(send_data);
    statusIdx = STATE_UPDATE_REQUEST;
    read_write_state_[statusIdx] = false;
	
    return true;
}


bool Ruby4Com::sendUpdateData(u8 type, std::vector<char> & file_data, 
									std::vector<char> & send_data, uint8_t & statusIdx )
{	
	u8 buf[8];
	buf[0] = 0xC2;	
	uint32_t len = file_data.size();
	//memcpy(&buf[1], &len, 4);
	if(NET_CMD_PATCH_UPDATE == type)
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x03, 3 + len);
    }
    else if (NET_CMD_CONFIG_UPDATE == type)
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x04, 3 + len);
    }
    else
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x01, 3 + len);
    }
    

	send_data.insert(send_data.end(), buf, buf +1);	
    send_data.insert(send_data.end(), file_data.begin(), file_data.end() );
	frameTailPack(send_data);
	statusIdx = STATE_UPDATE_SEND_DATA;
	read_write_state_[statusIdx] = false;
	
	return true;
}


bool Ruby4Com::sendUpdateCheckSum(u8 type, uint32_t checkSum, 
									std::vector<char> & send_data, uint8_t & statusIdx )
{	
	u8 buf[8];
	buf[0] = 0xC3;	
	memcpy(&buf[1], &checkSum, 4);
	
    if(NET_CMD_PATCH_UPDATE == type)
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x03, 7);
    }
    else
    {
    	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x01, 7);
    }

	send_data.insert(send_data.end(), buf, buf +5); 
	frameTailPack(send_data);
	statusIdx = STATE_UPDATE_CHECKSUM;
	read_write_state_[statusIdx] = false;
	
	return true;
}

bool Ruby4Com::SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}


// get query need send data
bool Ruby4Com::queryUpdateStatus(std::vector<char> & send_data, uint8_t & statusIdx )
{
	
	send_data = frameHeadPack(FIRMWARE_UPDATE_CLASS, 0x02, 2);
	frameTailPack(send_data);

	// 不用观注返回值
	statusIdx = STATE_UPDATE_CHECKSUM;
	read_write_state_[statusIdx] = false;
	
	return true;
}


bool Ruby4Com::getUpdateStatus(u8 &type, s32 &status)
{
	type = m_updateType;
	status = m_updateStatus;
	//printf("HeliosCom::getUpdateStatus(), type is %d, status = %d\n", type, status);
	
	return true;
}




// mode = 0 为待机模式， 1为正常模式
bool Ruby4Com::setMode(u8 mode, std::vector<char> & send_data, uint8_t & statusIdx)
{
	u8 buf[8];
	u16  type = (u16)mode;
	memcpy(buf, &type, 2);
	
	send_data = frameHeadPack(PARAM_SETTING_CLASS, 0x09, 4);
	send_data.insert(send_data.end(), buf, buf +2); 
	frameTailPack(send_data);
	statusIdx = STATE_SET_MODE;
	read_write_state_[statusIdx] = false;
	
	return true;

}

bool Ruby4Com::getMode( std::vector<char> & send_data, uint8_t & statusIdx)
{
    send_data = frameHeadPack(PARAM_INFO_GET_CLASS, 0x03, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_GET_MODE;
    read_write_state_[statusIdx] = false;
	
    return true;
}

bool Ruby4Com::getModeResult(uint8_t &mode)
{
    memcpy(&mode, &reg_back_data_[5], 1);

    return true;
}


bool Ruby4Com::rebootLidar(std::vector<char> & send_data, uint8_t & statusIdx)
{
	u8 buf[8];
	buf[0] = 1;
	send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x04, 3);
	send_data.insert(send_data.end(), buf, buf +1); 
	frameTailPack(send_data);
	statusIdx = STATE_REBOOT_LIDAR;
	read_write_state_[statusIdx] = false;

	return true;

}


bool Ruby4Com::readEventLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x08, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_READ_EVENT_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}

bool Ruby4Com::getEventLogData(std::vector<char> & recv_data)
{
	if( !read_write_state_[STATE_READ_EVENT_LOG])
	{
		return false;
	}
	
    if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t len = ( (uint8_t)reg_back_data_[3]<<8 ) | (uint8_t)reg_back_data_[4]; // 包里解析出来的长度= 3 + 真实数据
    uint16_t dataLen = len - 3; // 实实在在的 真实数据

	// Event log文件内容，，前两个字节表示总包数，接下来两个字节表示当前包数
	recv_data.insert(recv_data.end(), &reg_back_data_[5], &reg_back_data_[dataLen]);


    return true;

}


bool Ruby4Com::deleteEventLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x0A, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_DELETE_EVENT_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}

bool Ruby4Com::readMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x09, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_READ_MONITOR_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}

bool Ruby4Com::restartMonitorLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x0B, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_RESTART_MONITOR_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}



bool Ruby4Com::getMonitorLogData(std::vector<char> & recv_data)
{
	if( !read_write_state_[STATE_READ_MONITOR_LOG])
	{
		return false;
	}

    if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t len = ( (uint8_t)reg_back_data_[3]<<8 ) | (uint8_t)reg_back_data_[4]; // 包里解析出来的长度= 3 + 真实数据
    uint16_t dataLen = len - 3; // 实实在在的 真实数据

	// Event log文件内容，，前两个字节表示总包数，接下来两个字节表示当前包数
	recv_data.insert(recv_data.end(), &reg_back_data_[5], &reg_back_data_[dataLen]);


    return true;

}


bool Ruby4Com::readDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x0C, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_READ_DAEMON_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}

bool Ruby4Com::deleteDaemonLog(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x0D, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_DELETE_DAEMON_LOG;
    read_write_state_[statusIdx] = false;
	
    return true;

}

bool Ruby4Com::getDaemonData(std::vector<char> & recv_data)
{
	if( !read_write_state_[STATE_READ_DAEMON_LOG])
	{
		return false;
	}

	if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t len = ( (uint8_t)reg_back_data_[3]<<8 ) | (uint8_t)reg_back_data_[4]; // 包里解析出来的长度= 3 + 真实数据
    uint16_t dataLen = len - 3; // 实实在在的 真实数据

	// Event log文件内容，，前两个字节表示总包数，接下来两个字节表示当前包数
	recv_data.insert(recv_data.end(), &reg_back_data_[5], &reg_back_data_[dataLen]);


    return true;
}


bool Ruby4Com::readConfigFile(std::vector<char> & send_data, uint8_t &statusIdx)
{
    send_data = frameHeadPack(OTHER_CTRL_CLASS, 0x06, 2);

    frameTailPack(send_data);
   
    statusIdx = STATE_READ_CONFIG_FILE;
    read_write_state_[statusIdx] = false;
	
    return true;

}


bool Ruby4Com::getConfigFileData(std::vector<char> & recv_data)
{
	if( !read_write_state_[STATE_READ_CONFIG_FILE])
	{
		return false;
	}

	if(0 != reg_back_data_[4])
    {
        sprintf(m_msg, "Err: at fun %s() addr is 0x%x, response code is 0x%x", __func__, reg_back_data_[4]);
        writeErrLog(m_msg);
        return false;
    }

    uint16_t len = ( (uint8_t)reg_back_data_[3]<<8 ) | (uint8_t)reg_back_data_[4]; // 包里解析出来的长度= 3 + 真实数据
    uint16_t dataLen = len - 3; // 实实在在的 真实数据

	// Event log文件内容，，前两个字节表示总包数，接下来两个字节表示当前包数
	recv_data.insert(recv_data.end(), &reg_back_data_[5], &reg_back_data_[dataLen]);


    return true;
}


