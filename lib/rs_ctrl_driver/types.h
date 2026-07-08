#ifndef _TYPES_LIDAR_H_
#define _TYPES_LIDAR_H_

#include <stdint.h>

/* ------------------------ Defines ------------------------- */
typedef char                    s8;
typedef short                   s16;
typedef int                     s32;
typedef long long               s64;

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;



#define DOWNLOAD_PROGRESS               0X001
#define NET_CMD_TOP_PARA_UPDATE         0x002
#define NET_CMD_TOP_BIN_UPDATE          0x003
#define NET_CMD_TOPBACKUP_UPDATE        0x004
#define NET_CMD_BOT_BIN_UPDATE          0x005
#define NET_CMD_BOTBACKUP_UPDATE        0x006
#define NET_CMD_LINUX_APP_UPDATE		0x007
#define NET_CMD_LAPPBACKUP_UPDATE		0x008
#define NET_CMD_CGI_UPDATE			    0x00C
#define NET_CMD_CGI_BACK_UPDATE			0x00D
#define NET_CMD_MOTOR_UPDATE		    0x016
#define NET_CMD_MOTOR_BACK_UPDATE		0x017
#define NET_CMD_OTHER_UPDATE		    0x018

#define NET_CMD_RUBY4_UPDATE            0x020
#define NET_CMD_BP4_UPDATE              0x023
#define NET_CMD_PATCH_UPDATE            0X024
#define NET_CMD_CONFIG_UPDATE           0x033

#define NET_CMD_FIX_MACHINE_REGS        0x51   // 0350, 0351, 0360


#define NET_CMD_ACK_TOP_BIN_UPDATE		0x103
#define NET_CMD_ACK_TOPBACKUP_UPDATE	0x104
#define NET_CMD_ACK_BOT_BIN_UPDATE		0x105
#define NET_CMD_ACK_BOTBACKUP_UPDATE	0x106
#define NET_CMD_ACK_LINUX_APP_UPDATE	0x107
#define NET_CMD_ACK_LAPPBACKUP_UPDATE	0x108
#define NET_CMD_ACK_CGI_UPDATE			0x10C
#define NET_CMD_ACK_MOTOR_UPDATE		0x116

enum HeliosWaveMode
{
	H_WAVE_DUAL = 0,
	H_WAVE_STRONGEST,
	H_WAVE_LAST,	
	H_WAVE_FIRST,
};

enum Ruby4WaveMode
{
	R_WAVE_STRONGEST = 0,
	R_LAST,
	R_FIRST,
	R_STRONGEST_LAST,
	R_STRONGEST_FIRST,
	R_LAST_FIRST,
};

enum TimeSyncMode
{
	SYNC_GPS = 0,
	SYNC_PTP_E2E,
	SYNC_PTP_P2P,
	SYNC_PTP_GPTP,
	SYNC_PTP_E2E_L2,
};

enum LidarType  ///< LiDAR type
{   
    RS_BP3 = 1,
    RS_RUBY3,
    RS_RUBY_LITE,
    RS_HELIOS,
    RS_ROCK,
    RS_RUBY4,
    RS_M1,
    RS_M1P,
    RS_AIRY,
    RS_END,
};


#pragma pack(1)

typedef struct LidarPara
{  
   // sizeof(struct lidarPara) = 256 Bytes
   u16 motorSpeed;
   u16 motorPhaseLock;
   u16 timeSyncMode;
   u16 startFov;
   u16 endFov;
   u16 waveMode;
   u16 topVersion[2];
   u16 botVersion[2]; // 20
   
   u16 appVersion[2];
   u16 motorVersion[2];
   u16 topVerDate[2];//top firmware date
   u16 sensorTemper;
   u16 motorRealTimeSpeed;
   u16 motorRealTimePhase;
   u16 statusOfPhaseLock;   //40

   u16 statusOfCodeWheelCali;
   u16 pulseTrigMode;   
   u16 webVersiom[2];   
   u16 res0[16]; // 80

} lidarPara;
   

typedef union UnionParam
{   
    LidarPara ldPara;
    u16 res[128];
    
}UnionParam;



typedef struct HeliosConfigPara
{
    u8 sn[6];
    u8 mac[6];

    u8 ipLocal[4];
    u8 ipRemote[4];

    u16 msopPort;
    u16 difopPort;

    //UnionParam param;
    u16 res[64];
	u16 ptpDomainNumber;  // [0-127]
	u16 msopSrcPort;
	u16 difopSrcPort;
	u16 msopVlanId;       // [1, 4094]
	u16 msopVlanPriority; // [0, 7]
	u16 difopVlanId;      // [1, 4094]
	u16 difopVlanPriority;// [0, 7]
	u16 ptpVlanId;        // [1, 4094]
	u16 ptpVlanPriority;  // [0, 7]
	u16 reflectEnhance;   // {0->off, 1->0n}
	u16 rainMist;         // {0->off, 1->filter, 2->mark}
	u16 attactPointsFilterEnhance; // {0->off, 1->filter, 2->mark}
	u16 res1[52];


    u8 netmaskLocal[4];
    u8 gatewayLocal[4];

} HeliosConfigPara;



//---------ruby4 config---------------------------

typedef struct net_param_info
{
    u8 devMac[6];
    u8 ip[4];
    u8 mask[4];
    u8 gateway[4];
    u16 msopPort;
    u16 difopPort;
    u8 remoteIp[4];
} NetParam_st;

