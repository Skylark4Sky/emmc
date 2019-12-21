/*
* _COPYRIGHT_
*
* File Name: gisunlink_network_module.c
* System Environment: JOHAN-PC
* Created Time:2019-05-16
* Author: johan
* E-mail: johaness@qq.com
* Description: 
*
*/

#include <stdio.h>
#include <string.h>
#include "gisunlink.h"
#include "gisunlink_network_module.h"

#define GISUNLINK_PACKET_HEAD   0XAA
#define GISUNLINK_PACKET_TAIL   0XBB
#define GISUNLINK_PACKET_HEAD_SIZE          1
#define GISUNLINK_PACKET_LEN_SIZE           2
#define GISUNLINK_PACKET_FLOW_SIZE          4
#define GISUNLINK_PACKET_DIR_SIZE           1
#define GISUNLINK_PACKET_CMD_SIZE           1
#define GISUNLINK_PACKET_CHKSUM_SIZE        2
#define GISUNLINK_PACKET_TAIL_SIZE          1

#define GISUNLINK_PACKET_HEAD_TAIL_SIZE     2

#define UART_PACKET_NO_HEAD_AND_TAIL_SIZE (GISUNLINK_PACKET_LEN_SIZE + GISUNLINK_PACKET_FLOW_SIZE + GISUNLINK_PACKET_DIR_SIZE + GISUNLINK_PACKET_CMD_SIZE + GISUNLINK_PACKET_CHKSUM_SIZE)

#define UART_PACKET_BUF_MAX_LEN (512)
#define UART_PACKET_BUF_MIN_LEN (GISUNLINK_PACKET_HEAD_TAIL_SIZE + UART_PACKET_NO_HEAD_AND_TAIL_SIZE)

#define UART_PACKET_CMD_OFFSET (GISUNLINK_PACKET_HEAD_SIZE + GISUNLINK_PACKET_LEN_SIZE + GISUNLINK_PACKET_FLOW_SIZE + GISUNLINK_PACKET_DIR_SIZE)
#define UART_PACKET_DATA_OFFSET (GISUNLINK_PACKET_HEAD_SIZE + GISUNLINK_PACKET_LEN_SIZE + GISUNLINK_PACKET_FLOW_SIZE + GISUNLINK_PACKET_DIR_SIZE + GISUNLINK_PACKET_CMD_SIZE)

#define SET_NEXT_OFFSET(cur_offset,size) (cur_offset) += (size)
#define SET_NEXT_LEN(cur_length,length) (cur_length) = (length)
#define CLEAR_RECV_MODE(mode,net_len,offset) (mode) = GISUNLINK_RECV_HEAD; (net_len) = 0; (offset)++;
#define CPOY_DATA(src,dst,offset,len) memcpy(src + offset,dst,len);offset += len; 

enum {
	GISUNLINK_RECV_HEAD,
	GISUNLINK_RECV_LEN,
	GISUNLINK_RECV_FLOW,
	GISUNLINK_RECV_DIR,
	GISUNLINK_RECV_CMD,
	GISUNLINK_RECV_DATA,
	GISUNLINK_RECV_CHECK_SUM,
	GISUNLINK_RECV_TAIL,
};

typedef enum {
	DMA_TX_CHANNEL,
	DMA_RX_CHANNEL,
} USART_DMA_CHANNEL_ID;

typedef struct{	
	uint8_t RxBuffer[USART_BUF_DIMENSION][USART_RX_BUFFER_SIZE];
	uint16_t RxEndIndex[USART_BUF_DIMENSION];
	FlagStatus RxEndFlag[USART_BUF_DIMENSION];
	uint8_t RxDimension;
}Usart_RecvBuf;

typedef struct _gisunlink_network_module_ctrl {
	USART_ID uartID;
	uint32_t flowID;		//全局流控ID
	uint32_t req_id;		//请求ID
	USART_TypeDef *USART;
	DMA_Channel_TypeDef *tx_channel;
	DMA_Channel_TypeDef *rx_channel;
	uint8_t tx_buffer[USART_TX_BUFFER_SIZE];
	gisunlink_recv_frame recv;
} gisunlink_network_module_ctrl;

