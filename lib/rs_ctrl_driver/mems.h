#ifndef _TYPES_MEMS_H_
#define _TYPES_MEMS_H_



typedef struct
{
    uint32_t frameFlag;
    uint32_t length;
    uint32_t cmd;
    uint32_t checkSum;
} FrameHead;


constexpr std::size_t VIEWING_FIELD_PARAM_SIZE = 20;

constexpr uint32_t MAX_REG_ADDR = 0x83D00000;
constexpr uint32_t MIN_REG_ADDR = 0x83C00000;

// NOTE useful for all
constexpr uint32_t PL_FIRMWARE_VERSION_ADDR = 0x83c00000;  // PL端的固件版本号
constexpr uint32_t PS_FIRMWARE_VERSION_ADDR = 0x83c40800;  // PS端的固件版本号
constexpr uint32_t FIX_MIRROR_ADDR          = 0x83ca0000;  // 关闭振镜，0关闭，1打开
constexpr uint32_t UPLOAD_CLOSE_ADDR        = 0x83c00074;  // 点云数据上传，0关闭，1打开

#pragma pack(1)
typedef struct
{
    uint8_t MAC_address[6];  // lidar's MAC
    uint8_t mems_ip[4];      // lidar's ip
    uint8_t host_ip[4];      // lidar will send data to this ip

    uint16_t DIFOP_port;  // Device Info Output Protocol, lidar driver will capture this data to get device info
    uint16_t MSOP_port;   // Main data Stream Output Protocol, tcp connect to this port

    uint16_t sn[3];  // lidar's sn

    uint8_t subnet_mask[4];  // lidar's net mask
    uint8_t gateway[4];      // lidar's gateway

    uint8_t PTP_port;  //lidar's PTP port
} NetworkInfo;
#pragma pack()


#pragma pack(1)
typedef struct ViewingFieldParam
{
    uint32_t flag;
    struct data_t
    {
        uint8_t symbol;
        uint8_t value[2];
    } data[VIEWING_FIELD_PARAM_SIZE];
} ViewingFieldParam;
#pragma pack()


constexpr uint32_t FRAME_FLAG               = 0x12345678;  // mems
constexpr uint32_t VIEWING_FIELD_PARAM_FLAG = 0x11223344;

constexpr uint32_t NET_CMD_BEGIN                = 0x000;
constexpr uint32_t NET_CMD_READ_REGISTER        = 0x001;
constexpr uint32_t NET_CMD_WRITE_REGISTER       = 0x002;
constexpr uint32_t NET_CMD_READ_FLASH           = 0x003;
constexpr uint32_t NET_CMD_WRITE_FLASH          = 0x004;
constexpr uint32_t NET_CMD_READ_PARAMETER       = 0x005;  // 读网络信息
constexpr uint32_t NET_CMD_WRITE_PARAMETER      = 0x006;
constexpr uint32_t NET_CMD_FPGA_TO_FLASH        = 0x007;  // 固化
constexpr uint32_t NET_CMD_READ_FPGA_MEM        = 0x008;
constexpr uint32_t NET_CMD_WRITE_FPGA_MEM       = 0x009;
constexpr uint32_t NET_CMD_REG_ERASE_FLASH      = 0x00A;
constexpr uint32_t NET_CMD_SAMPLE_STORE         = 0x00B;
constexpr uint32_t NET_CMD_SAMPLE_SEND          = 0x00C;
constexpr uint32_t NET_CMD_SAMPLE_MODE          = 0x00D;
constexpr uint32_t NET_CMD_NORMAL_MODE          = 0x00E;
constexpr uint32_t NET_CMD_TCP_START            = 0x011;
constexpr uint32_t NET_CMD_TCP_END              = 0x012;
constexpr uint32_t NET_CMD_GET_INTENSITY        = 0x013;
constexpr uint32_t NET_CMD_READ_REGISTER_2      = 0x014;
constexpr uint32_t NET_CMD_WRITE_REGISTER_2     = 0x015;
constexpr uint32_t NET_CMD_READ_VIEW_PARAMETER  = 0x016;
constexpr uint32_t NET_CMD_WRITE_VIEW_PARAMETER = 0x017;
constexpr uint32_t NET_CMD_END                  = 0x050;

constexpr uint32_t NET_CMD_ACK_BEGIN                = 0x100;
constexpr uint32_t NET_CMD_ACK_READ_REGISTER        = 0x101;
constexpr uint32_t NET_CMD_ACK_WRITE_REGISTER       = 0x102;
constexpr uint32_t NET_CMD_ACK_READ_FLASH           = 0x103;
constexpr uint32_t NET_CMD_ACK_WRITE_FLASH          = 0x104;
constexpr uint32_t NET_CMD_ACK_READ_PARAMETER       = 0x105;
constexpr uint32_t NET_CMD_ACK_WRITE_PARAMETER      = 0x106;
constexpr uint32_t NET_CMD_ACK_FPGA_TO_FLASH        = 0x107;
constexpr uint32_t NET_CMD_ACK_READ_FPGA_MEM        = 0x108;
constexpr uint32_t NET_CMD_ACK_WRITE_FPGA_MEM       = 0x109;
constexpr uint32_t NET_CMD_ACK_REG_ERASE_FLASH      = 0x10A;
constexpr uint32_t NET_CMD_ACK_SAMPLE_STORE         = 0x10B;
constexpr uint32_t NET_CMD_ACK_SAMPLE_SEND          = 0x10C;
constexpr uint32_t NET_CMD_ACK_SAMPLE_MODE          = 0x10D;
constexpr uint32_t NET_CMD_ACK_NORMAL_MODE          = 0x10E;
constexpr uint32_t NET_CMD_ACK_TCP_START            = 0x111;
constexpr uint32_t NET_CMD_ACK_TCP_END              = 0x112;
constexpr uint32_t NET_CMD_ACK_GET_INTENSITY        = 0x113;
constexpr uint32_t NET_CMD_ACK_READ_REGISTER_2      = 0x114;
constexpr uint32_t NET_CMD_ACK_WRITE_REGISTER_2     = 0x115;
constexpr uint32_t NET_CMD_ACK_READ_VIEW_PARAMETER  = 0x116;
constexpr uint32_t NET_CMD_ACK_WRITE_VIEW_PARAMETER = 0x117;
constexpr uint32_t NET_CMD_ACK_END                  = 0x150;

#endif
