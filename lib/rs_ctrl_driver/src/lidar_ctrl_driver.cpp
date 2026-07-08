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

 这个是新的代码 
 
 *****************************************************************************/


#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <stdio.h>

#include "lidar_ctrl_driver.hpp"
#include "ctrl_driver_param.hpp"
#include "tcp_com.h"
#include "types.h"



robosense::lidar::LidarCtrlDriver::LidarCtrlDriver() 
{ 
    m_com = (TcpCom *)new TcpCom();
    ( (TcpCom *)m_com )->SetLidarType( RS_HELIOS);
}

robosense::lidar::LidarCtrlDriver::~LidarCtrlDriver()
{
    delete m_com;
}


int robosense::lidar::LidarCtrlDriver::setLidarType(LidarType type)
{
    int ret = -1;
    
    switch(type)
    {
        
    case RS_BP3:
    case RS_RUBY3:
    case RS_RUBY_LITE:
    case RS_HELIOS:
    case RS_ROCK:
    case RS_RUBY4:
        ret = 0;
        break;

    
    case RS_M1:
    case RS_M1P:
        ret = 0;
        break;

    case RS_AIRY:
    	ret = 0;
    	break;

    default:
        ret = -1;
        break;
        
    }

    
   ( (TcpCom *)m_com )->SetLidarType(type);

    return ret;

}


bool robosense::lidar::LidarCtrlDriver::init(RSCtrlDriverParam param)
{
    m_param.device_port = param.device_port;
    m_param.device_address = param.device_address;
    m_param.lidar_type = param.lidar_type;
    
    ( (TcpCom *)m_com )->SetLidarType( (LidarType)param.lidar_type);
    bool rst = ( (TcpCom *)m_com )->connect(param.device_address, param.device_port, 2);
    return rst;
}


void robosense::lidar::LidarCtrlDriver::uninit()
{
    disconnect();
}

bool robosense::lidar::LidarCtrlDriver::disconnect()
{
    return ( (TcpCom *)m_com )->disconnect();
}

bool robosense::lidar::LidarCtrlDriver::isConnected() const
{
    bool rst = ( (TcpCom *)m_com )->isConnected();
    if(rst)
    {
        return rst;
    }
    else
    {
        return ( (TcpCom *)m_com )->connect(m_param.device_address, m_param.device_port, 2);
    }
}


void robosense::lidar::LidarCtrlDriver::getLibVer(char *ver)
{
	char libver[32] = {"V2.3@20260123--17:20"};
	memcpy(ver, libver, sizeof(libver));
	printf("getLibVer() is %s\n", libver);
	
}



// 0为sleep mode低功耗, 1为正常工作模式, -1通信异常
bool robosense::lidar::LidarCtrlDriver::getMode(int* mode)  
{
    ConfigPara para;
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }

    u8 mm;
    getRes = ( (TcpCom *)m_com )->getMode(mm);
    if(getRes)
    {      
        if(mm == 0 || mm == 1)
        {
            *mode = (int)mm;
            return true;
        }
    }
    
    return false;

}


//  0 low power consumption mode, 
//  1 normal mode
bool robosense::lidar::LidarCtrlDriver::setMode(int mode)
{
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }
    
	u8 mm = (u8)mode;
    if(0 == mm || 1 == mm)
    {       
        return ( (TcpCom *)m_com )->setMode(mm);
    }
    else
    {
        return false;
    }

}

// 0 正向， 1反向, just for helios
bool robosense::lidar::LidarCtrlDriver::setMotorDir(u8 dir)  
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(0 == dir || 1 == dir)
	{		
		return ( (TcpCom *)m_com )->setMotorDir(dir);
	}
	else
	{
		return false;
	}

}

bool robosense::lidar::LidarCtrlDriver::getMotorDir(u8 &dir)
{
    ConfigPara para;
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }

    u8 mm;
    getRes = ( (TcpCom *)m_com )->getMotorDir(mm);
    if(getRes)
    {      
        if(mm == 0 || mm == 1)
        {
            dir = (u8)mm;
            return true;
        }
    }
    
    return false;


}

bool robosense::lidar::LidarCtrlDriver::getMonitorValue(float value[64])
{
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }
      
    return ( (TcpCom *)m_com )->getMonitorValue(value);

}



bool robosense::lidar::LidarCtrlDriver::rebootLidar()
{
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }
      
    return ( (TcpCom *)m_com )->rebootLidar();

}

void robosense::lidar::LidarCtrlDriver::rebootNoLink(char *ethName)
{
	((TcpCom *)m_com )->rebootNoLink(ethName);
}


