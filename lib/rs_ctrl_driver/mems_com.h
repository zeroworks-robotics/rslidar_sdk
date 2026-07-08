
#ifndef _MEMS_COM_H_
#define _MEMS_COM_H_

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <stdint.h>
#include <boost/asio.hpp>

#include "mems.h"
#include "lidar_base.h"


class MemsCom : public LidarBase
{
public:
    MemsCom();
    ~MemsCom();

    bool ParseRecvTcpData(std::vector<char> & data) override;

    /*
     * @brief     write vector of address and value to mems
     *
     * @param     reg_addr           vector of address to write
     * @param     reg_val            vector of value corresponding to reg_addr
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               write sucessfully
     * @return    false              some error happen
    **/
    bool writeRegData(const std::vector<uint32_t>& reg_addr,
                      const std::vector<int32_t>& reg_val,
                      std::vector<char> & send_data,
                      uint8_t & statusIdx ) override;
    /*
     * @brief     write vector of value start from address start_reg_addr
     *
     * @param     start_reg_addr     start from register address
     * @param     reg_val            vector of register value to write
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               write sucessfully
     * @return    false              some error happen
    **/
    bool writeRegData(const uint32_t start_reg_addr, std::vector<int32_t>& reg_val,
                            std::vector<char> & send_data, uint8_t & statusIdx ) override;
    /*
     * @brief     read register data of address in a vector
     *
     * @param     reg_addr           vector of address to read
     * @param     reg_val            vector of value corresponding to reg_addr
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               read sucessfully
     * @return    false              some error happen
    **/
    bool getReadRegSendData(const std::vector<uint32_t>& reg_addr, 
                            std::vector<int32_t>& reg_val, 
                            std::vector<char> & send_data, uint8_t & statusIdx) override;
                            
    bool getReadRegResult(const std::vector<uint32_t>& addrs2read, std::vector<int32_t>& reg_val) override;
        
    /*
     * @brief     read vector of value start from address start_reg_addr
     *
     * @param     start_reg_addr     start from register address
     * @param     reg_number         total number register to read
     * @param     reg_val            vector of register value to read
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               read sucessfully
     * @return    false              some error happen
    **/
    bool readRegData(const uint32_t start_reg_addr,
                     const uint32_t reg_number,
                     std::vector<int32_t>& reg_val,
                     std::vector<char> & send_data) override;


    /*
     * @brief     use to get intensity data, same as readRegData
    **/
    bool getIntensityData(const std::vector<uint32_t>& addrs2read,
                          std::vector<int32_t>& calib_data,
                          std::vector<char> & send_data) override;

    /*
     * @brief     read viewing field params
     *
     * @param     param              viewing field params in lidar
     * @return    true               get param sucessfully
     * @return    false              get param failed
    **/
    bool readViewingFieldParams(std::vector<double>& param) override;
    /*
     * @brief     write viewing field params in csv to lidar, it read params in csv and call writeViewingFieldParams(const std::vector<double>& param)
     *
     * @param     csv_path           csv file path
     * @return    true               write sucessfully
     * @return    false              write failed
    **/
    bool writeViewingFieldParams(const std::string& csv_path) override;
    /*
     * @brief     write viewing field params to lidar
     *
     * @param     param              params to write
     * @return    true               write sucessfully
     * @return    false              write failed
    **/
    bool writeViewingFieldParams(const std::vector<double>& param) override;
    /*
     * @brief     set lidar's network info // NOTE remeber to getNetworkInfo before, and only change info you want to change
     *
     * @param     network_info       lidar info to set
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               set sucessfully
     * @return    false              some error happen
    **/
    bool setNetworkInfo(const NetworkInfo& network_info, const uint32_t msec = 1000) override;
    /*
     * @brief     get the network info
     *
     * @param     network_info
    **/
    void getNetworkInfo(NetworkInfo& network_info) const override;

    /*
     * @brief     when write to MEMS, you just write to FPGA, register will reset after power off, if you want lidar keep
     *            register value after restart, you must fixRegister to write data to flash
     * @param     msec_10            wait n 10 seconds for reply from lidar
     * @return    true               set sucessfully
     * @return    false              some error happen
    **/
    bool fixRegister(const uint32_t msec = 1000) override;

    /*
     * @brief     this function is for lidar firmware update
     * @param     file_path          firmware for lidar,must be bin file
     * @param     msg                success or fail message
     * @param     update_type        0 for firmware update ,1 for firmware backup update
     * @return    true               set sucessfully
     * @return    false              some error happen
     **/
    bool writeToFlash(const std::string& file_path, std::string &msg, const int update_type = 0) override;

//// -------------------end of MEMS functions--------------------------------------
    bool writeCMD(const uint32_t cmd, const ReadWriteStateIndex index, uint32_t msec_10);
    bool readNetworkInfo();

    bool setConfigSendData(ConfigPara para, std::vector<char> & send_data, uint8_t & statusIdx) ;
    bool getConfigSendData(std::vector<char> & send_data, uint8_t & statusIdx);
    bool getConfigResult(ConfigPara &para);
    bool CtrlEyeSafe(u16 on, std::vector<char> & send_data, uint8_t &statusIdx);
    bool writeCalibrateReflectData(const std::vector<uint32_t>& reg_addr, std::vector<uint32_t>& reg_val,
                                         std::vector<char> & send_data,uint8_t & statusIdx);

    bool getReadCaliRefResult(std::vector<uint32_t>& addrs2read, std::vector<uint32_t>& vecRetRegVal);

    bool  SetMotorStopAngle(u16 angle, std::vector<char> & send_data, uint8_t &statusIdx); // 

    bool SetZeroAngle(s32 angle, std::vector<char> & send_data, uint8_t &statusIdx);
    bool GetZeroAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx);
    bool GetZeroAngleResult(s32 &angle);
    bool  SetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, std::vector<float>  angles);
    bool  GetChannelAngleSendData(std::vector<char> & send_data, uint8_t &statusIdx, u16 inAngleNum);
    bool  GetChannelAngleResult(std::vector<float>& angles);
    

                                      
private:



private:

    NetworkInfo net_work_info_;

    int  m_len = 0;
    char reg_back_data_[4096];
    char collect_back_data_[4096];
    char write_flash_back_data_[256];
};

   
#endif  // _TCP_COM_H_