//提交设备状态
typedef struct _gisunlink_post_info {
	uint8_t id_total;						//有多少个端口 
	uint8_t signal;							//当前网络信号值
	charge_com *comList[MAX_COM_QUANTITY];	//端口数据 顺序排放
	uint32_t energyOffset[MAX_COM_QUANTITY];	//端口数据 顺序排放
	uint16_t icReset[MAX_COM_QUANTITY];				//IC reset
} gisunlink_post_info;

typedef enum {
	MQTT_PUBLISH_NOACK,
	MQTT_PUBLISH_NEEDACK,
} MQTT_PUBLISH_ACK;

static Usart_RecvBuf RecvBuf;
static gisunlink_network_module_ctrl module_ctrl;
static gisunlink_frame frame_construction;
static gisunlink_frame post_construction;

typedef void GISUNLINK_MESSAGE_CALLBACK(gisunlink_recv_frame *frame);	

static uint8_t gisunlink_network_module_uart_config(gisunlink_network_module_ctrl *module_ctrl, int BaudRate,uint16_t WordLength,uint8_t Priority) {
	uint16_t uart_tx_io = 0;uint16_t uart_rx_io = 0;
	GPIO_TypeDef *GPIO_TYPE = NULL;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	switch(module_ctrl->uartID) {
		case USE_USART1:
			GPIO_TYPE = GPIOA;
			uart_tx_io = GPIO_Pin_9; uart_rx_io = GPIO_Pin_10;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);

			module_ctrl->USART = USART1;
			NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			
			break;
		case USE_USART2:
			GPIO_TYPE = GPIOA;
			uart_tx_io = GPIO_Pin_2; uart_rx_io = GPIO_Pin_3;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		

			module_ctrl->USART = USART2;
			NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
			break;
		case USE_USART3:
			GPIO_TYPE = GPIOB;
			uart_tx_io = GPIO_Pin_10; uart_rx_io = GPIO_Pin_11;	
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  
			module_ctrl->USART = USART3;		
			NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
			break;		
		default:
			return 1;
	}

	USART_DeInit(module_ctrl->USART);

	//USART_TX
	GPIO_InitStructure.GPIO_Pin = uart_tx_io;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIO_TYPE, &GPIO_InitStructure);

	//USART_RX	
	GPIO_InitStructure.GPIO_Pin = uart_rx_io;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIO_TYPE, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = BaudRate;								
	USART_InitStructure.USART_WordLength = WordLength;						
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				
	USART_InitStructure.USART_Parity = USART_Parity_No;					
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(module_ctrl->USART, &USART_InitStructure); 					

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Priority;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_Init(&NVIC_InitStructure);																	

	USART_ClearFlag(module_ctrl->USART, USART_FLAG_TC);	
	USART_ITConfig(module_ctrl->USART, USART_IT_TC, ENABLE);
	USART_ITConfig(module_ctrl->USART, USART_IT_IDLE, ENABLE);

	USART_Cmd(module_ctrl->USART, ENABLE);  
	return 0;
}

static uint8_t gisunlink_network_module_uart_dma_config(gisunlink_network_module_ctrl *module_ctrl) {
	DMA_InitTypeDef DMA_InitStructure;

	switch(module_ctrl->uartID) {
		case USE_USART1:
			module_ctrl->tx_channel = DMA1_Channel4; module_ctrl->rx_channel = DMA1_Channel5;
			break;
		case USE_USART2:
			module_ctrl->tx_channel = DMA1_Channel7; module_ctrl->rx_channel = DMA1_Channel6;
			break;
		case USE_USART3:
			module_ctrl->tx_channel = DMA1_Channel2; module_ctrl->rx_channel = DMA1_Channel3;
			break;		
		default:
			return 1;
	}

	if((u32)module_ctrl->tx_channel > (u32)DMA2) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);//DMA2时钟使能
	}else {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//DMA1时钟使能
	}

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&module_ctrl->USART->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_DeInit(module_ctrl->rx_channel);
	DMA_DeInit(module_ctrl->tx_channel);

	//设置接收的DMA
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Init(module_ctrl->rx_channel, &DMA_InitStructure);

	//设置发送的DMA
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_Init(module_ctrl->tx_channel, &DMA_InitStructure);

	//使能DMA串口功能
	USART_DMACmd(module_ctrl->USART, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);
	return 0;
}

