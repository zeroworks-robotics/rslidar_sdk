
#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include <chrono>         // std::chrono::seconds

//using namespace std::chrono_literals;


#pragma warning(disable:4996)

#include "tcp_com.h"



using namespace boost::asio;



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


u32 wasteTime(u32 count)
{
    u32 sum = 0;
    for(u32 k = 0; k < count; k++)
    {
        sum += k*count + k*3 + count*7;        

    }

    return sum;
}


static inline void msleepByBoost( std::size_t msec = 1000)
{

#if 0
    for(int k = 0; k < msec; ++k)
    {
        wasteTime(99);
    }


    // Get current time from the clock, using microseconds resolution
    const boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    // Get the time offset in current day
    const boost::posix_time::time_duration td = start.time_of_day();
    u64 ms = td.total_milliseconds();

    while(1)
    {
        wasteTime(99999);
        
        const boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
        // Get the time offset in current day
        const boost::posix_time::time_duration td2 = end.time_of_day();
        u64 ms2 = td2.total_milliseconds();

        if(ms2 - ms >= msec)
        {
            return ;
        }
    }
#endif


    auto start = std::chrono::high_resolution_clock::now(); //us
    std::chrono::duration<double, std::milli> elapsed;

    do {
        std::this_thread::yield();
        auto end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        
    } while ( elapsed.count() < msec);


}

static bool ping(const std::string& ip)
{
    // ping ip > abc.txt
    // find TTL
    return true;
}

TcpCom::TcpCom() : flag_thread_run_(false)
{
    m_usedCom = std::make_shared<LidarBase>();
    read_write_state_.fill(false);
    ptr_io_service_.reset(new boost::asio::io_service);
    ptr_deadline_timer_.reset(new deadline_timer(*ptr_io_service_));

    ptr_deadline_timer_->expires_at(boost::posix_time::pos_infin);

    checkDeadline();

    m_fp = fopen("tcpCom.txt", "a+");
}

TcpCom::~TcpCom()
{
    disconnect();
    fclose(m_fp);
}

bool TcpCom::connect(const std::string& ip, const uint16_t msop_port, const uint32_t sec)
{
    bool ping_pass = false;
    ip_            = ip;
    msop_port_     = msop_port;
    boost::system::error_code errCodes;
    for (int i = 0; i < 10; ++i)  // NOTE ping lidar will speed up lidar's connect
    {
        if (ping(ip))
        {
            ping_pass = true;
            break;
        }
    }
    if (!ping_pass)
    {
        std::cout << ip + " is not connectable. Please check it." << std::endl;
        return false;
    }
    if (nullptr == ptr_socket_ && nullptr !=ptr_io_service_)
    {
        ptr_socket_.reset(new ip::tcp::socket(*ptr_io_service_));
    }
    try
    {
        boost::system::error_code ec = boost::asio::error::would_block;
        boost::asio::ip::tcp::resolver::query query(ip, std::to_string(msop_port));
        boost::asio::ip::tcp::resolver::iterator iter = boost::asio::ip::tcp::resolver(*ptr_io_service_).resolve(query);

        ptr_deadline_timer_->expires_from_now(boost::posix_time::seconds(sec));
        boost::asio::async_connect(*ptr_socket_, iter, boost::lambda::var(ec) = boost::lambda::_1);

        do
        {
            ptr_io_service_->run_one(errCodes);
        }
        while (ec == boost::asio::error::would_block);

        // Determine whether a connection was successfully established. The
        // deadline actor may have had a chance to run and close our socket, even
        // though the connect operation notionally succeeded. Therefore we must
        // check whether the socket is still open before deciding if we succeeded
        // or failed.

        std::cout << "ec: " << ec.message() << std::endl;

        if (ec || !ptr_socket_->is_open())
        {
            //ptr_socket_->close();
            std::cout << "timeout to connect socket at host " << ip << " with port " << msop_port << std::endl;
            return false;
        }

        boost::asio::ip::tcp::socket::reuse_address ra(true);
        ptr_socket_->set_option(ra);
        ptr_socket_->set_option(ip::tcp::socket::keep_alive(true));
        //ptr_socket_->set_option(ip::tcp::socket::send_buffer_size(1));
        //ptr_socket_->set_option(ip::tcp::socket::receive_buffer_size(1));
        // ptr_socket_->set_option(ip::tcp::socket::receive_low_watermark(1));
        // ptr_socket_->set_option(ip::tcp::socket::send_low_watermark(1));
        ptr_socket_->set_option(ip::tcp::no_delay(true));

        flag_thread_run_.store(true);
        ptr_thread_.reset(new std::thread([this]()
        {
            slotReadTCPData();
        }));

        m_netOK = true;
        return true;
    }
    catch (boost::system::system_error& e)
    {
        std::cout << "failed to connect to " << ip << " at port " << msop_port << ": " << std::endl;
        std::cout << e.what() << std::endl;
        return false;
    }

}

bool TcpCom::disconnect()
{
    boost::system::error_code errCode;
    if (ptr_socket_ && flag_thread_run_)
    {
        // writeCMD(NET_CMD_TCP_END, STATE_TCP_END, 100);
        flag_thread_run_ = false;
        ptr_socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both,errCode);
        if (ptr_thread_->joinable())
        {
            ptr_thread_->join();
        }
        ptr_thread_.reset();
        ptr_socket_->close(errCode);
        ptr_socket_.reset();
        ptr_io_service_->stop();
		ptr_io_service_.reset();
    }

    return true;
}

bool TcpCom::isConnected() const
{
    bool isOpen = false;
    if(nullptr != ptr_socket_)
    {
        bool isTrue = ptr_socket_->is_open();    
        isOpen = (isTrue && m_netOK) ? true : false;
    }
    return isOpen;    // TODO
}

