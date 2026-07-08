

#include <iostream>

#pragma warning(disable:4996) 

#include "mems_com.h"





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



MemsCom::MemsCom()
{
  
}

MemsCom::~MemsCom()
{

}



bool MemsCom::ParseRecvTcpData(std::vector<char> & read_data_buffer_)
{
#if 1
    //read_data_buffer_.insert(read_data_buffer_.end(), precv_buffer, precv_buffer + read_size);
    while ( static_cast<uint64_t>(read_data_buffer_.size()) >= sizeof(FrameHead) )
    {

		FrameHead frame_head;
		char* data = read_data_buffer_.data();
		memcpy(&frame_head, data, sizeof(FrameHead));        

        while (frame_head.frameFlag != FRAME_FLAG)   
        {
            read_data_buffer_.erase(read_data_buffer_.begin());			

			if (read_data_buffer_.size() >= sizeof(FrameHead))
			{
				char* data = read_data_buffer_.data();
				memcpy(&frame_head, data, sizeof(FrameHead));
			}
			else
			{
				//read_data_buffer_.clear();
				return false;
			}
			
        }

        if (frame_head.checkSum != checkSum(frame_head))
        {
            read_data_buffer_.erase(read_data_buffer_.begin(), read_data_buffer_.begin() + 4);
            continue;
        }
        else
        {
            bool temp_bool   = false;
            if (sizeof(FrameHead) + frame_head.length > static_cast<uint64_t>(read_data_buffer_.size()) )
            {
                return false;
            }           
           

            switch (frame_head.cmd)
            {
                case NET_CMD_ACK_READ_REGISTER:  // 读寄存器
                {
                    memcpy(reg_back_data_, data + sizeof(frame_head), frame_head.length);
                    read_write_state_[STATE_READ_REG] = true;
                    break;
                }
                
                case NET_CMD_ACK_READ_REGISTER_2:  // 读连续寄存器, frame_header + reg_addr_begin + number of register +
                {
                    uint32_t register_num = 0;
                    memcpy(&register_num, data + sizeof(frame_head) + sizeof(uint32_t), sizeof(uint32_t));
                    if ((register_num + 2) * sizeof(int32_t) == frame_head.length)
                    {
                        memcpy(reg_back_data_, data + sizeof(frame_head) + sizeof(int32_t) * 2, register_num * sizeof(int32_t));
                        read_write_state_[STATE_READ_REG_2] = true;
                    }
                    break;
                }
                case NET_CMD_ACK_WRITE_REGISTER:  // 写寄存器
                {
                    read_write_state_[STATE_WRITE_REG] = true;
                    break;
                }
                case NET_CMD_ACK_WRITE_REGISTER_2:
                {
                    read_write_state_[STATE_WRITE_REG_2] = true;
                    break;
                }
                case NET_CMD_ACK_GET_INTENSITY:
                {
                    memcpy(collect_back_data_, data + sizeof(frame_head), frame_head.length);
                    read_write_state_[STATE_READ_INTENSITY] = true;
                    break;
                }
                case NET_CMD_ACK_READ_PARAMETER:
                {
                    memcpy(&net_work_info_, data + sizeof(frame_head), frame_head.length);
                    read_write_state_[STATE_READ_NETWORK_INFO] = true;
                    break;
                }
                case NET_CMD_ACK_WRITE_PARAMETER:
                {
                    read_write_state_[STATE_WRITE_NETWORK_INFO] = true;
                    break;
                }
                case NET_CMD_ACK_READ_VIEW_PARAMETER:
                {
                    if (frame_head.length == sizeof(ViewingFieldParam))
                    {
                        memcpy(reg_back_data_, data + sizeof(frame_head), frame_head.length);
                        read_write_state_[STATE_READ_VIEWING_FIELD_PARAM] = true;
                    }
                    break;
                }
                case NET_CMD_ACK_WRITE_VIEW_PARAMETER:
                {
                    read_write_state_[STATE_WRITE_SPLICE_PARAMS] = true;
                    break;
                }
                case NET_CMD_ACK_FPGA_TO_FLASH:
                {
                    read_write_state_[STATE_FIX_REG] = true;
                    break;
                }
                case NET_CMD_ACK_WRITE_FLASH:
                {
                    memcpy(write_flash_back_data_, data + sizeof(frame_head), frame_head.length);
                    uint32_t state;
                    memcpy(&state, write_flash_back_data_, sizeof(state));
                    if (state != 1)
                    {
                        read_write_state_[STATE_WRITE_FLASH] = true;
                    }
                    break;
                }
                case NET_CMD_ACK_WRITE_FPGA_MEM: // 对于helios是NET_CMD_ACK_READ_CONFIG
                {                    
                    memcpy(reg_back_data_, data + sizeof(frame_head), frame_head.length);
                    read_write_state_[STATE_GET_ALL_CONFIG] = true;
                    break;
                }
                
                default:
                    break;
                    
            }
            
            read_data_buffer_.erase(
                read_data_buffer_.begin(),
                read_data_buffer_.begin() + static_cast<int32_t>(sizeof(frame_head) + frame_head.length));
        }
    }
        
    return true;
    
#endif
}