static void gisunlink_network_module_set_dma_addr(gisunlink_network_module_ctrl * module_ctrl,uint8_t channel,uint32_t address, u16 size) {
	DMA_Channel_TypeDef *dma_channel = NULL;
	if(module_ctrl) {
		switch(channel) {
			case DMA_RX_CHANNEL:
				dma_channel = module_ctrl->rx_channel;
				break;
			case DMA_TX_CHANNEL:
				dma_channel = module_ctrl->tx_channel;
				while(DMA_GetCurrDataCounter(dma_channel));
				break;
		}

		if(dma_channel) {
			DMA_Cmd(dma_channel, DISABLE);
			dma_channel->CNDTR = size;
			dma_channel->CMAR = address;
			DMA_Cmd(dma_channel, ENABLE);
		}
	}
	return;
}

uint8_t gisunlink_network_module_init(USART_ID UsartID) {
	uint8_t ret = 1;
	uint8_t *dma_address = NULL;
	module_ctrl.uartID = UsartID; 
	module_ctrl.flowID = 0;
	module_ctrl.req_id = 0;	
	module_ctrl.recv.mode = GISUNLINK_RECV_HEAD;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	if(gisunlink_network_module_uart_config(&module_ctrl,115200,USART_WordLength_8b,5) == 0) {
		gisunlink_network_module_uart_dma_config(&module_ctrl);

		RecvBuf.RxDimension = 0;
		RecvBuf.RxDimension %= USART_BUF_DIMENSION;

		dma_address = RecvBuf.RxBuffer[RecvBuf.RxDimension];
		gisunlink_network_module_set_dma_addr(&module_ctrl,DMA_RX_CHANNEL,(uint32_t)dma_address,USART_RX_BUFFER_SIZE);
		ret = 0;
	}
	return ret;
}

static uint16_t gisunlink_check_sum(uint8_t *data,uint16_t data_len) {
	uint32_t check_sum = 0;
	while(data_len--) {
		check_sum += *data++;
	}
	while (check_sum >> 16) {
		check_sum = (check_sum >> 16) + (check_sum & 0xffff);
	}
	return (uint16_t)(~check_sum);
}

static gisunlink_frame *gisunlink_create_frame(uint8_t cmd,uint32_t flow_id,uint8_t flow_dir,uint8_t *data, uint16_t data_len) {
	gisunlink_frame *frame = &frame_construction;
	uint8_t *data_offset = NULL;
	uint8_t *chk_buf = NULL;
	uint16_t chk_len = 0;
	uint16_t chk_sum = 0;
	uint16_t raw_size = UART_PACKET_BUF_MIN_LEN + data_len;	

	if(raw_size > UART_PACKET_BUF_MAX_LEN) {
		return NULL;
	}

	frame->len = raw_size;

	data_offset = frame->buf;

	*(data_offset++) = GISUNLINK_PACKET_HEAD;

	*(data_offset++) = ((raw_size - GISUNLINK_PACKET_HEAD_TAIL_SIZE) & 0xFF);
	*(data_offset++) = (((raw_size - GISUNLINK_PACKET_HEAD_TAIL_SIZE) >> 8) & 0xFF);

	*(data_offset++) = (flow_id & 0xFF);
	*(data_offset++) = ((flow_id >> 8) & 0xFF);
	*(data_offset++) = ((flow_id >> 16) & 0xFF);
	*(data_offset++) = ((flow_id >> 24) & 0xFF);
	*(data_offset++) = flow_dir;
	*(data_offset++) = cmd;

	if(data_len) {
		memcpy(data_offset,data,data_len);
		data_offset += data_len;
	}

	chk_buf = (frame->buf + GISUNLINK_PACKET_HEAD_SIZE);
	chk_len = frame->len - GISUNLINK_PACKET_HEAD_TAIL_SIZE - GISUNLINK_PACKET_CHKSUM_SIZE;
	chk_sum = gisunlink_check_sum(chk_buf,chk_len);
	*(data_offset++) = (chk_sum & 0xFF);
	*(data_offset++) = ((chk_sum >> 8) & 0xFF);

	*(data_offset) = GISUNLINK_PACKET_TAIL;
	return frame;
}