typedef struct time_sync
{
    u8 mode;
    u8 status;
}TimeSync_st;


typedef struct motor_param_info
{
    u8  speed;
    u16 phase;
    u8  codeWheelCaliSt;
    u16 phaseLockSt;
    u16 realSpeed;
    u16 realPhase;
}MotorParam_st;



typedef struct version_info
{
    u8 topVersion[4];
    u8 botVersion[4];
    u8 appVersion[4];
    u8 webVersion[4];
    u8 motorVersion[4];
}Version_st;



// airy 0350 use the same lidar_info as ruby4

typedef struct lidar_info
{
    u8 sn[6];
    NetParam_st netInfo;
    TimeSync_st timeSyncInfo;
    u8 waveMode;
    Version_st versionInfo;
    float zeroAngle;
    u16 startFov;
    u16 endFov;
    MotorParam_st motorInfo;
}Ruby4LidarInfo;



typedef union ConfigPara
{   
    HeliosConfigPara hconfig;
    Ruby4LidarInfo r4info;
    
}ConfigPara;



typedef struct VersionRst
{
    u32 topVersion;
    u32 botVersion;
    u32 appVersion;
    u32 webVersion;
    u32 motorVersion;
}VersionRst;




//-------- the following is airy 0350 structs -----------

typedef struct NetInfo2
{
    u16 ptpNum;
	u16 msopVlanId;
	u16 msopVlanPrio;
	u16 difopVlanId;
	u16 difopVlanPrio;
	u16 ptpVlanId;
	u16 ptpVlanPrio;
	u16 resv[64];
	u8  reflectivityEnhance;
	u8  gpsBaud;
	u8  motorRotation;
	u8  respToPdelay;
	u8  noLeepSecond;
	u8  syncTimeoutVal;
	u8  unlockToLockTime;
	u8  lockToUnlockTime;
	u8  trailFilterLevel;
	u8  msopMode;
	u8  trailFilterEnhance;
	u8  triggerMode;
	u8  enableAngleWave;
	float   startAngle;
	float   angleStep;
	u32 angleWaveWidth;
	u32 webCfgVer;
	u8  rainDist;
	u8  rainSensitivity;
	u8  blockSensitivity;
	u8  trailSpecialMode;
	u16 u16FrameStartAngle;
	u8  machineSlideEn;
	u8  deadZone10cmEn;
	u8  topCh81858393En;
	u8  poorPerfChnMask;	
	u8  gapFilling;
	u8  u8RangingMode;   // 测距模式 0：精度优先  1：雨水灰尘  2：平衡模式
	u16 msopPort1;       // Brain corp降采样端口配置
	u8  u8Phy1000M;      // 0360用
	u8  u8PhyMaster;     // 0360用
	u8  u8Pkt48Line;     // 0 is 96; 1 is 48
	u8  u8PlasticFreqSaved;
	u16 u16PlasticFreq;	
	u8  u8RainFogSwitch; // 雨雾开关   0关闭，1打开
	u8  resv2;
	u16 resv6[40];	
    
} NetInfo2_st;


typedef struct NetInfoSet
{
    u16 ptpNum;
	u16 msopVlanId;
	u16 msopVlanPrio;
	u16 difopVlanId;
	u16 difopVlanPrio;
	u16 ptpVlanId;
	u16 ptpVlanPrio;
	u8  respToPdelay;
	u8  noLeepSecond;
	u8  syncTimeoutVal;
	u8  unlockToLockTime;
	u8  lockToUnlockTime;
	u8  resv1;
	u8  triggerMode;
	u8  enableAngleWave;
	float   startAngle;
	float   angleStep;
	u32 angleWaveWidth;
	u32 webCfgVer;
	u16 u16FrameStartAngle;
	u8  machineSlideEn;
	u8  deadZone10cmEn;
	u8  topCh81858393En;
	u8  poorPerfChnMask;
	u8  gapFilling;
	u8  u8RangingMode;
	u16 msopPort1;       // Brain corp降采样端口配置
	u8  u8Phy1000M;      // 0360用
	u8  u8PhyMaster;     // 0360用
	u8  u8Pkt48Line;     // 0 is 96; 1 is 48
	u8  u8PlasticFreqSaved;
	u16 u16PlasticFreq;	
	u8  u8RainFogSwitch; // 雨雾开关   0关闭，1打开
	u8  resv2;
	u16 resv[43];	
    
} NetInfoSet_st;


typedef struct ImuParam
{
    u8  enabled;     //  0 -- disabled, 1 -- enabled
	u8  dataRate;    //  0 -- 25Hz, 1 -- 50Hz, 2 -- 100Hz, 3 -- 200Hz, 4 -- 1000Hz
	u8  accelFsr;    //  0 -- [-2g, 2g], 1 -- [-4g, 4g], 2 -- [-8g, 8g], 3 -- [-16g, 16g]
	u8  gyroFsr;     //  0 -- [-250, 250] dps, 1 -- [-500, 500] dps, 2 -- [-1000, 1000] dps, 3 -- [-2000, 2000] dps,  
	u16 udpPort;     //  [1024, 65535]
	u16 vlanID;      //  [0, 7]
	u16 vlanPrio;    //  [0, 4094]
	u8  calibStatus; //  0 not calib; 1 is calibed
	u8  lpf;         //  [0, 255] Low pass filter
	u8  resv[8];	
    
} ImuParam_st;

//-------- end of airy 0350 structs -----------

#pragma pack()


#endif