int TcpCom::SetLidarType(LidarType type)
{
    int ret = -1;
    m_lidarType = type;

    if( nullptr != m_usedCom )
    {
        m_usedCom.reset();
    }
    
    switch(type)
    {
    case RS_RUBY4:
        ret = 0;
        m_usedCom = std::make_shared<Ruby4Com>();
        break;

    case RS_AIRY:
    	ret = 0;
        m_usedCom = std::make_shared<AiryCom>();
        break;
    
    case RS_BP3:
    case RS_RUBY3:
    case RS_RUBY_LITE:
    case RS_HELIOS:
    case RS_ROCK:
        ret = 0;
        m_usedCom = std::make_shared<HeliosCom>();
        break;
    
    case RS_M1:
    case RS_M1P:
        m_usedCom = std::make_shared<MemsCom>();
        ret = 0;
        break;
        
    default:
        m_usedCom = std::make_shared<LidarBase>();
        ret = -1;
        break;
    }
    return ret;
}

void TcpCom::slotReadTCPData()
{
    constexpr std::size_t MAX_READ_BUFFER = 1248;
    char* precv_buffer                    = reinterpret_cast<char*>(malloc(MAX_READ_BUFFER));
    while (flag_thread_run_.load())
    {
        boost::system::error_code ec;
        std::size_t read_size = ptr_socket_->read_some(boost::asio::buffer(precv_buffer, MAX_READ_BUFFER), ec);
        if(boost::asio::error::eof == ec || boost::asio::error::connection_reset == ec)
        {
            m_netOK = false;
            break;
        }
        if (ec && ec != boost::asio::error::eof)
        {
            std::cout << boost::system::system_error(ec).what() << std::endl;
            continue;
        }
        
        read_data_buffer_.insert(read_data_buffer_.end(), precv_buffer, precv_buffer + read_size);

        m_usedCom->ParseRecvTcpData(read_data_buffer_);

        /*
        read_data_buffer_.erase(
            read_data_buffer_.begin(),
            read_data_buffer_.begin() + static_cast<int32_t>(sizeof(frame_head) + frame_head.length));
        */

    }
    free(precv_buffer);
}



bool TcpCom::writeRegData(const std::vector<uint32_t>& reg_addr,
                          const std::vector<int32_t>& reg_val,
                          const uint32_t msec)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->writeRegData(reg_addr, reg_val, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx, msec);
    }
    else
    {
        return false;
    }
}

// 第一个参数类型不同。 一个是vector, 另一个是uint32_t
bool TcpCom::writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val, const uint32_t msec)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->writeRegData(start_reg_addr, reg_val, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx, msec);
    }
    else
    {
        return false;
    }
}

bool TcpCom::readRegData(const std::vector<uint32_t>& addrs2read,
                         std::vector<int32_t>& reg_val,
                         const uint32_t msec)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->getReadRegSendData(addrs2read, reg_val, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, msec) )
        {
            m_usedCom->getReadRegResult(addrs2read, reg_val);
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}

bool TcpCom::readRegData(const uint32_t start_reg_addr,
                         const uint32_t reg_number,
                         std::vector<int32_t>& reg_val,
                         const uint32_t msec)
{
    //return m_usedCom->readRegData(start_reg_addr, reg_val, msec);
}

bool TcpCom::writeCalibrateReflectData(std::vector<uint32_t>& addrs2read,std::vector<uint32_t>& reg_val,
                                       std::vector<uint32_t>& vecRetRegVal, const uint32_t msec)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->writeCalibrateReflectData(addrs2read, reg_val, send_data, statusIdx) )
    {
        //const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, msec) )
        {
            m_usedCom->getReadCaliRefResult(addrs2read, vecRetRegVal);
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }
}


bool TcpCom::GetPatchVersion(u32 &ver)
{
   bool rst = false;
   uint8_t statusIdx = 0;
   std::vector<char> send_data;

   if( m_usedCom->getPatchVerSendData(send_data, statusIdx) )
   {
	  if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 1000) )
	  {
		  if( m_usedCom->getPatchVerResult(ver) )
		  {
			  
			  rst = true;
		  } 		  
	  }
   }

   return rst;

}
									   
									   

bool TcpCom::GetConfigVersion(char ver[32])
{
	bool rst = false;
	uint8_t statusIdx = 0;
	std::vector<char> send_data;

	if( m_usedCom->getConfigVerSendData(send_data, statusIdx) )
	{
	   if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 1000) )
	   {
		   if( m_usedCom->getConfigVerResult(ver) )
		   {
			   
			   rst = true;
		   }		   
	   }
	}

	return rst;

}


bool TcpCom::GetLidarConfig(ConfigPara &para)
{
    bool rst = false;
    uint8_t statusIdx = 0;
    std::vector<char> send_data;

    if( m_usedCom->getConfigSendData(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 2000) )
        {
            if( m_usedCom->getConfigResult(para) )
            {
                
                rst = true;
            }           
        }
    }
    
    return rst;

}

bool TcpCom::setMotorDir(u8 dir){
	
	bool rst = false;
    uint8_t statusIdx = 0;
    std::vector<char> send_data;

    if( m_usedCom->setMotorDir(dir, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 400) )
        {                
            rst = true;                      
        }
    }
    
    return rst;    
}

bool TcpCom::getMotorDir(u8 &dir)
{
	
	bool rst = false;
	uint8_t statusIdx = 0;
	std::vector<char> send_data;

	if( m_usedCom->getMotorDir(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 300) )
		{				 
			return m_usedCom->getMotorResult(dir);						
		}
	}
	
	return rst;    
}


bool TcpCom::setMode(u8 mode)
{
	
	bool rst = false;
    uint8_t statusIdx = 0;
    std::vector<char> send_data;

    if( m_usedCom->setMode(mode, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 4000) )
        {                
            rst = true;                      
        }
    }
    
    return rst;    
}