uint16_t gisunlink_network_module_write(uint8_t cmd,uint8_t *buffer, uint16_t size) {
	gisunlink_frame *frame = NULL;
	if(size > (USART_TX_BUFFER_SIZE - UART_PACKET_BUF_MIN_LEN)) {
		return 0;
	}

	frame = gisunlink_create_frame(cmd,++module_ctrl.flowID,GISUNLINK_COMM_REQ,buffer,size);

	if(frame) {
		while(DMA_GetCurrDataCounter(module_ctrl.tx_channel));
		memcpy(module_ctrl.tx_buffer,frame->buf,frame->len);
		gisunlink_network_module_set_dma_addr(&module_ctrl,DMA_TX_CHANNEL,(uint32_t)module_ctrl.tx_buffer,frame->len);		
		return frame->len;		
	}
	return 0;
}

uint16_t gisunlink_network_module_respond(uint32_t id,uint8_t cmd,uint8_t *buffer, uint16_t size) {
	gisunlink_frame *frame = NULL;
	if(size > (USART_TX_BUFFER_SIZE - UART_PACKET_BUF_MIN_LEN)) {
		return 0;
	}	

	frame = gisunlink_create_frame(cmd,id,GISUNLINK_COMM_RES,buffer,size);
	if(frame) {
		while(DMA_GetCurrDataCounter(module_ctrl.tx_channel));
		memcpy(module_ctrl.tx_buffer,frame->buf,frame->len);
		gisunlink_network_module_set_dma_addr(&module_ctrl,DMA_TX_CHANNEL,(uint32_t)module_ctrl.tx_buffer,frame->len);		
		return frame->len;		
	}
	return 0;	
}