bool robosense::lidar::LidarCtrlDriver::setIpPort(char *ip, char *mask, u16 msopPort)
{
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }

	u8 ipVal[4] = {0}, idx = 0;
    char *ss = strtok(ip, ".");
    
    
    while (ss != NULL)
    {    
		ipVal[idx] = atoi(ss);

		idx++;
		ss = strtok(NULL, ".");          // 再次调用strtok函数

		if(idx >= 4)
		{
			break;
		}
	}

	if(idx < 4)
	{
		printf("bad ip address %s, right like this: xxx.xxx.xxx.xxx \n", ip);
		return false;
	}

	if(ipVal[0] >= 224)
	{
		printf("bad ip address %s, the first address need < 224 \n", ip);
		return false;
	}

	if( ipVal[1] == 255 || ipVal[2] == 255 || ipVal[3] == 255 || ipVal[3] == 0 
	   || ipVal[1] == 0 || ipVal[2] == 0)
	{
		printf("bad ip address %s, do not use 255 or 0 \n", ip);
		return false;
	}


	
	u8 maskVal[4] = {0};
	ss = strtok(mask, ".");
	idx = 0;
	
	while (ss != NULL)
	{	
	   maskVal[idx] = atoi(ss);

	   idx++;
	   ss = strtok(NULL, ".");			// 再次调用strtok函数

	   if(idx >= 4)
	   {
		   break;
	   }
	}

	if(idx < 4)
	{
	   printf("bad ip mask address %s, right like this: xxx.xxx.xxx.xxx \n", mask);
	   return false;
	}

	if(maskVal[0] != 255)
	{
		printf("bad ip mask address %s, the first address need 255 \n", mask);
		return false;
	}

	if(  maskVal[1] == 0 || maskVal[1] == 255)
	{
	    // ok
	}
	else
	{
	   printf("bad ip mask 2 address %s, should be 0 or 255  \n", mask);
	   return false;
	}

	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        memcpy(para.hconfig.ipLocal, ipVal, sizeof(ipVal));
	        memcpy(para.hconfig.netmaskLocal, maskVal, sizeof(maskVal));
			para.hconfig.msopPort = msopPort;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type )
		{
			memcpy(para.r4info.netInfo.ip, ipVal, sizeof(ipVal));
			memcpy(para.r4info.netInfo.mask, maskVal, sizeof(maskVal));
			para.r4info.netInfo.msopPort = msopPort;
			return ( (TcpCom *)m_com )->SetLidarNetInfo(para.r4info.netInfo);
		}
		
    }
    else
    {        
        return false;
    }

}


bool robosense::lidar::LidarCtrlDriver::setWaveMode(u8 waveMode)
{
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }
    
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {        	
			para.hconfig.res[5] = (u16)waveMode;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type  || CRS_AIRY == m_param.lidar_type)
		{
			return ( (TcpCom *)m_com )->SetLidarWaveMode(waveMode);
		}
		
    }
    else
    {        
        return false;
    }

}


bool robosense::lidar::LidarCtrlDriver::setMotorPhaseLockCfg(u16 angle)
{  
	if(!isConnected())
	{
		return false;
	}

	if(CRS_RUBY4 == m_param.lidar_type	|| CRS_AIRY == m_param.lidar_type)
	{
		return ( (TcpCom *)m_com )->SetMotorPhaseLockCfg(angle);  
	}
	else
	{
		return false;
	}
}

bool robosense::lidar::LidarCtrlDriver::setLidarWorkMode(u8 mode)
{
	if(!isConnected())
	{
		return false;
	}

	if(mode > 2)
	{
		return false;
	}

	if(CRS_RUBY4 == m_param.lidar_type	|| CRS_AIRY == m_param.lidar_type)
	{
		return ( (TcpCom *)m_com )->SetLidarWorkMode(mode);  
	}
	else
	{
		return false;
	}

}

bool robosense::lidar::LidarCtrlDriver::setMotorStopAngle(u16 angle)
{
	if(!isConnected())
	{
		return false;
	}

	if(CRS_RUBY4 == m_param.lidar_type	|| CRS_AIRY == m_param.lidar_type)
	{
		return ( (TcpCom *)m_com )->SetMotorStopAngle(angle);  
	}
	else
	{
		return false;
	}


}


bool robosense::lidar::LidarCtrlDriver::setMotorSpeed(u8 level)
{
	if(!isConnected())
	{
		return false;
	}

	if(level > 3)
	{
		return false;
	}

	if(CRS_RUBY4 == m_param.lidar_type	|| CRS_AIRY == m_param.lidar_type)
	{
		return ( (TcpCom *)m_com )->SetMotorSpeed(level);  
	}
	else
	{
		return false;
	}

}


bool robosense::lidar::LidarCtrlDriver::setLidarFov(u16 start, u16 end)
{  
	if(!isConnected())
	{
		return false;
	}

	if(CRS_RUBY4 == m_param.lidar_type	|| CRS_AIRY == m_param.lidar_type)
	{
		return ( (TcpCom *)m_com )->SetLidarFov(start, end);  
	}
	else
	{
		return false;
	}
}

// 不允许设置MAC地址
bool robosense::lidar::LidarCtrlDriver::setLidarNetInfo(NetParam_st netInfo)
{  
    if(!isConnected())
    {
        return false;
    }

    if(CRS_RUBY4 == m_param.lidar_type  || CRS_AIRY == m_param.lidar_type)
	{
		ConfigPara params;
		bool rst = getConfigParams(params);
		if(!rst)
		{
			printf("getConfigParams() failed.\n");
			return false;
		}
			
		memcpy(netInfo.devMac, params.r4info.netInfo.devMac, sizeof(netInfo.devMac));
    	return ( (TcpCom *)m_com )->SetLidarNetInfo(netInfo);  
    }
    else
    {
		return false;
    }
}