bool TcpCom::getMode(u8 &mode)
{
	
	bool rst = false;
    uint8_t statusIdx = 0;
    std::vector<char> send_data;

    if( m_usedCom->getMode(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 300) )
        {                
            return m_usedCom->getModeResult(mode);                      
        }
    }
    
    return rst;    
}


bool TcpCom::SetLidarConfig(ConfigPara para)
{
    bool rst = false;
    uint8_t statusIdx = 0;
    std::vector<char> send_data;

    if( m_usedCom->setConfigSendData(para, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 5000) )
        {                
            rst = true;                      
        }
    }
    
    return rst;

}


void TcpCom::getNetworkInfo(NetworkInfo& network_info) const
{
    //network_info = net_work_info_;
}



bool TcpCom::readNetworkInfo()
{

    return false;

}

bool TcpCom::readViewingFieldParams(std::vector<double>& param)
{
    return false;
}

bool TcpCom::writeViewingFieldParams(const std::string& csv_path)
{
    return false;
}

bool TcpCom::writeViewingFieldParams(const std::vector<double>& param)
{
    return false;
}

bool TcpCom::fixRegister(const uint32_t msec_10)
{
    return false;
}

bool TcpCom::getIntensityData(const std::vector<uint32_t>& addrs2read,
                              std::vector<int32_t>& calib_data,
                              const uint32_t msec_10)
{
    return false;

}

bool TcpCom::writeToFlash(const std::string& file_path, std::string& msg, const int update_type)
{
    return false;
}

bool TcpCom::update(u8 type, std::vector<char> file_data)
{
	uint8_t statusIdx = 0;
	std::vector<char> send_data;

	fprintf(m_fp, "in update(), type = %d\n", type);

	if( RS_RUBY4 == m_lidarType || RS_AIRY == m_lidarType)  // ruby4, bp4
	{
		
		uint32_t len = file_data.size(), k, idx;
	    uint32_t count = len / 1024;
	    uint32_t left = len % 1024;
	    uint8_t  buf[8] = {0};
	    bool rst;
        std::vector<char> data;

		printf("TcpCom::update(), len = %d, count = %d, please wait\n", len, count);

		
		//printf("test TcpCom::update(), type = %d\n", type);

        m_usedCom->sendUpdateRequest(type, len, send_data, statusIdx) ;
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx, 30*1000 ) )
        {
            for(k = 0; k < count; ++k)
            {    
            	idx = k + 1;
            	memcpy(buf, &idx, 4);
            	data.clear();
            	data.insert(data.end(), buf, buf+4);
                data.insert(data.end(), file_data.begin() + k *1024, file_data.begin() + (k+1) *1024);
                m_usedCom->sendUpdateData(type,data, send_data, statusIdx) ;
                if( !sendData(send_data, (const ReadWriteStateIndex)statusIdx ) )
                {                
					fprintf(m_fp, "in update(), line = %d, sendData() return false.\n", __LINE__);
                    return false;
                }

                if( 0 == (k%10) )
	            {
					printf(".");
					fflush(stdout);
	            }
	            
            }

            if(left != 0 )
            {
            	idx = k + 1;
                memcpy(buf, &idx, 4);
            	data.clear();
            	data.insert(data.end(), buf, buf+4);
            	
                data.insert(data.end(), file_data.begin() + k *1024, file_data.end());
                m_usedCom->sendUpdateData(type, data, send_data, statusIdx) ;
                if( !sendData(send_data, (const ReadWriteStateIndex)statusIdx ) )
                {                
                    fprintf(m_fp, "in update(), line = %d, sendData() return false.\n", __LINE__ );
                    return false;
                }
            }

            //C3 CHECKSUM
            uint32_t sum = 0;
            for(k = 0; k < len; ++k)
            {
                sum += (uint8_t)file_data[k];
            }

            m_usedCom->sendUpdateCheckSum(type, sum, send_data, statusIdx) ;
            if( !sendData(send_data, (const ReadWriteStateIndex)statusIdx ) )
            {
                return false;
            }

			printf("\n");

            return true;
        }
        else
        {        	
            fprintf(m_fp, "in update(), line = %d, sendData() return false.\n", __LINE__ );
            return false;
        }   

	}	

	else if((type < NET_CMD_RUBY4_UPDATE) || (type == NET_CMD_CONFIG_UPDATE))
	{
		// just for helios
		if( m_usedCom->update(type, file_data, send_data, statusIdx) )
		{
			int len = send_data.size(), k;
	        int count = len / (10*1024);
	        k = len % (10*1024);
			printf("TcpCom::update(), len = %d, count = %d, left = %d\n", len, count, k);

			for(k = 0; k < count; ++k)
			{
				std::vector<char> data;
				data.insert(data.end(), send_data.begin() + k *10*1024, send_data.begin() + (k+1) *10*1024);
	            sendData(data, (const ReadWriteStateIndex)statusIdx, 60);
	            //printf("TcpCom::update(), k = %d\n", k);
	            if( 0 == (k%3) )
	            {
					printf(".");
					fflush(stdout);
	            }
	        }

	        if( (len % (10*1024) ) != 0 )
	        {
	        	//printf("TcpCom::update(), the last pkg, count = %d\n", count);
				std::vector<char> data;
				data.insert(data.end(), send_data.begin() + count *10*1024, send_data.end());
	            sendData(data, (const ReadWriteStateIndex)statusIdx, 30);
	            printf("*");
	            fflush(stdout);
			}

	        printf("\n");
	        return true;
		}
		else
		{
		   return false;
		}

	}


}

bool TcpCom::getUpdateStatus(u8 &type, s32 &status)
{
	uint8_t statusIdx = 0;
	std::vector<char> send_data;
	if( m_usedCom->queryUpdateStatus(send_data, statusIdx) ) // just for ruby4/bp4
	{
    	sendData(send_data, (const ReadWriteStateIndex)statusIdx ); 
    }

	return m_usedCom->getUpdateStatus(type, status);
}