gisunlink_frame *gisunlink_struct_to_bytes(uint8_t cmd, uint8_t subcmd, void *structptr) {	
	gisunlink_frame *frame = NULL;
	uint8_t *data_offset = NULL;
	gisunlink_post_info *post_info = NULL;
	
	if(cmd == GISUNLINK_TASK_CONTROL && structptr != NULL) {
		uint8_t index = 0;
		charge_com *com = NULL;
		
		frame = &post_construction;

		data_offset = frame->buf;
		post_info = (gisunlink_post_info *)structptr;
		
		switch(subcmd) {
			case GISUNLINK_START_CHARGE:		//开始充电
			case GISUNLINK_CHARGEING:				//充电中
			case GISUNLINK_CHARGE_FINISH:		//充电完成
			case GISUNLINK_CHARGE_LEISURE:	//空闲中	
			case GISUNLINK_CHARGE_BREAKDOWN://充电故障
			case GISUNLINK_CHARGE_NO_LOAD:	//空载充电			
			case GISUNLINK_UPDATE_FIRMWARE:	//固件升级		
			case GISUNLINK_COM_UPDATE:			//端口数据更新
			case GISUNLINK_STOP_CHARGE:			//停止充电
				{					
					*(data_offset++) = subcmd; //拷贝子命令			
					
					if(subcmd == GISUNLINK_CHARGEING || subcmd == GISUNLINK_CHARGE_LEISURE) {
						*(data_offset++) = MQTT_PUBLISH_NOACK;
					} else {
						*(data_offset++) = MQTT_PUBLISH_NEEDACK;						
					}


					*(data_offset++) = post_info->signal;	//拷贝网络信号值	
					*(data_offset++) = post_info->id_total; //拷贝端口数量	
					
					for(index = 0; index < post_info->id_total; index++) {
						com = post_info->comList[index];								
						*(data_offset++) = com->id;
					}
					
					for(index = 0; index < post_info->id_total; index++) {
						
						uint32_t used_energyOffset = 0;
						uint16_t icReset = 0;
						
						com = post_info->comList[index];		
						used_energyOffset = com->used + post_info->energyOffset[index];						//已用多少度电;
						icReset = post_info->icReset[index];
 						
						*(data_offset++) = (com->token & 0xFF);
						*(data_offset++) = ((com->token >> 8) & 0xFF);
						*(data_offset++) = ((com->token >> 16) & 0xFF);
						*(data_offset++) = ((com->token >> 24) & 0xFF);		
						
						*(data_offset++) = (com->total & 0xFF);
						*(data_offset++) = ((com->total >> 8) & 0xFF);
						*(data_offset++) = ((com->total >> 16) & 0xFF);
						*(data_offset++) = ((com->total >> 24) & 0xFF);				

						*(data_offset++) = (used_energyOffset & 0xFF);
						*(data_offset++) = ((used_energyOffset >> 8) & 0xFF);
						*(data_offset++) = ((used_energyOffset >> 16) & 0xFF);
						*(data_offset++) = ((used_energyOffset >> 24) & 0xFF);	
						
						*(data_offset++) = (com->ptime & 0xFF);
						*(data_offset++) = ((com->ptime >> 8) & 0xFF);
						*(data_offset++) = ((com->ptime >> 16) & 0xFF);
						*(data_offset++) = ((com->ptime >> 24) & 0xFF);						
						
						*(data_offset++) = (com->utime & 0xFF);
						*(data_offset++) = ((com->utime >> 8) & 0xFF);
						*(data_offset++) = ((com->utime >> 16) & 0xFF);
						*(data_offset++) = ((com->utime >> 24) & 0xFF);	
						
						*(data_offset++) = (com->cur_current & 0xFF);						//当前充电电流
						*(data_offset++) = ((com->cur_current >> 8) & 0xFF);
						*(data_offset++) = ((com->cur_current >> 16) & 0xFF);
						*(data_offset++) = ((com->cur_current >> 24) & 0xFF);	
						
						*(data_offset++) = (icReset & 0xFF);
						*(data_offset++) = ((icReset >> 8) & 0xFF);
						*(data_offset++) = (com->max_current & 0xFF);
						*(data_offset++) = ((com->max_current >> 8) & 0xFF);
												
						//操作完清0
						com->cur_current = 0;
						
						*(data_offset++) = (com->breakdown & 0xFF);								
						*(data_offset++) = (com->enable & 0xFF);	
						*(data_offset++) = (com->cur_behavior & 0xFF);
					}

					frame->len = data_offset - frame->buf;
					break;				
				}
		}
	}
	return frame;
}

