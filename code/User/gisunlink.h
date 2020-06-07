/*
* _COPYRIGHT_
*
* File Name:gisunlink.h
* System Environment: JOHAN-PC
* Created Time:2019-08-13
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#ifndef __GISUNLINK_H__
#define __GISUNLINK_H__

#include "stm32f10x.h"
#include "md5.h"
#include "gisunlink_type.h"

#define FIRMWARE_VERSION "202006072150"

#ifdef __cplusplus
extern "C"
{
#endif

//端口信息
typedef struct _charge_com {
	volatile uint8_t id;													//ID
	volatile uint8_t enable;											//使能
	volatile uint32_t token;											//服务器下发的充电令牌
	volatile uint32_t total; 											//最大充电度数
	volatile uint32_t used;												//已充电度数
	volatile uint32_t ptime;											//最大可充电时长
	volatile uint32_t utime;											//已充电时长
	volatile uint32_t max_current;								//最大可充电电流
	volatile uint32_t cur_current;								//当前充电电流	
	volatile uint8_t breakdown;										//是否故障
	volatile uint8_t breakdown_time;							//故障检测时间
	volatile uint8_t cur_behavior;							  //当前行为
} charge_com, *Pcharge_com;

typedef struct _gisunlink_conf{
	volatile uint8_t init;
	volatile uint8_t mode;
	volatile uint8_t firmware_md5[FIRMWARE_MD5_SIZE];
	//volatile uint32_t firmware_version;
} gisunlink_conf, *Pgisunlink_conf;

//设备固件信息
typedef struct _gisunlink_firmware_info {
	uint32_t size;																//文件大小
	uint8_t  md5[FIRMWARE_MD5_SIZE];							//文件md5 长度32
} gisunlink_firmware_info;

typedef struct _gisunlink_firmware {
	MD5_CTX md5;
	uint8_t md5_digest[16];
	volatile gisunlink_firmware_info info;
	volatile uint16_t transfer_offset;
	volatile uint8_t transfer_timeout;
	volatile uint8_t transfer_status;
} gisunlink_firmware, *Pgisunlink_firmware;

typedef struct _gisunlink_system {
	uint8_t deviceSN[STM32_UNIQUEID_SIZE];							//STM UID
	volatile uint8_t get_deviceSn;											//读取设备串号标志
	volatile uint8_t network_connect;										//网络联网标志
	volatile uint8_t no_load_time;											//空载时间
	volatile uint8_t network_state;											//网络状态标志
	gisunlink_firmware firmware;												//固件升级相关
	volatile uint8_t post_tick;													//定时提交
	volatile uint8_t route_work;												//端口是否在工作 
	volatile uint32_t tick;															//系统滴答时间
	volatile uint32_t recvTime;									    		//包接收时间
	volatile uint32_t state_recvTime;									  //包接收时间	
	volatile uint8_t signal;														//网络信号强度
} gisunlink_system, *Pgisunlink_system;

//系统全局定义
typedef struct _gisunlink {
	volatile uint8_t system_update_flag;							//系统升级标志
	volatile uint32_t energyOffset[MAX_COM_QUANTITY];	//充电度保存
	volatile uint16_t icReset[MAX_COM_QUANTITY];			//IC复位
	volatile uint8_t breakdown_chk[MAX_COM_QUANTITY];
	charge_com comList[MAX_COM_QUANTITY];							//充电端口 
	gisunlink_conf conf;
	gisunlink_system system;
} gisunlink, *Pgisunlink;

//}volatile gisunlink, *Pgisunlink;

typedef struct _gisunlink_frame {
    uint16_t len;
    uint8_t buf[USART_TX_BUFFER_SIZE];
} gisunlink_frame;

//无线网络数据包
typedef struct _gisunlink_recv_frame {
	uint8_t mode;
	uint16_t len; 
	uint32_t id;
	uint8_t dir;
	uint8_t cmd;
	uint8_t buffer[USART_RX_BUFFER_SIZE];
	uint8_t *data; //数据指针
	uint16_t data_len;
	uint16_t chk_sum;
} gisunlink_recv_frame;

//充电任务下发
typedef struct _gisunlink_charge_task  {
	uint8_t id;								//端口号
	uint32_t token;							//服务器下发的充电令牌
	uint32_t total;							//最大充电度数 小数值放大4位 as: 1.663 -> 1663
	uint32_t ptime;							//最大充电时长(秒)
	uint32_t max_current;					//最大充电电流
} gisunlink_charge_task;

//查询终端当前状态
typedef struct _gisunlink_device_status {
	uint8_t id;								//端口号 com:0xFF 全部上报
} gisunlink_device_status;

//终止本次充电任务
typedef struct _gisunlink_stop_charge_task {
	uint8_t id;								//端口号 com
	uint32_t token;							//服务器下发的充电令牌	
	uint8_t force_stop;						//是否强制停止充电 force_stop=1,强制停止充电。force_stop=0， 停止充电，需判断token与控制器存储的token是否一致，如果不一致，返回停止失败
} gisunlink_stop_charge_task;

//设置设备配置
typedef struct _gisunlink_set_config {
	uint8_t allow_zero_balance;				//允许空负载通电n秒 (最大255秒)
} gisunlink_set_config;

//设备固件信息
typedef struct _gisunlink_device_firmware_info {
	uint32_t version;						//文件版本
	uint32_t size;							//文件大小
	uint8_t *md5;							//文件md5 长度32
} gisunlink_device_firmware_info;

//固件传输信息
typedef struct _gisunlink_device_firmware_transfer {
	uint16_t offset;						//偏移
	uint16_t size;							//大小
	uint8_t *data;							//数据
} gisunlink_device_firmware_transfer;

//命令请求状态
typedef enum {
	GISUNLINK_COMM_REQ,						//请求
	GISUNLINK_COMM_RES,						//回复
} COMM_TYPE;	

typedef enum {
	GISUNLINK_NETMANAGER_IDLE,              //空闲
	GISUNLINK_NETMANAGER_START,             //开始连接
	GISUNLINK_NETMANAGER_CONNECTING,        //连接中
	GISUNLINK_NETMANAGER_CONNECTED,         //已连接
	GISUNLINK_NETMANAGER_DISCONNECTED,      //断开连接
	GISUNLINK_NETMANAGER_RECONNECTING,      //重连中
	GISUNLINK_NETMANAGER_ENT_CONFIG,        //进入配对
	GISUNLINK_NETMANAGER_EXI_CONFIG,        //退出配对
	GISUNLINK_NETMANAGER_SAVE_CONFIG,       //保存配对信息
	GISUNLINK_NETMANAGER_TIME_SUCCEED,      //同步时钟成功
	GISUNLINK_NETMANAGER_TIME_FAILED,       //同步时钟失败
	GISUNLINK_NETMANAGER_CONNECTED_SER,			//已连上平台
	GISUNLINK_NETMANAGER_DISCONNECTED_SER,	//断开平台连接
	GISUNLINK_NETMANAGER_GSM_CONNECTED,		//GSM注册成功
	GISUNLINK_NETMANAGER_GSM_DISCONNECTED,	//GSM注册失败	
	GISUNLINK_NETMANAGER_UNKNOWN
} GISUNLINK_NETMANAGER_WORK_STATE;

//模组通信命令
typedef enum {
	GISUNLINK_NETWORK_STATUS = 0x05,		//网络当前状态
	GISUNLINK_NETWORK_RESET = 0x06,			//重设置网络链接 (仅仅针对wifi模块)
	GISUNLINK_NETWORK_RSSI = 0x07,			//网络信号强度

	GISUNLINK_DEV_FW_INFO = 0x08,				//设置固件升级版本
	GISUNLINK_DEV_FW_TRANS = 0x09,			//固件数据传输
	GISUNLINK_DEV_FW_READY = 0x0A,			//固件确认完成
	GISUNLINK_DEV_SN = 0x0B,						//固件确认完成	

	GISUNLINK_TASK_CONTROL = 0x0C,			//网络数据透传
	GISUNLINK_HW_SN = 0x0D,							//硬件串号
	GISUNLINK_FIRMWARE_VERSION = 0x0E,	//硬件固件号	
} GISUNLINK_MODULE_CMD;

typedef enum {
	//服务器下发
	GISUNLINK_CHARGE_TASK = 0x10,				//充电任务下发
	GISUNLINK_DEVIDE_STATUS = 0x11,			//查询终端状态
	GISUNLINK_EXIT_CHARGE_TASK = 0x12,	//终止本次充电任务
	GISUNLINK_SET_CONFIG = 0x13,				//下发配置设置
	GISUNLINK_RESTART = 0x1E,						//重启设备	
	//设备上报
	GISUNLINK_START_CHARGE = 0x14,			//开始充电
	GISUNLINK_CHARGEING = 0x15,					//充电中
	GISUNLINK_CHARGE_FINISH	= 0x16,			//充电完成
	GISUNLINK_CHARGE_LEISURE = 0x17,		//空闲中
	GISUNLINK_CHARGE_BREAKDOWN = 0x18,	//故障
	GISUNLINK_CHARGE_NO_LOAD = 0x19,		//空载	
	GISUNLINK_UPDATE_FIRMWARE = 0x1A,		//固件升级		
	GISUNLINK_COM_UPDATE = 0x1B,				//端口参数刷新		
	GISUNLINK_STOP_CHARGE = 0x1C,				//停止充电		
	GISUNLINK_COM_NO_UPDATE = 0x1D,			//端口参数没有刷新
} GISUNLINK_TASK_CONTROL_SUB_CMD;

enum {
	GISUNLINK_TRANSFER_FAILED = 0x00,
	GISUNLINK_NEED_UPGRADE = 0x63,			//需要升级
	GISUNLINK_NO_NEED_UPGRADE = 0x64,		//不需要升级
	GISUNLINK_FIRMWARE_CHK_OK = 0x89,  	//固件检查OK
	GISUNLINK_FIRMWARE_CHK_NO_OK = 0x90,//固件检查不OK
	GISUNLINK_DEVICE_TIMEOUT = 0xff,
};	

typedef enum {
	TIMER_UNKNOWN,
	TIMER_10S,
	TIMER_60S,
} TIMER_TYPE;

#ifdef __cplusplus
}
#endif

#endif //__GISUNLINK_H__
