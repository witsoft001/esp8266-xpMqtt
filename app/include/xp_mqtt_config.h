#ifndef __XP_MQTT_CONFIG_H__
#define __XP_MQTT_CONFIG_H__

//注:要预留10K内存给这个xp_mqtt(好吃内存的说~~~)

/***********************************************************************************/
//                 DEBUG
/***********************************************************************************/
#define XP_MQTT_DEBUG   1
#if XP_MQTT_DEBUG
#include "driver.h"
#define m_printf( format, ... )      printf( "D: "format"\n", ##__VA_ARGS__ )
#else
#define m_printf( format, ... ) 
#endif

/***********************************************************************************/
//                 XP_MQTT任务设置
/***********************************************************************************/
#define XP_MQTT_TASK_STACK_DEPTEH   640     //xp_mqtt任务堆栈
#define XP_MQTT_TASK_PRIORITY       9       //xp_mqtt任务优先级

//队列数据包
//最大topic长度
#define XP_MQTT_MAX_TOPICLEN        70
//最大数据体长度
#define XP_MQTT_MAX_PAYLOADLEN      200
//数据发送队列项数
#define XP_MQTT_SEND_MSG_QUEUE_LEN  3
//数据接收队列项数
#define XP_MQTT_RCV_MSG_QUEUE_LEN   5

//可订阅主题个数
//(根据用户所需个数来定，这里使用的是mqttClient当前最大订阅数)
//#define XP_MQTT_SUBSCIBE_NUM    MAX_MESSAGE_HANDLERS       
//(这里固定为5)
#define XP_MQTT_SUBSCIBE_NUM        5   

#define XP_MQTT_BUF_SIZE            1024    //数据收发缓存区大小

#define XP_MQTT_YIELD_TIME          100     //会话阻塞时间(也是发包最小间隔)
#define XP_MQTT_RECONNECT_INTERVAL  1000    //mqtt重连间隔,单位ms

#endif