uint8_t gisunlink_bytes_to_struct(uint8_t cmd,void *structure,uint8_t *buffer, uint16_t size) {
	uint8_t ret = 1;
	if(structure && size) {
		if(cmd == GISUNLINK_TASK_CONTROL) {
			uint8_t subcmd = *buffer;
			uint8_t *data = buffer+1;
			switch(subcmd) {
				case GISUNLINK_CHARGE_TASK: 
					{
						gisunlink_charge_task *charge_task = (gisunlink_charge_task *)structure;	
						charge_task->id = data[0];
						charge_task->token = data[1] + (data[2] << 8) + (data[3] << 16) + (data[4] << 24);	
						charge_task->total = data[5] + (data[6] << 8) + (data[7] << 16) + (data[8] << 24);	
						charge_task->max_current = data[9] + (data[10] << 8) + (data[11] << 16) + (data[12] << 24);	
						charge_task->ptime = data[13] + (data[14] << 8) + (data[15] << 16) + (data[16] << 24);
					}
					break;					
				case GISUNLINK_DEVIDE_STATUS: 
					{
						gisunlink_device_status *device_status = (gisunlink_device_status *)structure;	
						device_status->id = data[0];
					}
					break;
				case GISUNLINK_EXIT_CHARGE_TASK: 
					{
						gisunlink_stop_charge_task *stop_charge = (gisunlink_stop_charge_task *)structure;
						stop_charge->id = data[0];
						stop_charge->token = data[1] + (data[2] << 8) + (data[3] << 16) + (data[4] << 24);
						stop_charge->force_stop = data[5];					
					}
					break;
				case GISUNLINK_SET_CONFIG: 
					{
						gisunlink_set_config *set_config = (gisunlink_set_config *)structure;
						set_config->allow_zero_balance = data[0];					
					}
					break;
			}						
		} else {
			switch(cmd) {
				case GISUNLINK_DEV_FW_INFO: 
					{			
						uint16_t packet_size = (sizeof(uint32_t) + sizeof(uint32_t));
						if(size > packet_size) {
							gisunlink_device_firmware_info *firmware = (gisunlink_device_firmware_info *)structure;			
							firmware->version = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);				
							firmware->size = buffer[4] + (buffer[5] << 8) + (buffer[6] << 16) + (buffer[7] << 24);					
							firmware->md5 = buffer+8;	
							ret = 0;
						}
						break;				
					}
				case GISUNLINK_DEV_FW_TRANS: 
					{
						uint16_t packet_size = (sizeof(uint16_t) + sizeof(uint16_t));
						if(size > packet_size) {
							gisunlink_device_firmware_transfer *transfer = (gisunlink_device_firmware_transfer *)structure;
							transfer->offset = buffer[0] + (buffer[1] << 8);
							transfer->size = buffer[2] + (buffer[3] << 8);
							transfer->data = buffer+4;
							ret = 0;
						}
						break;				
					}
				case GISUNLINK_DEV_FW_READY:
					break;
			}					
		}
	}
	return ret;
}

static void gisunlink_process_frame(GISUNLINK_MESSAGE_CALLBACK *msgCb,gisunlink_recv_frame *frame) {
	uint16_t chk_sum = 0;
	if(msgCb && frame) {
		if(module_ctrl.req_id != frame->id) { //过滤重复包
			module_ctrl.req_id = frame->id;
			chk_sum = gisunlink_check_sum(frame->buffer,frame->len - GISUNLINK_PACKET_HEAD_TAIL_SIZE - GISUNLINK_PACKET_CHKSUM_SIZE);
			if(frame->chk_sum == chk_sum) {
				msgCb(frame);
			}			
		}
	}
}

