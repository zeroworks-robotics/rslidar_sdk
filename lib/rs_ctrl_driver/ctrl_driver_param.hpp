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

#include <string>
#include <stdint.h>
#include <iostream>
#include <stdlib.h>

namespace robosense
{
namespace lidar
{

enum CtrlLidarType  ///< LiDAR type
{   
    CRS_BP3 = 1,
    CRS_RUBY3,
    CRS_RUBY_LITE,
    CRS_HELIOS,
    CRS_ROCK,
    CRS_RUBY4, // BP4  the same
    CRS_M1,
    CRS_M1P,
    CRS_AIRY,
    CRS_END,
};

inline std::string ctrlLidarTypeToStr(CtrlLidarType type)
{
  std::string str = "";

  switch (type)
  {
    case CRS_BP3:
      str = "CRS_BP3";
      break;
    case CRS_HELIOS:
      str = "CRS_HELIOS";
      break;
    case CRS_RUBY3:
      str = "CRS_RUBY3";
      break;
    case CRS_RUBY_LITE:
      str = "CRS_RUBY_LITE";
      break;
    case CRS_RUBY4:
      str = "CRS_RUBY4";
      break;
    case CRS_M1:
      str = "CRS_M1";
      break;
    case CRS_M1P:
      str = "CRS_M1P";
      break;
	case CRS_AIRY:
	  str = "CRS_AIRY";
      break;
      
    default:
      str = "ERROR";
  }

  return str;
}

inline CtrlLidarType strToCtrlLidarType(const std::string& type)
{
  if (type == "CRS_BP3")
  {
    return CRS_BP3;
  }
  else if (type == "CRS_HELIOS")
  {
    return CRS_HELIOS;
  }
  else if (type == "CRS_RUBY3")
  {
    return CRS_RUBY3;
  }
  else if (type == "CRS_RUBY_LITE")
  {
    return CRS_RUBY_LITE;
  }
  else if (type == "CRS_RUBY4")
  {
    return CRS_RUBY4;
  }
  else if (type == "CRS_M1")
  {
    return CRS_M1;
  }
  else if (type == "CRS_M1P")
  {
    return CRS_M1P;
  }
  else if (type == "CRS_AIRY")
  {
    return CRS_AIRY;
  }   
  else
  {
    std::cout << "Wrong Ctrl lidar type: " << type << std::endl;
    std::cout << "Please give correct type: CRS_BP2V5, CRS_HELIOS, CRS_RUBY3, CRS_RUBY_LITE, CRS_RUBY4, CRS_M1, CRS_M1P."
             << std::endl;
    exit(-1);
  }
}

struct RSCtrlDriverParam
{
    std::string   device_address;
    uint16_t      device_port;
    CtrlLidarType lidar_type;
};

}  // namespace lidar
}  // namespace robosense