bool TcpCom::sendData(const std::vector<char>& send_data, const ReadWriteStateIndex index, uint32_t msec)
{
    if (!flag_thread_run_)
    {
        std::cout << "Please connect lidar first" << std::endl;
        return false;
    }

    // boost::asio::write(*ptr_socket_.get(), boost::asio::buffer(send_data));
    // ptr_socket_->write(boost::asio::buffer(send_data));
    ptr_deadline_timer_->expires_from_now(boost::posix_time::milliseconds(msec));
    boost::system::error_code ec = boost::asio::error::would_block;

    //async_write
    boost::asio::async_write(*ptr_socket_, boost::asio::buffer(send_data), boost::lambda::var(ec) = boost::lambda::_1);


    for (int k = 0; k < msec/10 +1; k++)
    {
        if ( m_usedCom->checkRecvCmd(index))
        {
            return true;
        }

        msleepByBoost(10);
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        //boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(5));  
        //std::this_thread::sleep_for( std::chrono::seconds(1) );
    }

    return false;
}

void TcpCom::checkDeadline()
{
    if (ptr_deadline_timer_->expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        boost::system::error_code ignored_ec;
        ptr_socket_->close(ignored_ec);
        ptr_deadline_timer_->expires_at(boost::posix_time::pos_infin);
    }

    ptr_deadline_timer_->async_wait(std::bind(&TcpCom::checkDeadline, this));
}

bool TcpCom::setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec )
{
    return false;

}

int TcpCom::SetLidarSn(u8 sn[6])
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarSn(sn, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}


int TcpCom::SetLidarNetInfo(NetParam_st netInfo)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarNetInfo(netInfo, send_data, statusIdx) )
    {
        sendData(send_data, (const ReadWriteStateIndex)statusIdx);
        return true;
    }
    else
    {
        return false;
    }
}

bool TcpCom::rebootLidar()
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->rebootLidar(send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }

}


//  ethName="/dev/eth0"
void TcpCom::rebootNoLink(char *ethName)
{

#if 1

	// 1. 创建通信的套接字
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1)
	{	
		perror("socket");
		exit(0);
	}

	// 2. 设置广播属性
	int opt  = 1;
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
	char buf[1024] = {"reboot"};
	struct sockaddr_in cliaddr;
	int len = sizeof(cliaddr);
	cliaddr.sin_family = AF_INET;
	  /* 指定接口 绑定接口 */
	struct ifreq nif;
	strcpy(nif.ifr_name, ethName);
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif)) <  0) 
	{
		close(fd);
		printf("bind interface fail, errno: %d \r\n", errno);
		//return -1;
	}
	//cliaddr.sin_addr.s_addr = inet_addr("192.168.1.102");
	cliaddr.sin_port = htons(56789); //广播消息
	//inet_pton(AF_INET, "192.168.200.255", &cliaddr.sin_addr.s_addr);//
	inet_pton(AF_INET, "255.255.255.255", &cliaddr.sin_addr.s_addr);
	//使用受限广播也可以正常通讯// 3. 通信

	// 数据广播
	sendto(fd, buf, strlen(buf)+1, 0, (struct sockaddr*)&cliaddr, len);
	// printf("发送的广播的数据: %s\n", buf);

	close(fd);

	/*
	//一个服务的类，给这个UDP通信初始化
	io_service io_serviceA;
	//通过服务给这个UDP通信初始化
	ip::udp::socket udp_socket(io_serviceA, ip::udp::endpoint(ip::udp::v4(),10061 ) );
	udp_socket.set_option(socket_base::broadcast(true));
	//设置连接的IP还有端口
	ip::udp::endpoint local_add(ip::address::from_string("255.255.255.255"), 56789);
	//添加协议
	//udp_socket.open(local_add.protocol());
	char sendstr[1024] = { "reboot" };//字符串

	udp_socket.send_to(boost::asio::buffer("reboot"), local_add);
	*/
#endif

}

int TcpCom::SetLidarSyncMode(u8 mode)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarSyncMode(mode, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}


int TcpCom::SetLidarWaveMode(u8 mode)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarWaveMode(mode, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}


int TcpCom::SetLidarFov(u16 start, u16 end)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarFov(start, end, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}


int TcpCom::SetMotorPhaseLockCfg(u16 angle)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetMotorPhaseLockCfg(angle, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

bool TcpCom::SetMotorStopAngle(u16 angle)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetMotorStopAngle(angle, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }


}


int TcpCom::SetMotorSpeed(u8 level) // 1 ---300; 2---600; 3---1200
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetMotorSpeed(level, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

//int SetLidarTrigerAngle(u16 angle);
int TcpCom::SetLidarWorkMode(u8 mode)  // 0  standby, 1 normal
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetLidarWorkMode(mode, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

int TcpCom::CtrlCodeWheelCalibrate(u8 ctrl)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->CtrlCodeWheelCalibrate(ctrl, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

int TcpCom::WriteTopFlash(u32 startAddr, char *file)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->WriteTopFlash(startAddr, file, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

int TcpCom::ReadTopFlash(u32 startAddr, char *file)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->ReadTopFlash(startAddr, file, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

int TcpCom::CtrlEyeSafe(u16 on)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->CtrlEyeSafe(on, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

bool TcpCom::SetZeroAngle(s32 angle)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetZeroAngle(angle, send_data, statusIdx) )
    {
        return sendData(send_data, (const ReadWriteStateIndex)statusIdx);
    }
    else
    {
        return false;
    }
}

bool TcpCom::GetZeroAngle(s32 &angle)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->GetZeroAngleSendData(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return m_usedCom->GetZeroAngleResult(angle);
        } 
        return false;
    }
    else
    {
        return false;
    }
}

bool TcpCom::GetChannelAngle(std::vector<float>& angles, u16 inAngleNum)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->GetChannelAngleSendData(send_data, statusIdx, inAngleNum) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return m_usedCom->GetChannelAngleResult(angles);
        } 
        return false;
    }
    else
    {
        return false;
    }
}