bool MemsCom::writeRegData(const std::vector<uint32_t>& reg_addr,
                           const std::vector<int32_t>& reg_val,
                           std::vector<char> & send_data,
                           uint8_t & statusIdx )
{
    if (reg_addr.size() != reg_val.size() || reg_addr.size() > 60)  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }
    //writeCMD(NET_CMD_TCP_START, STATE_TCP_START, 10);
    send_data = frameHeadPack(NET_CMD_WRITE_REGISTER, (sizeof(uint32_t) + sizeof(int32_t)) * reg_addr.size());
    for (int i = 0, reg_addr_size = reg_addr.size(); i < reg_addr_size; ++i)
    {
        if (reg_addr[i] > MAX_REG_ADDR || reg_addr[i] < MIN_REG_ADDR)
        {
            return false;
        }
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_addr[i])) + sizeof(uint32_t));
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<int32_t*>(&reg_val[i])),
                         reinterpret_cast<char*>(const_cast<int32_t*>(&reg_val[i])) + sizeof(int32_t));
    }

    statusIdx = STATE_WRITE_REG;
    read_write_state_[STATE_WRITE_REG] = false;
    
    return true;
}

bool MemsCom::writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val,
                                std::vector<char> & send_data, uint8_t & statusIdx )
{
    int reg_number = reg_val.size();  // NOTE more than 60 will lead to unexpected error
    if (start_reg_addr > MAX_REG_ADDR || (start_reg_addr + sizeof(uint32_t) * reg_number) > MAX_REG_ADDR ||
        start_reg_addr < MIN_REG_ADDR || reg_number < 1 || reg_number > 60)
    {
        return false;
    }
    send_data = frameHeadPack(NET_CMD_WRITE_REGISTER_2, sizeof(uint32_t) * (reg_number + 2));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)) + sizeof(uint32_t));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_number),
                     reinterpret_cast<char*>(&reg_number) + sizeof(uint32_t));
    for (int i = 0; i < reg_number; ++i)
    {
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_val[i]),
                         reinterpret_cast<char*>(&reg_val[i]) + sizeof(uint32_t));
    }
    
    statusIdx = STATE_WRITE_REG_2;
    read_write_state_[STATE_WRITE_REG_2] = false;

    return true;
}

bool MemsCom::getReadRegSendData(const std::vector<uint32_t>& addrs2read, 
                                std::vector<int32_t>& reg_val, 
                                std::vector<char> & send_data,
                                uint8_t & statusIdx) 
{
    if (addrs2read.size() > 60)  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }
    send_data = frameHeadPack(NET_CMD_READ_REGISTER, sizeof(uint32_t) * addrs2read.size());

    for (int i = 0, addr_size = addrs2read.size(); i < addr_size; ++i)
    {
        if (addrs2read[i] > MAX_REG_ADDR || addrs2read[i] < MIN_REG_ADDR)
        {
            return false;
        }
        send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&addrs2read[i])),
                         reinterpret_cast<char*>(const_cast<uint32_t*>(&addrs2read[i])) + sizeof(uint32_t));
    }

    statusIdx = STATE_READ_REG;
    read_write_state_[STATE_READ_REG] = false;


}


bool MemsCom::getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val) 
{
    int addr_size = addrs2read.size();
    reg_val.resize(addr_size);
    for (int i = 0; i < addr_size; ++i)
    {
        memcpy(&reg_val[i], reg_back_data_ + 4 * i, sizeof(int32_t));
    }

    return true;

}