bool robosense::lidar::LidarCtrlDriver::setLockPhase(u16 angle)
{
	if(angle > 360)
	{
		printf("setLockPhase() bad angle, should be in [0, 360]\n " );		
		return false;
	}

	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

    if( CRS_HELIOS ==  m_param.lidar_type)
   {
	    std::vector<uint32_t> addr;
	    std::vector<int32_t> reg_val;

	    addr.push_back(0x83c01004);
	    reg_val.push_back((int32_t)angle);

		getRes = ( (TcpCom *)m_com )->writeRegData(addr, reg_val); 

   }
   else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type)
   {	  	
		getRes = ( (TcpCom *)m_com )->SetMotorPhaseLockCfg(angle);
   }   
   
   return getRes;

	
}

bool robosense::lidar::LidarCtrlDriver::writeReg(u32 addr, u32 data)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(addr < 0xffff || (addr > 0x83bfffff  && addr < 0x83d00000  ))
	{
		// ok
	}
	else
	{
		
		printf("reg addr is out of range, should be in [0, 0xffff] or [0x83c00000, 0x83d00000] \n " );		
		return false;
	}

	std::vector<uint32_t> addr_vet;
	std::vector<int32_t>  data_vet;

	if( CRS_HELIOS ==  m_param.lidar_type)
   {

	    if(addr < 0xffff)
	    {
			data = 0x01000000 + addr * 256 + data;
    		addr = 0x83c20030;
	    }

	    addr_vet.push_back(addr);
	    data_vet.push_back((int32_t)data);

		getRes = ( (TcpCom *)m_com )->writeRegData(addr_vet, data_vet); 

   }
   else if (CRS_M1P == m_param.lidar_type)
   {
		return false;
   }
   else //if(CRS_RUBY4 == m_param.lidar_type)
   {	
	   addr_vet.push_back(addr);
	   data_vet.push_back((int32_t)data);
	   
	   getRes = ( (TcpCom *)m_com )->writeRegData(addr_vet, data_vet); 
   }   
   
   return getRes;

}


bool robosense::lidar::LidarCtrlDriver::readReg(u32 addr, u32 &value)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(addr < 0xffff || (addr > 0x83bfffff  && addr < 0x83d00000  ))
	{
		// ok
	}
	else
	{
		
		printf("reg addr is out of range, should be in [0, 0xffff] or [0x83c00000, 0x83d00000] \n " );		
		return false;
	}

	std::vector<uint32_t> addr_vet;
	std::vector<uint32_t>  data_vet;
	
	if(RS_HELIOS == m_param.lidar_type )
	{
		if(addr < 0xffff)
		{
			uint32_t regValTemp = 0x02000000 + addr * 256;
			uint32_t regAddrTemp = 0x83c20030;

			for(int count = 0; count < 5; ++count )
			{
				writeReg(regAddrTemp, regValTemp);

				u32 tmpVal;
				readReg(0x83c20034, tmpVal);
				if( (tmpVal & 0x10000) != 0 )
				{
					value =  tmpVal & 0xFF;
					return true;
				}
			}

			return false;
		}
	}

	std::vector<uint32_t> addr_vec;
	std::vector<int32_t> reg_val;

	addr_vec.push_back(addr);
	u32 val = 0;
	reg_val.push_back((int32_t)val);
	
	getRes = ( (TcpCom *)m_com )->readRegData(addr_vec, reg_val); 
	//printf("readreg return value is %d\n", (int)rst);
/*
	int k = reg_val.size();
	for(int i = 0; i < k; i++)
	{
		printf("readreg value is 0x%x\n", reg_val[i] );
	}*/
	
	value = reg_val[0];

	return getRes;

}

#if  0
// [0, 127]
bool robosense::lidar::LidarCtrlDriver::setPtpDomainNumber(u16 num)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(num > 127)
	{
		printf("setPtpDomainNumber() param is error, should be in [0, 127]\n")
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.ptpDomainNumber = (u16)num;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::setMsopSrcPort(u16 port)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.msopSrcPort = port;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::setDifopSrcPort(u16 port)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.difopSrcPort = port;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

// [1, 4094]
bool robosense::lidar::LidarCtrlDriver::setMsopVlanId(u16 id)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(id < 1 || id > 4094)
	{
		printf("setMsopVlanId() para is wrong, should be in [1, 4094]\n");
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.msopVlanId = id;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::setDifopVlanId(u16 id)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

    if(id < 1 || id > 4094)
	{
		printf("setDifopVlanId() para is wrong, should be in [1, 4094]\n");
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.difopVlanId = id;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::setPtpVlanId(u16 id)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if(id < 1 || id > 4094)
	{
		printf("setPtpVlanId() para is wrong, should be in [1, 4094]\n");
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.ptpVlanId = id;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

// en = 1 , enable,  on
// en = 0 , disable, off
bool robosense::lidar::LidarCtrlDriver::setReflectEnhance(u16 en)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if( en > 1)
	{
		printf("setReflectEnhance() para is wrong, should be in [0, 1]\n");
		return false;
	}

	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.reflectEnhance = en;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

// 0->off, 1->filter, 2->mark
bool robosense::lidar::LidarCtrlDriver::setRainMist(u16 type)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}
	if( type > 2)
	{
		printf("setRainMist() para is wrong, should be in [0, 2]\n");
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.rainMist = type;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }

}

// 0->off, 1->filter, 2->mark
bool robosense::lidar::LidarCtrlDriver::setAttactPointsFilterEnhance(u16 type)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	if( type > 2)
	{
		printf("setAttactPointsFilterEnhance() para is wrong, should be in [0, 2]\n");
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.attactPointsFilterEnhance = type;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type)
		{
			return false;
		}
		
    }
    else
    {        
        return false;
    }
    
}