bool TcpCom::SetChannelAngle(std::vector<float>  angles)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->SetChannelAngleSendData(send_data, statusIdx, angles) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        } 
        return false;
    }
    else
    {
        return false;
    }
}


bool TcpCom::readDaemonLog(std::vector<char> & recvData)
{
	recvData.clear();
	uint8_t statusIdx = 0;
	std::vector<char> send_data, tmpData;
	int firstPkg = 0, getData = false;
	unsigned short int totalPkg, curPkg;
	unsigned char ch1, ch2, ch3, ch0;
	if( m_usedCom->readDaemonLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			while(1)
			{
				getData = false;
				for(int k = 0; k < 100; ++k)
				{
					tmpData.clear();
					if( m_usedCom->getDaemonData(tmpData) )
					{
						getData = true;
						break;
					}
					else
					{				
						msleepByBoost(2);;
					}
				}

				if( !getData)
				{
					return false; // 200ms timeout
				}
				else
				{
					std::vector<char> realData(tmpData.begin() + 4, tmpData.end());
					recvData.insert(recvData.end(), realData.begin(), realData.end());
					
					ch0 = (unsigned char)tmpData[0];
					ch1 = (unsigned char)tmpData[1];

					ch2 = (unsigned char)tmpData[2];
					ch3 = (unsigned char)tmpData[3];

					// ch0, ch1为总包数， ch2, ch3为当前包数
					if( (ch0 == ch2) && (ch1 == ch3) ) 
					{
						break;
					}
					
				}

			}
		
			return true;
			
		} 
		return false;
	}
	else
	{
		return false;
	}

}


bool TcpCom::deleteDaemonLog()
{
	uint8_t statusIdx = 0;
	std::vector<char> send_data;
	if( m_usedCom->deleteDaemonLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			return true;
		} 
		return false;
	}
	else
	{
		return false;
	}
}


bool TcpCom::readConfigFile(std::vector<char> & recvData)
{
	recvData.clear();
	uint8_t statusIdx = 0;
	std::vector<char> send_data, tmpData;
	int firstPkg = 0, getData = false;
	unsigned short int totalPkg, curPkg;
	unsigned char ch1, ch2, ch3, ch0;
	if( m_usedCom->readConfigFile(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			while(1)
			{
				getData = false;
				for(int k = 0; k < 100; ++k)
				{
					tmpData.clear();
					if( m_usedCom->getConfigFileData(tmpData) )
					{
						getData = true;
						break;
					}
					else
					{				
						msleepByBoost(2);;
					}
				}

				if( !getData)
				{
					return false; // 200ms timeout
				}
				else
				{
					std::vector<char> realData(tmpData.begin() + 4, tmpData.end());
					recvData.insert(recvData.end(), realData.begin(), realData.end());
					
					ch0 = (unsigned char)tmpData[0];
					ch1 = (unsigned char)tmpData[1];

					ch2 = (unsigned char)tmpData[2];
					ch3 = (unsigned char)tmpData[3];

					// ch0, ch1为总包数， ch2, ch3为当前包数
					if( (ch0 == ch2) && (ch1 == ch3) ) 
					{
						break;
					}
					
				}

			}
		
			return true;
			
		} 
		return false;
	}
	else
	{
		return false;
	}

}


bool TcpCom::readEventLog(std::vector<char> & recvData)
{
	recvData.clear();
	uint8_t statusIdx = 0;
	std::vector<char> send_data, tmpData;
	int firstPkg = 0, getData = false;
	unsigned short int totalPkg, curPkg;
	unsigned char ch1, ch2, ch3, ch0;
	if( m_usedCom->readEventLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			while(1)
			{
				getData = false;
				for(int k = 0; k < 100; ++k)
				{
					tmpData.clear();
					if( m_usedCom->getEventLogData(tmpData) )
					{
						getData = true;
						break;
					}
					else
					{				
						msleepByBoost(2);;
					}
				}

				if( !getData)
				{
					return false; // 200ms timeout
				}
				else
				{
					std::vector<char> realData(tmpData.begin() + 4, tmpData.end());
					recvData.insert(recvData.end(), realData.begin(), realData.end());
					
					ch0 = (unsigned char)tmpData[0];
					ch1 = (unsigned char)tmpData[1];

					ch2 = (unsigned char)tmpData[2];
					ch3 = (unsigned char)tmpData[3];

					// ch0, ch1为总包数， ch2, ch3为当前包数
					if( (ch0 == ch2) && (ch1 == ch3) ) 
					{
						break;
					}
					
				}

			}
		
			return true;
			
		} 
		return false;
	}
	else
	{
		return false;
	}

}

bool TcpCom::deleteEventLog()
{
	uint8_t statusIdx = 0;
	std::vector<char> send_data;
	if( m_usedCom->deleteEventLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			return true;
		} 
		return false;
	}
	else
	{
		return false;
	}

}

