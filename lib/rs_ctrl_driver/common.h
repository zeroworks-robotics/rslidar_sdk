/******************************************************************************
 *
 * Copyright (C) 2014 - 2019 RoboSense, Co., Ltd.  All rights reserved.
 *
 ******************************************************************************/
/*
 * @file common.h
 *
 * Ver    Who     Date         Changes
 * -----  ------  ----------   ---------------------------------------------
 * 1.00a  sloan     2020/09/03   First release
 *

not used file.

 
 ******************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

#define CH_SEL 0x2600

#define TX_1_8 0x1103
#define TX_9_16 0x1104
#define TX_17_24 0x1105
#define TX_25_32 0x1106
#define TX_33_40 0x1107
#define TX_41_48 0x1108
#define TX_49_56 0x1109
#define TX_57_64 0x110a
#define TX_65_72 0x110b
#define TX_73_80 0x110c
#define TX_81_88 0x110d
#define TX_89_96 0x110e
#define TX_97_104 0x110f
#define TX_105_112 0x1110
#define TX_113_120 0x1111
#define TX_121_128 0x1112

#define DATA_LOCK 0x2601
#define AREA_GDI_H 0x2612
#define AREA_GDI_L 0x2613
#define DIST_GDI_H 0x2614
#define DIST_GDI_L 0x2615

#define TMP_H 0x301f
#define TMP_L 0x3020

#define CHARGE_FIX_EN 0x1208
#define CHARGE_FIX_VALUE 0x1209

#define VGA_FIX_ADDR_EN 0x120a
#define VGA_FIX_ADDR 0x120b

#define CHARGE_AREA_COMPENSATION 0x200f
#define VBR_ADDR 0x3002
#define ROTATOR_SPEED 0x83c00210

#define FRAME_FLAG 0x12345678

#define NET_CMD_BEGIN 0x000
#define NET_CMD_READ_REGISTER 0x001
#define NET_CMD_WRITE_REGISTER 0x002
#define NET_CMD_TOP_BIN_UPDATE 0x003
#define NET_CMD_TOPBACKUP_UPDATE 0x004
#define NET_CMD_BOT_BIN_UPDATE 0x005
#define NET_CMD_BOTBACKUP_UPDATE 0x006
#define NET_CMD_LINUX_APP_UPDATE 0x007
#define NET_CMD_LAPPBACKUP_UPDATE 0x008
#define NET_CMD_READ_CONFIG 0x009
#define NET_CMD_WRITE_CONFIG 0x00A
#define NET_CMD_TOP_PARA_UPDATE 0x00B
#define NET_CMD_BOT_PARA_UPDATE 0x00C
#define NET_CMD_TOP_FLASH_READ 0x00D
#define NET_CMD_TOP_FLASH_WRITE 0x00E
#define NET_CMD_TOP_READ_REGISTER 0x00F
#define NET_CMD_TOP_WRITE_REGISTER 0x010
#define NET_CMD_TOP_GET_INTENSITY 0x011
#define NET_CMD_END 0x050

#define NET_CMD_ACK_BEGIN 0x100
#define NET_CMD_ACK_READ_REGISTER 0x101
#define NET_CMD_ACK_WRITE_REGISTER 0x102
#define NET_CMD_ACK_TOP_BIN_UPDATE 0x103
#define NET_CMD_ACK_TOPBACKUP_UPDATE 0x104
#define NET_CMD_ACK_BOT_BIN_UPDATE 0x105
#define NET_CMD_ACK_BOTBACKUP_UPDATE 0x106
#define NET_CMD_ACK_LINUX_APP_UPDATE 0x107
#define NET_CMD_ACK_LAPPBACKUP_UPDATE 0x108
#define NET_CMD_ACK_READ_CONFIG 0x109
#define NET_CMD_ACK_WRITE_CONFIG 0x10A
#define NET_CMD_ACK_TOP_PARA_UPDATE 0x10B
#define NET_CMD_ACK_BOT_PARA_UPDATE 0x10C
#define NET_CMD_ACK_TOP_FLASH_READ 0x10D
#define NET_CMD_ACK_TOP_FLASH_WRITE 0x10E
#define NET_CMD_ACK_TOP_READ_REGISTER 0x10F
#define NET_CMD_ACK_TOP_WRITE_REGISTER 0x110
#define NET_CMD_ACK_TOP_GET_INTENSITY 0x111
#define NET_CMD_ACK_END 0x150

typedef struct
{
  quint32 frameFlag;
  quint32 length;
  quint32 cmd;
  quint32 checkSum;
} NET_FRAME_HEAD;

typedef struct configPara
{
  quint8 sn[6];
  quint8 mac[6];

  quint8 ipLocal[4];
  quint8 ipRemote[4];

  quint16 msopPort;
  quint16 difopPort;

  quint16 res[12];

} configPara;

quint16 checkSum(NET_FRAME_HEAD frameHead)
{
  quint32 sum = 0;

  sum += frameHead.frameFlag & 0xFFFF;
  sum += (frameHead.frameFlag >> 16) & 0xFFFF;

  sum += frameHead.length & 0xFFFF;
  sum += (frameHead.length >> 16) & 0xFFFF;

  sum += frameHead.cmd & 0xFFFF;
  sum += (frameHead.cmd >> 16) & 0xFFFF;

  sum = (sum >> 16) + (sum & 0xFFFF);

  return static_cast<quint16>(~sum);
}

QByteArray frameHeadPack(quint32 cmd, quint32 length)
{
  QByteArray frameHeadBytes;

  NET_FRAME_HEAD frameHead;

  frameHead.frameFlag = FRAME_FLAG;
  frameHead.cmd = cmd;
  frameHead.length = length;
  frameHead.checkSum = checkSum(frameHead);

  frameHeadBytes.append(reinterpret_cast<char*>(&frameHead), sizeof(frameHead));

  return frameHeadBytes;
}

#endif  // COMMON_H