bool MemsCom::readRegData(const uint32_t start_reg_addr,
                          const uint32_t reg_number,
                          std::vector<int32_t>& reg_val,
                          std::vector<char> & send_data)
{

    return false;
    /*
    if (start_reg_addr > MAX_REG_ADDR || (start_reg_addr + sizeof(uint32_t) * reg_number) > MAX_REG_ADDR ||
        start_reg_addr < MIN_REG_ADDR || reg_number > 60)  // NOTE more than 60 will lead to unexpected error
    {
        return false;
    }
    send_data = frameHeadPack(NET_CMD_READ_REGISTER_2, sizeof(uint32_t) * 2);
    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&start_reg_addr)) + sizeof(uint32_t));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_number)),
                     reinterpret_cast<char*>(const_cast<uint32_t*>(&reg_number)) + sizeof(uint32_t));

    //if (sendData(send_data, STATE_READ_REG_2, msec_10))
    {
        reg_val.resize(reg_number);
        for (int i = 0; i < reg_number; ++i)
        {
            memcpy(&reg_val[i], reg_back_data_ + 4 * i, sizeof(int32_t));
        }
        return true;
    }
    //else
    {
        return false;
    }
    */
}

void MemsCom::getNetworkInfo(NetworkInfo& network_info) const
{
    network_info = net_work_info_;
}

bool MemsCom::setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec_10)
{
    std::vector<char> send_data = frameHeadPack(NET_CMD_WRITE_PARAMETER, sizeof(network_info));
    send_data.insert(send_data.end(), reinterpret_cast<const char*>(&network_info),
                     reinterpret_cast<const char*>(&network_info) + sizeof(network_info));
    //return sendData(send_data, STATE_WRITE_NETWORK_INFO, msec_10);
    return false;
}

bool MemsCom::readNetworkInfo()
{
     return false;

    /*
    if (isConnected())
    {
        return writeCMD(NET_CMD_READ_PARAMETER, STATE_READ_NETWORK_INFO, 100);
    }
    else
    {
        return false;
    }*/

}

bool MemsCom::readViewingFieldParams(std::vector<double>& param)
{
    return false;

   #if 0
    if (isConnected() && writeCMD(NET_CMD_READ_VIEW_PARAMETER, STATE_READ_VIEWING_FIELD_PARAM, 100))
    {
        ViewingFieldParam vfp;
        memcpy(&vfp, reg_back_data_, sizeof(ViewingFieldParam));
        if (vfp.flag == VIEWING_FIELD_PARAM_FLAG)
        {
            std::vector<double>().swap(param);
            double temp = 0.0;
            for (std::size_t i = 0; i < VIEWING_FIELD_PARAM_SIZE; ++i)
            {
                temp = static_cast<double>((static_cast<uint16_t>(vfp.data[i].value[0]) << 8) +
                                           static_cast<uint16_t>(vfp.data[i].value[1]));
                temp /= 100.0;
                if (vfp.data[i].symbol)
                {
                    temp = 0.0 - temp;
                }
                param.push_back(temp);
            }
            return true;
        }
    }

    return false;

#endif
}

bool MemsCom::writeViewingFieldParams(const std::string& csv_path)
{
    // TODO
    // QFile csv_file(csv_path);
    // if (csv_file.open(QIODevice::ReadOnly))
    // {
    //   QTextStream stream(&csv_file);
    //   std::stringList string_list;
    //   while (!stream.atEnd())
    //   {
    //     string_list.push_back(stream.readLine());
    //   }
    //   csv_file.close();
    //   if (string_list.size() < VIEWING_FIELD_PARAM_SIZE)
    //   {
    //     return false;
    //   }
    //   std::vector<double> param;

    //   for (std::size_t i = 0; i < VIEWING_FIELD_PARAM_SIZE; ++i)
    //   {
    //     bool is_ok       = false;
    //     double temp_data = string_list.at(i).toDouble(&is_ok);
    //     if (is_ok)
    //     {
    //       param.push_back(temp_data);
    //     }
    //   }
    //   return writeViewingFieldParams(param);
    // }
    return false;
}

bool MemsCom::writeViewingFieldParams(const std::vector<double>& param)
{
    if (param.size() < VIEWING_FIELD_PARAM_SIZE)
    {
        return false;
    }

    ViewingFieldParam vfp;
    vfp.flag = VIEWING_FIELD_PARAM_FLAG;
    for (std::size_t i = 0; i < VIEWING_FIELD_PARAM_SIZE; ++i)
    {
        uint16_t temp_value = 0;
        if (param[i] < 0.0)
        {
            vfp.data[i].symbol = 1;
            temp_value         = static_cast<uint16_t>(0 - param[i] * 100.0);
        }
        else
        {
            vfp.data[i].symbol = 0;
            temp_value         = static_cast<uint16_t>(param[i] * 100.0);
        }
        vfp.data[i].value[0] = static_cast<uint8_t>(temp_value >> 8);
        vfp.data[i].value[1] = static_cast<uint8_t>(temp_value);
    }
    std::vector<char> send_data = frameHeadPack(NET_CMD_WRITE_VIEW_PARAMETER, sizeof(ViewingFieldParam));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&vfp),
                     reinterpret_cast<char*>(&vfp) + sizeof(ViewingFieldParam));
    //return (sendData(send_data, STATE_WRITE_SPLICE_PARAMS, 100));
}