static void gisunlink_network_module_parse_frame(GISUNLINK_MESSAGE_CALLBACK *msgCb,uint8_t *buffer,uint16_t size) {
	uint16_t offset = 0; uint16_t next_len = 0;uint16_t copy_offset = 0;
	uint8_t *data = NULL;

	gisunlink_recv_frame *frame = &module_ctrl.recv;
	if(msgCb && size) {	
		while(offset < size) {
			if(next_len + offset > size) {
				break;
			}
			data = buffer + offset;
			switch(frame->mode) {
				case GISUNLINK_RECV_HEAD:
					copy_offset = 0;
					if(data[0] == GISUNLINK_PACKET_HEAD) {						
						SET_NEXT_LEN(next_len,GISUNLINK_PACKET_LEN_SIZE);
						SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_HEAD_SIZE);
						frame->mode = GISUNLINK_RECV_LEN;
					} else {
						CLEAR_RECV_MODE(frame->mode,next_len,offset);
					}
					break;
				case GISUNLINK_RECV_LEN:	
					frame->len = data[0] + (data[1] << 8);
					frame->len += GISUNLINK_PACKET_HEAD_TAIL_SIZE;
					if(frame->len > UART_PACKET_BUF_MAX_LEN || frame->len < UART_PACKET_BUF_MIN_LEN) {
						CLEAR_RECV_MODE(frame->mode,next_len,offset);
					} else {						
						frame->data_len = frame->len - UART_PACKET_BUF_MIN_LEN;
						CPOY_DATA(frame->buffer,data,copy_offset,GISUNLINK_PACKET_LEN_SIZE);
						SET_NEXT_LEN(next_len,GISUNLINK_PACKET_FLOW_SIZE);
						SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_LEN_SIZE);
						frame->mode = GISUNLINK_RECV_FLOW;
					}
					break;
				case GISUNLINK_RECV_FLOW:
					frame->id = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);						
					CPOY_DATA(frame->buffer,data,copy_offset,GISUNLINK_PACKET_FLOW_SIZE);				
					SET_NEXT_LEN(next_len,GISUNLINK_PACKET_DIR_SIZE);						
					SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_FLOW_SIZE);				
					frame->mode = GISUNLINK_RECV_DIR;
					break;
				case GISUNLINK_RECV_DIR:
					frame->dir = data[0];
					if(frame->dir == GISUNLINK_COMM_REQ || frame->dir == GISUNLINK_COMM_RES) {							
						SET_NEXT_LEN(next_len,GISUNLINK_PACKET_CMD_SIZE);
						CPOY_DATA(frame->buffer,data,copy_offset,GISUNLINK_PACKET_DIR_SIZE);
						SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_DIR_SIZE);				
						frame->mode = GISUNLINK_RECV_CMD;
					} else {
						CLEAR_RECV_MODE(frame->mode,next_len,offset);						
					}
					break;
				case GISUNLINK_RECV_CMD:
					frame->cmd = data[0];
					switch(frame->cmd) {
						case GISUNLINK_NETWORK_STATUS:
						case GISUNLINK_NETWORK_RESET:
						case GISUNLINK_NETWORK_RSSI:
						case GISUNLINK_DEV_FW_INFO:
						case GISUNLINK_DEV_FW_TRANS:
						case GISUNLINK_DEV_FW_READY:
						case GISUNLINK_DEV_SN:
						case GISUNLINK_TASK_CONTROL:			
						case GISUNLINK_HW_SN:
						case GISUNLINK_FIRMWARE_VERSION:
							SET_NEXT_LEN(next_len,frame->data_len);
							CPOY_DATA(frame->buffer,data,copy_offset,GISUNLINK_PACKET_CMD_SIZE);
							SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_CMD_SIZE);								
							frame->mode = GISUNLINK_RECV_DATA;							
							break;
						default:
							CLEAR_RECV_MODE(frame->mode,next_len,offset);	
					}
					break;
				case GISUNLINK_RECV_DATA:
					if(frame->data_len) {
						frame->data = frame->buffer + copy_offset;
						CPOY_DATA(frame->buffer,data,copy_offset,frame->data_len);					
					} else {
						frame->data = NULL;
					}					
					
					if(frame->cmd == GISUNLINK_NETWORK_STATUS) {
						
						frame->cmd = frame->cmd;
					}
					
					SET_NEXT_LEN(next_len,GISUNLINK_PACKET_CHKSUM_SIZE);
					SET_NEXT_OFFSET(offset,frame->data_len);		
					frame->mode = GISUNLINK_RECV_CHECK_SUM;		
					break;
				case GISUNLINK_RECV_CHECK_SUM:
					frame->chk_sum = data[0] + (data[1] << 8);
					SET_NEXT_LEN(next_len,GISUNLINK_PACKET_TAIL_SIZE);
					SET_NEXT_OFFSET(offset,GISUNLINK_PACKET_CHKSUM_SIZE);		
					frame->mode = GISUNLINK_RECV_TAIL;						
					break;
				case GISUNLINK_RECV_TAIL:
					if(data[0] == GISUNLINK_PACKET_TAIL) {
						gisunlink_process_frame(msgCb,frame);
					}
					CLEAR_RECV_MODE(frame->mode,next_len,offset);	
					break;
			}
		}
	} 
	return;
}