#endif

bool robosense::lidar::LidarCtrlDriver::setConfigParams(ConfigPara params)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

		
	getRes = ( (TcpCom *)m_com )->SetLidarConfig(params);
    if(getRes)
    {
		return true;	
    }
    else
    {        
        return false;
    }

}



bool robosense::lidar::LidarCtrlDriver::getConfigParams(ConfigPara &params)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	memset(&params, 0, sizeof(params) );
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {  
		memcpy(&params, &para, sizeof(params));
		return true;	
    }
    else
    {        
        return false;
    }

}



bool robosense::lidar::LidarCtrlDriver::setTimeSyncMode(u8 timeMode)
{

	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}
	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        para.hconfig.res[2] = (u16)timeMode;
			return ( (TcpCom *)m_com )->SetLidarConfig(para);
		}
		else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type )
		{
			return ( (TcpCom *)m_com )->SetLidarSyncMode(timeMode);
		}		
    }
    else
    {        
        return false;
    }


}


bool robosense::lidar::LidarCtrlDriver::getWaveMode(u8 &waveMode)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        waveMode = para.hconfig.res[5];
			return  true;
		}
		else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type )
		{
			waveMode = para.r4info.waveMode;
			return true;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::getTimeSyncMode(u8 &timeMode)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        timeMode = para.hconfig.res[2];
			return  true;
		}
		else if(CRS_RUBY4 == m_param.lidar_type ||  CRS_AIRY == m_param.lidar_type )
		{
			timeMode = para.r4info.timeSyncInfo.mode;
			return true;
		}
		
    }
    else
    {        
        return false;
    }

}

//  status : 1为成功
bool robosense::lidar::LidarCtrlDriver::getPhaseStatus(u8 &status)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	
	ConfigPara para;
	
	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	        

        if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        status = (u8)para.hconfig.res[19];       
			printf("getPhaseStatus() phaselockSt = %d,   phase = %d, \n", status, para.hconfig.res[1]);
			return  true;
		}
		else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type)
		{
			status = (u8)para.r4info.motorInfo.phaseLockSt;
			printf("getPhaseStatus() phaselockSt = %d,   phase = %d, \n", status, para.r4info.motorInfo.phase);
			return true;
		}
		
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::ctrlEyeSafe(u8 on)
{
    ConfigPara para;
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }	
    
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(on); 
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(on); 	
    
    return getRes;
}

bool robosense::lidar::LidarCtrlDriver::openLaser()
{
    ConfigPara para;
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }

	uint32_t a = 0x83c20004;
	int32_t  v = 1;
    std::vector<uint32_t> addr;
    std::vector<int32_t> reg_val;

    addr.push_back(a);
    reg_val.push_back(v);
    
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(0); 
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(0); 

	for(int k = 0; k < 5; ++k)
	{
    	getRes =  ( (TcpCom *)m_com )->writeRegData(addr, reg_val); 
	}
    
    return getRes;

}

bool robosense::lidar::LidarCtrlDriver::closeLaser()
{
    ConfigPara para;
    bool connect = isConnected();
    bool getRes = false;
    if(!connect)
    {
        return false;
    }

	uint32_t a = 0x83c20004;
	int32_t  v = 0, back = 100;
    std::vector<uint32_t> addr;
    std::vector<int32_t> reg_val;
    std::vector<int32_t> readVal;

    addr.push_back(a);
    reg_val.push_back(v);
    readVal.push_back(back);
	
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(1); 
    getRes =  ( (TcpCom *)m_com )->CtrlEyeSafe(1); 
	
	for(int k = 0; k < 50; ++k)
	{
    	getRes =  ( (TcpCom *)m_com )->writeRegData(addr, reg_val); 
    	getRes =  ( (TcpCom *)m_com )->writeRegData(addr, reg_val); 
        //usleep(100*1000);
    	getRes =  ( (TcpCom *)m_com )->readRegData(addr, readVal);
		printf("read back value is %d\n ", readVal[0]);		
		if( 0 == readVal[0] )
		{
			break;
		}

//		printf("write at  %d times\n ", k);		

	}
    
    return getRes;

}