bool MemsCom::fixRegister(const uint32_t msec_10)
{
    return writeCMD(NET_CMD_FPGA_TO_FLASH, STATE_FIX_REG, msec_10);
}

bool MemsCom::getIntensityData(const std::vector<uint32_t>& addrs2read,
                               std::vector<int32_t>& calib_data,
                               std::vector<char> & send_data)
{
    uint32_t reg_val  = 0;
    uint32_t reg_addr = 0;
    send_data = frameHeadPack(NET_CMD_GET_INTENSITY, (sizeof(reg_addr) + sizeof(reg_addr)) * (2 + addrs2read.size()));
    constexpr uint32_t REF_DATA_UPDATA_EN_ADDR = 0x83c00050;  // 当前通道更新使能, 2.0 的为 0x83C200B8
    reg_addr                                   = REF_DATA_UPDATA_EN_ADDR;
    reg_val                                    = static_cast<uint32_t>(0x1);
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_addr),
                     reinterpret_cast<char*>(&reg_addr) + sizeof(reg_addr));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_val),
                     reinterpret_cast<char*>(&reg_val) + sizeof(reg_val));

    reg_addr = REF_DATA_UPDATA_EN_ADDR;
    reg_val  = static_cast<uint32_t>(0x0);
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_addr),
                     reinterpret_cast<char*>(&reg_addr) + sizeof(reg_addr));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_val),
                     reinterpret_cast<char*>(&reg_val) + sizeof(reg_val));

    for (int i = 0, addr_size = addrs2read.size(); i < addr_size; ++i)
    {
        reg_addr = addrs2read[i];
        if (reg_addr > MAX_REG_ADDR || reg_addr < MIN_REG_ADDR)
        {
            return false;
        }
        reg_val = static_cast<uint32_t>(0x1000000);
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_addr),
                         reinterpret_cast<char*>(&reg_addr) + sizeof(reg_addr));
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&reg_val),
                         +reinterpret_cast<char*>(&reg_val) + sizeof(reg_val));
    }
    //if (sendData(send_data, STATE_READ_INTENSITY, msec_10))
    {
        calib_data.resize(addrs2read.size());
        for (int i = 0, addr_size = addrs2read.size(); i < addr_size; ++i)
        {
            memcpy(&calib_data[i], collect_back_data_ + 8 * (2 + i) + 4, sizeof(int32_t));
        }
        return true;
    }
    //else
    {
        return false;
    }
}