void gisunlink_network_module_read(void(*msgCb)(gisunlink_recv_frame *frame)) {
	uint8_t loop = 0;
	uint16_t data_len = 0;
	uint8_t *dma_address = NULL;
	for(loop = 0; loop < USART_BUF_DIMENSION; loop++) {
		data_len = (USART_RX_BUFFER_SIZE - RecvBuf.RxEndIndex[loop]);
		if(RecvBuf.RxEndFlag[loop] == SET) {
			dma_address = RecvBuf.RxBuffer[loop];
			gisunlink_network_module_parse_frame(msgCb,dma_address,data_len);
			RecvBuf.RxEndFlag[loop] = RESET;
			RecvBuf.RxEndIndex[loop] = 0;
		}
	}
}

void gisunlink_network_single_com_post(gisunlink *global, uint8_t comID, uint8_t behavior) {

	gisunlink_frame *frame = NULL;
	gisunlink_post_info post_info = {0};	
	
	post_info.signal = global->system.signal;
	post_info.id_total = SINGLE_RECORD;
	post_info.comList[0] = &global->comList[comID];	
	post_info.energyOffset[0] = global->energyOffset[comID];
	post_info.icReset[0] = global->icReset[comID];
	
		//转换端口数据为字节
	frame = gisunlink_struct_to_bytes(GISUNLINK_TASK_CONTROL,behavior,&post_info);
	if(frame) {
			//传递给服务器
		gisunlink_network_module_write(GISUNLINK_TASK_CONTROL,frame->buf,frame->len);
	}	
}

void gisunlink_network_multi_com_post(gisunlink *global, uint8_t behavior) {

	uint8_t comID = 0;
	gisunlink_frame *frame = NULL;
	gisunlink_post_info post_info = {0};

	post_info.signal = global->system.signal;
	post_info.id_total = MULTI_RECORD;
	for(comID = 0; comID < MULTI_RECORD; comID++) {
		post_info.comList[comID] = &global->comList[comID];		
		post_info.energyOffset[comID] = global->energyOffset[comID];	
		post_info.icReset[comID] = global->icReset[comID];		
	}	
		
	//转换端口数据为字节
	frame = gisunlink_struct_to_bytes(GISUNLINK_TASK_CONTROL,behavior,&post_info);
	if(frame) {
		//传递给服务器
		gisunlink_network_module_write(GISUNLINK_TASK_CONTROL,frame->buf,frame->len);
	}	
}

void gisunlink_network_module_irq(void) {
	uint16_t temp;
	uint8_t *next_dma_address = NULL;
	if(USART_GetITStatus(module_ctrl.USART,USART_IT_IDLE)!= RESET) {
		temp = module_ctrl.USART->SR;
		temp = module_ctrl.USART->DR; 
		temp = temp;
		RecvBuf.RxEndFlag[RecvBuf.RxDimension] = SET;
		RecvBuf.RxEndIndex[RecvBuf.RxDimension] = DMA_GetCurrDataCounter(module_ctrl.rx_channel);
		RecvBuf.RxDimension++;
		RecvBuf.RxDimension %= USART_BUF_DIMENSION;
		next_dma_address = RecvBuf.RxBuffer[RecvBuf.RxDimension];
		gisunlink_network_module_set_dma_addr(&module_ctrl,DMA_RX_CHANNEL,(uint32_t)next_dma_address,USART_RX_BUFFER_SIZE);
		USART_ClearITPendingBit(module_ctrl.USART, USART_IT_IDLE);			
	}			

	if(USART_GetITStatus(module_ctrl.USART,USART_IT_TC)!= RESET) {
		DMA_Cmd(module_ctrl.tx_channel, DISABLE);
		module_ctrl.tx_channel->CNDTR = 0;
		USART_ClearITPendingBit(module_ctrl.USART, USART_IT_TC);
	} 
}