bool robosense::lidar::LidarCtrlDriver::getConfigVersion(char ver[32])
{
	//ver = 0;
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
        printf("Failed in getConfigVersion() function, the net is not connected.\n" );
		return false;
	}

	getRes = ( (TcpCom *)m_com )->GetConfigVersion(ver);
    if(getRes)
    {	
    
        //printf("OK in GetConfigVersion() function.\n" );
		
    }
    else
    {        
        printf("Failed in GetConfigVersion() function .\n" );
    }

    return getRes;


}

bool robosense::lidar::LidarCtrlDriver::getPatchVersion(u32 &ver)
{
	ver = 0;
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
        printf("Failed in getPatchVersion() function, the net is not connected.\n" );
		return false;
	}

	getRes = ( (TcpCom *)m_com )->GetPatchVersion(ver);
    if(getRes)
    {	
    
        //printf("OK in getConfigVersion() function.\n" );
		
    }
    else
    {        
        printf("Failed in GetPatchVersion() function .\n" );
    }

    return getRes;


}

bool robosense::lidar::LidarCtrlDriver::getVersions(VersionRst &ver)
{
	ConfigPara para;
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
        printf("Failed in GetVersions() function, the net is not connected.\n" );
		return false;
	}

	getRes = ( (TcpCom *)m_com )->GetLidarConfig(para);
    if(getRes)
    {	
    
        printf("OK in GetVersions() function.\n" );

        if( CRS_BP3 ==  m_param.lidar_type)
        {
	        memcpy(&ver, &para.hconfig.res[6], sizeof(ver));
			memcpy(&ver.motorVersion , &para.hconfig.res[12], 4);
			memcpy(&ver.webVersion , &para.hconfig.res[16], 4);
		}
        else if( CRS_HELIOS ==  m_param.lidar_type)
        {
	        memcpy(&ver, &para.hconfig.res[6], sizeof(ver));
			memcpy(&ver.motorVersion , &para.hconfig.res[12], 4);
			memcpy(&ver.webVersion , &para.hconfig.res[22], 4);
		}
		else if(CRS_RUBY4 == m_param.lidar_type || CRS_AIRY == m_param.lidar_type)
		{
			memcpy(&ver, &para.r4info.versionInfo, sizeof(ver));
		}
		
    }
    else
    {        
        printf("Failed in GetVersions() function, GetLidarConfig failed .\n" );
    }

    return getRes;
	
}