bool MemsCom::writeToFlash(const std::string& file_path, std::string& msg, const int update_type)
{
    if (file_path.find("bin", 0) != file_path.length() - 3)
    {
        msg = "firmware file must be bin file.\n";
        return false;
    }

    //if (0 == access(file_path.c_str(), F_OK))
	if(1)
    {
        FILE* file = NULL;
        file       = fopen(file_path.c_str(), "rb");

        if (file == NULL)
        {
            msg = "File open failed, please try it again\n";
            fclose(file);
            return false;
        }

        fseek(file, 0, SEEK_END);
        int len = ftell(file);
        rewind(file);

        if (0xA00000 < len || len < 0x400000)
        {
            msg = "file len must be 0x400000-0xA00000.\n";
            fclose(file);
            return false;
        }

        std::vector<char> send_data = frameHeadPack(NET_CMD_WRITE_FLASH, static_cast<uint32_t>(len) + 12);

        uint32_t flash_offset;
        if (update_type == 0)
        {
            flash_offset = 0x00000000;
        }
        else if (update_type == 1)
        {
            flash_offset = 0x00A00000;
        }
        else
        {
            msg = "update type only support 0 or 1.\n";
            fclose(file);
            return false;
        }

        //uint8_t buffer[len];
		//uint8_t buffer[1024*1024];
		uint8_t* buffer = new uint8_t[len];
        fread(buffer, 1, len, file);
        fclose(file);

        send_data.insert(send_data.end(), reinterpret_cast<char*>(&flash_offset),
                         reinterpret_cast<char*>(&flash_offset) + sizeof(flash_offset));

        uint32_t flashCheckSum = static_cast<uint32_t>(checkSum(buffer, len));
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&flashCheckSum),
                         reinterpret_cast<char*>(&flashCheckSum) + sizeof(flashCheckSum));

        uint32_t flashWriteSize = static_cast<uint32_t>(len);
        send_data.insert(send_data.end(), reinterpret_cast<char*>(&flashWriteSize),
                         reinterpret_cast<char*>(&flashWriteSize) + sizeof(flashWriteSize));

        send_data.insert(send_data.end(), reinterpret_cast<char*>(&buffer),
                         reinterpret_cast<char*>(&buffer) + sizeof(buffer));

        read_write_state_[STATE_WRITE_FLASH] = false;
        //if (sendData(send_data, STATE_WRITE_FLASH, 300))
        {
            uint32_t state;
            memcpy(&state, write_flash_back_data_, sizeof(state));
            if (state == 0xF1)
            {
                msg = "FW transport checksum error.\n";
                return false;
            }
            else if (state == 0xF2)
            {
                msg = "FW rlash write with checksum error.\n";
                return false;
            }
            else if (state == 0)
            {
                msg = "FW update success.\n";
                return true;
            }
            else
            {
                msg = "Update timeout,failed.\n";
                return true;
            }
        }
        //else
        {
            msg = "Update timeout,failed.\n";
            return true;
        }
    }
    else
    {
        msg = "Flie does not existed!\n";
        return false;
    }
}

bool MemsCom::writeCMD(const uint32_t cmd, const ReadWriteStateIndex index, uint32_t msec_10)
{
    std::vector<char> send_data = frameHeadPack(cmd, 0);
    //return sendData(send_data, index, msec_10);
    return false;
}


bool MemsCom::setConfigSendData(ConfigPara para, std::vector<char> & send_data, uint8_t & statusIdx)
{
    send_data = frameHeadPack(NET_CMD_WRITE_PARAMETER, sizeof(para));
    send_data.insert(send_data.end(), reinterpret_cast<char*>(&para), reinterpret_cast<char*>(&para) + sizeof(para)); 
    statusIdx = STATE_SET_ALL_CONFIG;
    read_write_state_[statusIdx] = false;
    return true;
}


// 在mems项目中 009是NET_CMD_WRITE_FPGA_MEM， 在helios中是NET_CMD_READ_CONFIG
bool MemsCom::getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx)
{
    // NET_CMD_WRITE_FPGA_MEM       = 0x009;  // NET_CMD_READ_CONFIG
    send_data = frameHeadPack(NET_CMD_READ_PARAMETER, 0);

    statusIdx = STATE_GET_ALL_CONFIG;
    read_write_state_[statusIdx] = false;
    return true;
}

bool MemsCom::getConfigResult(ConfigPara &para)
{
    memcpy(&para.hconfig, &reg_back_data_[0], sizeof(ConfigPara));

    return true;
}

bool MemsCom::CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx)
{

    return false;
}



//反射率标定
bool MemsCom::writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                         std::vector<char> & send_data,uint8_t & statusIdx)
{

    return false;

}


 bool MemsCom::getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal)
 {

 
     return false;
 }

bool MemsCom::SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx)
{
	return false;
}

 
 bool MemsCom::SetZeroAngle(s32 angle, std::vector<char> & send_data, uint8_t &statusIdx)
 {
     return false;
 }
 
 
bool MemsCom::GetZeroAngleSendData( std::vector<char> & send_data, uint8_t &statusIdx)
{

    return false;
}
 
bool  MemsCom::GetZeroAngleResult(s32 &angle)
{
    memcpy(&angle, reg_back_data_ , sizeof(s32)); 
    return true;
}

bool  MemsCom::SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles)
{
    return false;
}

bool  MemsCom::GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum)
{
	return false;
}

bool  MemsCom::GetChannelAngleResult(std::vector<float>& angles)
{
	return false;
}



/*
bool MemsCom::GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx)
{
    return false;

}

bool MemsCom::GetChannelAngleResult(std::vector<int32_t>& angles)
{
    uint16_t size = m_len / 4;     
    angles.resize(size);
    
    for (int i = 0; i < size; ++i)
    {       
        memcpy(&angles[i], reg_back_data_ + (4 * i), sizeof(int32_t));    
    }
    

    return true;

}

*/