bool TcpCom::readMonitorLog(std::vector<char> & recvData)
{
	recvData.clear();
	uint8_t statusIdx = 0;
	std::vector<char> send_data, tmpData;
	int firstPkg = 0, getData = false;
	unsigned short int totalPkg, curPkg;
	unsigned char ch1, ch2, ch3, ch0;

	if( m_usedCom->readMonitorLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{
			while(1)
			{
				getData = false;
				for(int k = 0; k < 100; ++k)
				{
					if( m_usedCom->getMonitorLogData(tmpData) )
					{
						getData = true;
						break;
					}
					else
					{				
						msleepByBoost(2);
					}
				}

				if( !getData)
				{
					return false; // 200ms timeout
				}
				else
				{
					std::vector<char> realData(tmpData.begin() + 4, tmpData.end());
					recvData.insert(recvData.end(), realData.begin(), realData.end());
					
					ch0 = (unsigned char)tmpData[0];
					ch1 = (unsigned char)tmpData[1];

					ch2 = (unsigned char)tmpData[2];
					ch3 = (unsigned char)tmpData[3];

                    printf("%d, %d, %d, %d\n", ch0, ch1, ch2, ch3);

					// ch0, ch1为总包数， ch2, ch3为当前包数
					if( (ch0 == ch2) && (ch1 == ch3) ) 
					{
						break;
					}
					
				}

			}
		
			return true;
			
		} 
		return false;
	}
	else
	{
		return false;
	}

}

bool TcpCom::restartMonitorLog()
{
	uint8_t statusIdx = 0;
	std::vector<char> send_data;
	if( m_usedCom->restartMonitorLog(send_data, statusIdx) )
	{
		if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
		{			
			return true;
		} 
		return false;
	}
	else
	{
		return false;
	}

}


bool TcpCom::getMonitorValue(float value[64])
{
	uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->getMonitorSendData(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            m_usedCom->getMonitorResult(value);
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }


}


bool TcpCom::getImuParams(ImuParam &getParams)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->getImuParamsSendData(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            m_usedCom->getImuParamsResult(getParams);
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }


}


bool TcpCom::setImuParams(ImuParam setParams)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setImuParamsSendData(setParams, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}

bool TcpCom::getSupplementParams(NetInfo2 &getParams)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->getSupplementParamsSendData(send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
        	return m_usedCom->getSupplementParamsResult(getParams);
           
        }

        return false;
    }
    else
    {
        return false;
    }

}

bool TcpCom::setSomeSupplementParams(NetInfoSet setParams)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setSomeSupplementParamsSendData(setParams, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}
bool TcpCom::setReflectEnhance(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setReflectEnhanceSendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}
bool TcpCom::setGpsBaud(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setGpsBaudSendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}

bool TcpCom::setTrailFilter(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setTrailFilterSendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}
bool TcpCom::setRainBlockDetectDistance(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setRainBlockDetectDistanceSendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}
bool TcpCom::setRainDetectSensitivity(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setRainDetectSensitivitySendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}
bool TcpCom::setBlockDetectSensitivity(u8 level)
{
    uint8_t statusIdx = 0;
    std::vector<char> send_data;
    if( m_usedCom->setBlockDetectSensitivitySendData(level, send_data, statusIdx) )
    {
        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
        {
            return true;
        }

        return false;
    }
    else
    {
        return false;
    }

}


// 解析16进制字符串为无符号整数
uint32_t TcpCom::hexStrToUInt(const char *str) 
{
    uint32_t result = 0;
    sscanf(str, "%x", &result);
    return result;
}