/*
    type：
        NET_CMD_TOP_BIN_UPDATE
        NET_CMD_TOPBACKUP_UPDATE      
        NET_CMD_BOT_BIN_UPDATE        
        NET_CMD_BOTBACKUP_UPDATE      
        NET_CMD_LINUX_APP_UPDATE        
        NET_CMD_LAPPBACKUP_UPDATE       
        NET_CMD_CGI_UPDATE            
        NET_CMD_MOTOR_UPDATE
        NET_CMD_RUBY4_UPDATE
        NET_CMD_BP4_UPDATE
        
    fileName: need contains string as follow:
        helios_top_xxxxxxx.bin
        helios_topbackup_xxxxx.bin
        helios_bot_xxxxx.bin            
        helios_botbackup_xxxxx.bin     
        helios_app_xxxxx.elf   
        helios_appbackup_xxxxx.elf             
        helios_cgi_xxxxx.tar.gz            
        helios_mot_xxxxx.hex
        ruby4_system_upgrade.zip
        bp4_system_upgrade.zip

*/
bool robosense::lidar::LidarCtrlDriver::update(u8 type, char *fileName)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
        printf("Failed in update() function, the net is not connected.\n" );
		return false;
	}

    char *ch6 = NULL;
	if( CRS_AIRY ==  m_param.lidar_type)
	{
		if(NET_CMD_FIX_MACHINE_REGS == type)
		{
			ch6 = strstr(fileName, ".txt");
			if(ch6 == NULL)
			{
			
				printf("Failed in update() function,  type is 0x20, fileName is wrong, should contais .txt \n");
				return false;
			}
			else
			{
				return ( (TcpCom *)m_com )->fixRegsCfg(fileName);
				
			}
		}
	}

    bool goodIdx = false;
    if( type >= NET_CMD_TOP_BIN_UPDATE && type <= NET_CMD_LAPPBACKUP_UPDATE)
    {
        goodIdx = true;
    }
    else
    {
        if(   NET_CMD_CGI_UPDATE == type || NET_CMD_MOTOR_UPDATE == type
           || NET_CMD_BP4_UPDATE == type || NET_CMD_RUBY4_UPDATE == type 
           || NET_CMD_PATCH_UPDATE == type 
           || NET_CMD_CONFIG_UPDATE == type  )
        {
            goodIdx = true;
        }
    }

    if(!goodIdx)
    {
        printf("Failed in update() function, the param type = 0x%x is wrong.\n", type);
        return false;
    }


    char *names3[32] = {(char*)"bpearl_top_", (char*)"bpearl_topbackup_", (char*)"bpearl_bot_", (char*)"bpearl_botbackup_",
                        (char*)"bpearl_app_", (char*)"bpearl_appbackup_", (char*)"bpearl_cgi_", (char*)"bpearl_mot_",
                        (char*)"ruby4_system_upgrade",  (char*)"bp4_system_upgrade", (char *)"patch", (char *)"config"};

    char *names1[32] = {(char*)"helios_top_",(char*) "helios_topbackup_", (char*)"helios_bot_", (char*)"helios_botbackup_",
                        (char*)"helios_app_", (char*)"helios_appbackup_", (char*) "helios_cgi_", (char*)"helios_mot_",
                        (char*)"ruby4_system_upgrade",  (char*)"bp4_system_upgrade", (char *)"patch", (char *)"config"};
                        
    char *names2[32] = {(char*)".bin", (char*)".bin", (char*)".bin",    (char*) ".bin",
                        (char*)".elf", (char*)".elf", (char*)".tar.gz", (char*)".hex",
                        (char*)".zip", (char*)".zip", (char*)".tar.gz", (char*)".tar.gz", };

	char *names4[32] = {(char*)".bit", (char*)".bit", (char*)".bin",    (char*) ".bin",
						(char*)".elf", (char*)".elf", (char*)".tar.gz", (char*)".txt",
						(char*)".zip", (char*)".zip", (char*)".tar.gz", (char*)".tar.gz", };


                            // 3,4, 5 
    u8 typeIdx[64] = {0, 0, 0, 0, 1, 2 ,3 ,4 ,5 , 0 };
    typeIdx[NET_CMD_CGI_UPDATE]   = 6;
    typeIdx[NET_CMD_MOTOR_UPDATE] = 7;
    typeIdx[NET_CMD_RUBY4_UPDATE] = 8;
    typeIdx[NET_CMD_BP4_UPDATE]   = 9;    
    typeIdx[NET_CMD_PATCH_UPDATE] = 10;
    typeIdx[NET_CMD_CONFIG_UPDATE]= 11;

    char *ch = NULL;

    if( CRS_AIRY ==  m_param.lidar_type)
    {
		ch = strstr(fileName, "airy");
    }
	else if( CRS_BP3 ==	m_param.lidar_type)
	{
    	ch = strstr(fileName, names3[ typeIdx[type] ]);
    }
    else
    {
		ch = strstr(fileName, names1[ typeIdx[type] ]);
    }
    
    if( NULL == ch)
    {
        //printf("Failed in update() function, the param fileName = %s is wrong, bad begin name.\n", fileName);
        //return false;
    }

    char *ch2 = NULL;
    if( CRS_AIRY ==  m_param.lidar_type)
    {
		if(NET_CMD_TOP_BIN_UPDATE == type)
		{
			ch2 = strstr(fileName, ".bin");
		}
		else if(NET_CMD_BOT_BIN_UPDATE == type)
		{
			ch2 = strstr(fileName, ".bit");
		}
		else  if(NET_CMD_LINUX_APP_UPDATE == type)// APP
		{
			ch2 = strstr(fileName, ".hs_fs");
		}
		else  //fix machine regs 
		{
			ch2 = strstr(fileName, ".txt");
		}
    }
    else if( CRS_BP3 ==	m_param.lidar_type)
	{
    	ch2 = strstr(fileName, names4[ typeIdx[type] ]);
    }
    else
    {
		ch2 = strstr(fileName, names2[ typeIdx[type] ]);
    }
    
    if( NULL == ch2)
    {
        printf("Failed in update() function, the param fileName = %s is wrong, bad end name.\n", fileName);
        return false;
    }


    FILE*fp;
    fp = fopen(fileName,"rb");// localfile文件名   
    if(NULL == fp)
    {   
        printf("Failed in update() function, fopen() fileName = %s .\n", fileName);
        return false;
    }
    
    fseek(fp, 0L, SEEK_END); /* 定位到文件末尾 */
    int flen = ftell(fp); /* 得到文件大小 */
    if(flen > 6*1024*1024 || flen < 100 )
    {
        printf("Failed in update() function, fileName = %s  len is %d, out of range [1KB, 5MB].\n", fileName, flen);
        fclose(fp);
        return false;
    }

	// just for debug
	//printf("update file len is %d\n", flen);
	
    char *p = new char[flen]; /* 根据文件大小动态分配内存空间 */
    fseek(fp, 0L, SEEK_SET); /* 定位到文件开头 */
    fread(p, flen, 1, fp); /* 一次性读取全部文件内容 */
    fclose(fp);

    std::vector<char> data;
    data.insert(data.end(), p, p+flen);
    getRes = ( (TcpCom *)m_com )->update(type, data);

    delete []p;
    if(getRes)
    {  
        printf("in update() function have done transmit the data to lidar.\n" );        
    }
    else
    {        
        printf("Failed in update() function .\n" );
    }

    return getRes;
    
}


bool robosense::lidar::LidarCtrlDriver::getUpdateStatus(u8 &type, s32 &status)
{
	bool getRes = false;	
    getRes = ( (TcpCom *)m_com )->getUpdateStatus(type, status);

    if(getRes)
    {       
        //printf("OK getUpdateStatus() function .\n" );        
    }
    else
    {        
        printf("Failed in getUpdateStatus() function .\n" );
    }

    return getRes;

}

bool robosense::lidar::LidarCtrlDriver::readConfigToFile(char *fileName)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	std::vector<char> data;	
	getRes = ( (TcpCom *)m_com )->readConfigFile(data);
    if(getRes)
    {
		FILE *fp = fopen(fileName, "w");
		char *ch = &data[0];
		int len = data.size();
		fwrite(ch, 1, len, fp );
		fclose(fp);
    
		return true;	
    }
    else
    {        
        return false;
    }

}


/*helios用的函数, 新增加2024.4.1*/
bool robosense::lidar::LidarCtrlDriver::readEventLog(char *fileName)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	std::vector<char> data;	
	getRes = ( (TcpCom *)m_com )->readEventLog(data);
    if(getRes)
    {
		FILE *fp = fopen(fileName, "w");
		char *ch = &data[0];
		int len = data.size();
		fwrite(ch, 1, len, fp );
		fclose(fp);
    
		return true;	
    }
    else
    {        
        return false;
    }

}

bool robosense::lidar::LidarCtrlDriver::deleteEventLog()
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

		
	getRes = ( (TcpCom *)m_com )->deleteEventLog();
    if(getRes)
    {
		return true;	
    }
    else
    {        
        return false;
    }

}


bool robosense::lidar::LidarCtrlDriver::readMonitorLog(char *fileName)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	std::vector<char> data;		
	getRes = ( (TcpCom *)m_com )->readMonitorLog(data);
    if(getRes)
    {    
    	FILE *fp = fopen(fileName, "w");
		char *ch = &data[0];
		int len = data.size();
		fwrite(ch, 1, len, fp );
		fclose(fp);
		return true;	
    }
    else
    {        
        return false;
    }

}


bool robosense::lidar::LidarCtrlDriver::restartMonitorLog()
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

		
	getRes = ( (TcpCom *)m_com )->restartMonitorLog();
    if(getRes)
    {
		return true;	
    }
    else
    {        
        return false;
    }
    
}


bool robosense::lidar::LidarCtrlDriver::readDaemonLog(char *fileName)
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

	std::vector<char> data; 	
	getRes = ( (TcpCom *)m_com )->readDaemonLog(data);
	if(getRes)
	{	 
		FILE *fp = fopen(fileName, "w");
		char *ch = &data[0];
		int len = data.size();
		fwrite(ch, 1, len, fp );
		fclose(fp);
		return true;	
	}
	else
	{		 
		return false;
	}

}


bool robosense::lidar::LidarCtrlDriver::deleteDaemonLog()
{
	bool connect = isConnected();
	bool getRes = false;
	if(!connect)
	{
		return false;
	}

		
	getRes = ( (TcpCom *)m_com )->deleteDaemonLog();
    if(getRes)
    {
		return true;	
    }
    else
    {        
        return false;
    }
    
}


bool robosense::lidar::LidarCtrlDriver::getZeroAngle(s32 &angle)
{
    if(!isConnected())
    {
        return false;
    }
    return ( (TcpCom *)m_com )->GetZeroAngle(angle); 

}

bool robosense::lidar::LidarCtrlDriver::getChannelAngle(std::vector<float>& angles, u16 inAngleNum)
{
    if(!isConnected())
    {
        return false;
    }
    return ( (TcpCom *)m_com )->GetChannelAngle(angles, inAngleNum); 

}


bool robosense::lidar::LidarCtrlDriver::setZeroAngle(s32 angle)
{
    if(!isConnected())
    {
        return false;
    }
    return ( (TcpCom *)m_com )->SetZeroAngle(angle); 

}

bool robosense::lidar::LidarCtrlDriver::setChannelAngle(std::vector<float>        angles)
{
    if(!isConnected())
    {
        return false;
    }
    return ( (TcpCom *)m_com )->SetChannelAngle(angles); 

}




bool robosense::lidar::LidarCtrlDriver::setImuParams(ImuParam setParams)
{
	if(CRS_AIRY != m_param.lidar_type)
	{
		printf("bad lidar type, just support CRS_AIRY\n");
		return false;
	}

	if(setParams.udpPort < 1024)
	{
		printf("Error, in setImuParams() func, udpPort should > 1024 \n");
		return false;
	}

	if(setParams.vlanID > 7 )
	{
		printf("Error, in setImuParams() func, vlanID should < 8 \n");
		return false;
	}
	
	if(setParams.vlanPrio > 4094 )
	{
		printf("Error, in setImuParams() func, vlanPrio should < 4094 \n");
		return false;
	}

	

	return ( (TcpCom *)m_com )->setImuParams(setParams); 

}


bool robosense::lidar::LidarCtrlDriver::getImuParams(ImuParam &getParams)
{
	if(CRS_AIRY != m_param.lidar_type)
	{
		printf("bad lidar type, just support CRS_AIRY\n");
		return false;
	}

	return ( (TcpCom *)m_com )->getImuParams(getParams); 

}