bool TcpCom::fixRegsCfg(char *fileName)
{
	if(m_lidarType != RS_AIRY)
	{
		printf("type is not RS_AIRY\n");
		return false;
	}

	
    FILE*file;
    file = fopen(fileName,"rt");//     
    if(NULL == file)
    {   
        printf("Failed in fixRegsCfg() function, fopen() fileName = %s  failed.\n", fileName);
        return false;
    }
	else
    {
		//printf("open file OK\n");
    } 

    bool finalRst = true;

	char buf[2048] = {0};
	int len = 0;
	uint16_t total459RegNum = 0, totalTopRegNum = 0, totalBotRegNum = 0;
	uint16_t reg459Count = 0, regTopCount = 0, regBotCount = 0;
	uint16_t pkg459Num = 0, pkgTopNum = 0, pkgBotNum = 0;
	uint16_t zero = 0;

	char line[1024];
	std::vector<char> cmdVec;    
	uint8_t statusIdx = 0;
    std::vector<char> send_data;
	
	while (fgets(line, sizeof(line), file) != NULL) 
	{
		//printf("read file line = %s\n", line);
		// 去除行末换行符
		char *nl = strchr(line, '\n');
		if (nl) 
		{
			*nl = '\0';
		}
		
		// 去除行首和行尾空白字符
		char *start = line;
	/*	while (isspace((unsigned char)*start)) 
			start++;
		
		if (*start == '\0')
			continue; // 空行
	*/		
			
		//printf("after process  file line = %s\n", line);
		// 处理寄存器总数行
		if (strstr(start, "bottom_regs") != NULL) 
	    {
			char *value = strchr(start, '=');
			if (value != NULL) 
			{
				value++; // 跳过'='
				totalBotRegNum = (uint16_t)strtoul(value, NULL, 10);
				printf("get bottom_regs, num is %d\n", totalBotRegNum);
				buf[0] = 0xE0;
				buf[1] = 0x16;
				len = 8;
				if (totalBotRegNum == 0) 
				{
					memcpy(&buf[2], &zero, 2);
					memcpy(&buf[4], &zero, 2);
					memcpy(&buf[6], &zero, 2);
					printf("send buf with 8 bytes\n");
					//sendDataToClient(buf, 8);
					//usleep(200 * 1000); // 200毫秒
					cmdVec.clear();
					cmdVec.insert(cmdVec.end(), buf, buf +8);
					if( m_usedCom->setFixBotRegSendData(cmdVec, send_data, statusIdx) )
				    {
				        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
				        {
				            //return true;
				            printf("recv fix bot reg ack, OK \n");
				        }
				        else
				        {
				        	fclose(file);
							return false;	
				        }
				   }
				   else
				   {
				   		fclose(file);
						return false;
				   }				        
									
				}
			}
		} 
		else if (strstr(start, "top_regs") != NULL) 
		{
			char *value = strchr(start, '=');
			if (value != NULL) 
			{
				value++;
				totalTopRegNum = (uint16_t)strtoul(value, NULL, 10);
				buf[0] = 0xE0;
				buf[1] = 0x15;
				len = 8;
				if (totalTopRegNum == 0) 
				{
					memcpy(&buf[2], &zero, 2);
					memcpy(&buf[4], &zero, 2);
					memcpy(&buf[6], &zero, 2);
					//sendDataToClient(buf, 8);
					//usleep(200 * 1000);
					cmdVec.clear();
					cmdVec.insert(cmdVec.end(), buf, buf +8);
					if( m_usedCom->setFixTopRegSendData(cmdVec, send_data, statusIdx) )
				    {
				        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
				        {
				            //return true;
				            printf("recv fix bot reg ack, OK \n");
				        }
				        else
				        {
				        	fclose(file);
							return false;	
				        }
				   }
				   else
				   {
				   		fclose(file);
						return false;
				   }
				      
				}
			}
		} 
		else if (strstr(start, "459_regs") != NULL) 
		{
			//printf("get 459_regs\n");
			char *value = strchr(start, '=');
			if (value != NULL)
			{
				value++;
				total459RegNum = (uint16_t)strtoul(value, NULL, 10);
				buf[0] = 0xE0;
				buf[1] = 0x13;
				len = 8;
				
				char qstr[256];
				snprintf(qstr, sizeof(qstr), "total459RegNum = %u", total459RegNum);
			//	printf(qstr);
				
				if (total459RegNum == 0) 
				{
					memcpy(&buf[2], &zero, 2);
					memcpy(&buf[4], &zero, 2);
					memcpy(&buf[6], &zero, 2);
					//sendDataToClient(buf, 8);
					//usleep(200 * 1000);
					cmdVec.clear();
					cmdVec.insert(cmdVec.end(), buf, buf +8);
					if( m_usedCom->setFix459RegSendData(cmdVec, send_data, statusIdx) )
				    {
				        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
				        {
				            //return true;
				            printf("recv fix 459 reg ack, OK \n");
				        }
				        else
				        {
				        	fclose(file);
							return false;	
				        }
				   }
				   else
				   {
				   		fclose(file);
						return false;
				   }

				      
				}
			}
		}
		
		// 分割行数据
		char *token, *saveptr;
		char *line_copy = line;
		int token_count = 0;
		char *tokens[4] = {0};
		
		token = strtok_r(line_copy, "_", &saveptr);
		while (token != NULL && token_count < 4) 
		{
			tokens[token_count++] = token;
			token = strtok_r(NULL, "_", &saveptr);
		}
		//free(line_copy);
	
		//printf("token_count = %d, tokens[0] = %s\n", token_count, tokens[0]);
	
		// 处理459寄存器写入
		if (token_count == 3 && strcmp(tokens[0], "W") == 0) 
		{
			uint32_t regAddr = hexStrToUInt(tokens[1]);
			memcpy(&buf[len], &regAddr, 4);
			len += 4;
			
			uint32_t regVal_write = hexStrToUInt(tokens[2]);
			memcpy(&buf[len], &regVal_write, 4);
			len += 4;
			
			reg459Count++;
		//	printf("get 459_regs , regBotcount = %d\n", reg459Count);
			
			if ((reg459Count >= 100) || ((100 * pkg459Num + reg459Count) >= total459RegNum)) 
			{
				//m_wr459OK = false;
				buf[0] = 0xE0;
				buf[1] = 0x13;
				memcpy(&buf[2], &total459RegNum, 2);
				memcpy(&buf[4], &pkg459Num, 2);
				memcpy(&buf[6], &reg459Count, 2);
				//sendDataToClient(buf, len);
				//usleep(200 * 1000);	
				
				printf("send 459 regs to fix. count = %d, pkgCount = %d\n", reg459Count, pkg459Num);

				cmdVec.clear();
				cmdVec.insert(cmdVec.end(), buf, buf +len);
				if( m_usedCom->setFix459RegSendData(cmdVec, send_data, statusIdx) )
			    {
			        if( sendData(send_data, (const ReadWriteStateIndex)statusIdx), 15*1000 )
			        {
			            //return true;
			            printf("recv fix bot reg ack, OK \n");
			        }
			        else
			        {
			        	fclose(file);
						return false;	
			        }
			   }
			   else
			   {
			   		fclose(file);
					return false;
			   }				      
								
				pkg459Num++;
				reg459Count = 0;
				len = 8;
				memset(buf, 0, sizeof(buf));

				/*
				// 等待写入完成
				for (int k = 0; k < 150; ++k) 
				{
					usleep(100 * 1000);
					// 实际应用中应检查真实的状态
					if (m_wr459OK) break;
				}
				*/
				
				char qstr[256];
				snprintf(qstr, sizeof(qstr), "total459RegNum = %u, pkg459Num = %u, reg459Count = %u\n", 
						total459RegNum, pkg459Num, reg459Count);
			//	printf(qstr);

				/*
				if (!m_wr459OK) 
				{
					if (m_defualtFile) 
					{
						printf("Flash更新: 写459寄存器超时\n");
					}
					finalRst = false;
					printf("提示: 写459寄存器超时\n");
				}
				*/
			}
			
			if ((100 * pkg459Num + reg459Count) >= total459RegNum) 
			{
				printf("wait 3 seconds for fixing 459 regs  \n");
				msleepByBoost(3000);
			/*
				usleep(3000 * 1000); // 3秒
				if (m_defualtFile) 
				{
					printf("Flash更新: 完成 459寄存器 固化\n");
				}*/
			}
		}
		// 处理底板寄存器写入
		else if (token_count == 4 && strcmp(tokens[0], "B") == 0)
		{
			uint32_t regAddr = hexStrToUInt(tokens[2]);
			memcpy(&buf[len], &regAddr, 4);
			len += 4;
			
			uint32_t regVal_write = hexStrToUInt(tokens[3]);
			memcpy(&buf[len], &regVal_write, 4);
			len += 4;
			
			regBotCount++;
			
		//	printf("get bottom_regs , regBotcount = %d\n", regBotCount);
			if ((regBotCount >= 100) || ((100 * pkgBotNum + regBotCount) >= totalBotRegNum)) 
			{
				//m_wrBotRegsOK = false;
				buf[0] = 0xE0;
				buf[1] = 0x16;
				memcpy(&buf[2], &totalBotRegNum, 2);
				memcpy(&buf[4], &pkgBotNum, 2);
				memcpy(&buf[6], &regBotCount, 2);
				//sendDataToClient(buf, len);
				//usleep(200 * 1000);
				
				printf("send bot regs count = %d, pkgCount = %d\n", regBotCount, pkgBotNum);
				
				 cmdVec.clear();
				 cmdVec.insert(cmdVec.end(), buf, buf +len);
				 if( m_usedCom->setFixBotRegSendData(cmdVec, send_data, statusIdx) )
				 {
					 if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
					 {
						 //return true;
						 printf("recv fix bot reg ack, OK \n");
					 }
					 else
					 {
						 fclose(file);
						 return false;	 
					 }
				}
				else
				{
					 fclose(file);
					 return false;
				}			   
					 
			    //printf("send bot regs, regBotcount = %d\n",);
				pkgBotNum++;
				regBotCount = 0;
				len = 8;
				memset(buf, 0, sizeof(buf));

				/*
				for (int k = 0; k < 50; ++k)
				{
					usleep(100 * 1000);
					if (m_wrBotRegsOK) break;
				}
				
				if (!m_wrBotRegsOK) 
				{
					finalRst = false;
					if (m_defualtFile) {
						printf("Flash更新: 写 bot 底板 寄存器超时, Failed \n");
					}
					printf("提示: 写 bot 底板 寄存器超时\n");
				}
				*/
				char qstr[256];
				snprintf(qstr, sizeof(qstr), "totalBotRegNum = %u, pkgBotNum = %u, regBotCount = %u \n", 
						totalBotRegNum, pkgBotNum, regBotCount);
			//	printf(qstr);
			}
			
			if ((100 * pkgBotNum + regBotCount) >= totalBotRegNum) 
			{
				printf("wait 2 seconds for fixing bot regs  \n");
				msleepByBoost(2000);
			/*
				usleep(2000 * 1000);
				if (m_defualtFile) {
					printf("Flash更新: 写 bot 底板 寄存器 OK \n");
				}*/
			}
		}
		// 处理顶板寄存器写入
		else if (token_count == 4 && strcmp(tokens[0], "T") == 0) 
		{
			uint32_t regAddr = hexStrToUInt(tokens[2]);
			memcpy(&buf[len], &regAddr, 4);
			len += 4;
			
			uint32_t regVal_write = hexStrToUInt(tokens[3]);
			memcpy(&buf[len], &regVal_write, 4);
			len += 4;
			
			regTopCount++;
			
			if ((regTopCount >= 100) || ((100 * pkgTopNum + regTopCount) >= totalTopRegNum)) 
			{
				//m_wrTopRegsOK = false;
				buf[0] = 0xE0;
				buf[1] = 0x15;
				memcpy(&buf[2], &totalTopRegNum, 2);
				memcpy(&buf[4], &pkgTopNum, 2);
				memcpy(&buf[6], &regTopCount, 2);
				//sendDataToClient(buf, len);
				//usleep(200 * 1000);
				
				printf("send top_regs , regtopcount = %d, pkgCount = %d\n", regTopCount, pkgTopNum);
				 cmdVec.clear();
				 cmdVec.insert(cmdVec.end(), buf, buf +len);
				 if( m_usedCom->setFixTopRegSendData(cmdVec, send_data, statusIdx) )
				 {
					 if( sendData(send_data, (const ReadWriteStateIndex)statusIdx) )
					 {
						 //return true;
						 printf("recv fix top reg ack, OK \n");
					 }
					 else
					 {
						 fclose(file);
						 return false;	 
					 }
				}
				else
				{
					 fclose(file);
					 return false;
				}	
				
				pkgTopNum++;
				regTopCount = 0;
				len = 8;
				memset(buf, 0, sizeof(buf));

				/*
				for (int k = 0; k < 50; ++k) 
				{
					usleep(100 * 1000);
					if (m_wrTopRegsOK) break;
				}
				
				if (!m_wrTopRegsOK) 
				{
					finalRst = false;
					if (m_defualtFile) 
					{
						printf("Flash更新: 写 top 顶板 寄存器超时, Failed.\n");
					}
					printf("提示: 写 top 顶板 寄存器超时\n");
				}
				*/
				
				char qstr[256];
				snprintf(qstr, sizeof(qstr), "totalTopRegNum = %u, pkgTopNum = %u, regTopCount = %u\n", 
						totalTopRegNum, pkgTopNum, regTopCount);
			//	printf(qstr);
			}
			
			if ((100 * pkgTopNum + regTopCount) >= totalTopRegNum) 
			{
				printf("wait 5 seconds for fixing top regs  \n");
				msleepByBoost(5000);
			/*
				usleep(5000 * 1000);
				if (m_defualtFile) 
				{
					printf("Flash更新: 写 top 顶板 寄存器 OK \n");
				}*/
			}
		}
	}
	
	fclose(file);
	
	// 等待PS回复
	//usleep(3000 * 1000);
	
	if (finalRst) 
	{
		printf("info: update machine config OK!\n");
		return true;
		
	} else {
		printf("Error: update machine config failed\n");
		return false;
	}

}