bool robosense::lidar::LidarCtrlDriver::getSupplementParams(NetInfo2 &get_suparam, NetInfoSet &set_suparam)
{
	bool rst =  ( (TcpCom *)m_com )->getSupplementParams(get_suparam); 

	set_suparam.ptpNum = get_suparam.ptpNum;
	set_suparam.msopVlanId = get_suparam.msopVlanId;
	set_suparam.msopVlanPrio = get_suparam.msopVlanPrio;
	set_suparam.difopVlanId = get_suparam.difopVlanId;
	set_suparam.difopVlanPrio = get_suparam.difopVlanPrio;
	set_suparam.ptpVlanId = get_suparam.ptpVlanId;
	set_suparam.ptpVlanPrio = get_suparam.ptpVlanPrio;
	
	set_suparam.respToPdelay = get_suparam.respToPdelay;
	set_suparam.noLeepSecond = get_suparam.noLeepSecond;
	set_suparam.syncTimeoutVal = get_suparam.syncTimeoutVal;
	set_suparam.unlockToLockTime = get_suparam.unlockToLockTime;
	set_suparam.lockToUnlockTime = get_suparam.lockToUnlockTime;
	
	set_suparam.triggerMode = get_suparam.triggerMode;
	set_suparam.enableAngleWave = get_suparam.enableAngleWave;
	set_suparam.startAngle = get_suparam.startAngle;
	set_suparam.angleStep = get_suparam.angleStep;
	set_suparam.angleWaveWidth = get_suparam.angleWaveWidth;
	set_suparam.webCfgVer = get_suparam.webCfgVer;
	
	set_suparam.u16FrameStartAngle = get_suparam.u16FrameStartAngle;
	set_suparam.machineSlideEn = get_suparam.machineSlideEn;
	set_suparam.deadZone10cmEn = get_suparam.deadZone10cmEn;
	set_suparam.topCh81858393En = get_suparam.topCh81858393En;
	set_suparam.poorPerfChnMask = get_suparam.poorPerfChnMask;
	set_suparam.gapFilling = get_suparam.gapFilling;
	set_suparam.u8RangingMode = get_suparam.u8RangingMode; 
	set_suparam.msopPort1 = get_suparam.msopPort1;
	set_suparam.u8Phy1000M = get_suparam.u8Phy1000M;
	set_suparam.u8PhyMaster = get_suparam.u8PhyMaster;
	set_suparam.u8Pkt48Line = get_suparam.u8Pkt48Line;
	set_suparam.u8RainFogSwitch = get_suparam.u8RainFogSwitch;
	
	set_suparam.u8PlasticFreqSaved = get_suparam.u8PlasticFreqSaved;
	set_suparam.u16PlasticFreq = get_suparam.u16PlasticFreq;
	
	set_suparam.resv2 = get_suparam.resv2;
	memcpy(set_suparam.resv, get_suparam.resv6, sizeof(get_suparam.resv6));  
	
	return rst;
	
}

bool robosense::lidar::LidarCtrlDriver::setSomeSupplementParams(NetInfoSet setParams)
{
	return ( (TcpCom *)m_com )->setSomeSupplementParams(setParams); 
}

bool robosense::lidar::LidarCtrlDriver::setReflectEnhance(u8 level)
{
	if(level > 3)
	{
		printf("Error, in setReflectEnhance() func, level should < 4\n");
		return false;
	}
	return ( (TcpCom *)m_com )->setReflectEnhance(level); 
}

bool robosense::lidar::LidarCtrlDriver::setGpsBaud(u8 level)
{
	if(level < 3 || level > 16 )
	{
		printf("Error, in setGpsBaud() func, level should in [3, 16]\n");
		return false;
	}

	return ( (TcpCom *)m_com )->setGpsBaud(level); 
} 

bool robosense::lidar::LidarCtrlDriver::setTrailFilter(u8 level)
{
	if(level < 1 || level > 7 )
	{
		printf("Error, in setTrailFilter() func, level should in [1, 7]\n");
		return false;
	}

	return ( (TcpCom *)m_com )->setTrailFilter(level); 

}
bool robosense::lidar::LidarCtrlDriver::setRainBlockDetectDistance(u8 level)
{	
	if(level > 3)
	{
		printf("Error, in setRainBlockDetectDistance() func, level should < 4\n");
		return false;
	}
	return ( (TcpCom *)m_com )->setRainBlockDetectDistance(level); 
}
bool robosense::lidar::LidarCtrlDriver::setRainDetectSensitivity(u8 level)
{
	if(level > 2)
	{
		printf("Error, in setRainDetectSensitivity() func, level should < 3\n");
		return false;
	}

	return ( (TcpCom *)m_com )->setRainDetectSensitivity(level); 
}
bool robosense::lidar::LidarCtrlDriver::setBlockDetectSensitivity(u8 level)
{
	if(level > 2)
	{
		printf("Error, in setBlockDetectSensitivity() func, level should < 3\n");
		return false;
	}

	return ( (TcpCom *)m_com )->setBlockDetectSensitivity(level); 
}

