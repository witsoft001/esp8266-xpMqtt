#ifndef __XP_MQTT_H__
#define __XP_MQTT_H__

/***********************************************************************************/
//                 头文件引用
/***********************************************************************************/
#include "c_types.h"
#include "mqtt/MQTTClient.h"
#include "xp_mqtt_state_code.h"
#include "xp_mqtt_config.h"


/***********************************************************************************/
//                 宏
/***********************************************************************************/
#ifndef IFA
#define IFA     ICACHE_FLASH_ATTR
#endif

/***********************************************************************************/
//                 枚举
/***********************************************************************************/
//XP_MQTT_SendMsg信息发送模式
typedef enum
{
    SEND_MODE_BLOCK = 0,    //阻塞模式
    SEND_MODE_UNBLOCK,      //无阻模式
}XP_MQTT_SEND_MODE;

//topic项操作状态
typedef enum
{
    ITEM_HANDLE_IDLE = 0, //该项操作为空(不进行操作)    
    ITEM_HANDLE_ADD,      //该项进行增加操作(订阅)
    ITEM_HANDLE_ADD_UNSUB,//该项进行增加操作(topic加入过滤器，但不向borker订阅)
    ITEM_HANDLE_DEL,      //该项进行删除操作(去订阅)
}XP_MQTT_ITEM_HANDLE;

/***********************************************************************************/
//                 数据结构
/***********************************************************************************/
//mqtt配置表
typedef struct
{
//MQTTPacket_connectData
    char* borkerHost;
    char* usermame;
    char* password;
    char* clientID;
    uint8 MQTTVersion;
    uint8 cleansession;
    uint16 borkerPort;
    uint16 keepAliveInterval;
    uint32 mqttCommandTimeout;
    
    //错误信息回调函数
    void (*xp_mqtt_error_cellback)( XP_MQTT_ERR_CODE err );
    //事件回调函数
    void (*xp_mqtt_event_cellback)( XP_MQTT_EVENT event );
}XP_MQTT_CONFIG;

//mqtt数据包(用于队列)
typedef struct
{
    XP_MQTT_SEND_MODE sendMode;
    uint8   qos;
    uint8   retained;
    uint8   dup; //发送次数
    
    uint8   topic[XP_MQTT_MAX_TOPICLEN+1];
    uint8   payload[XP_MQTT_MAX_PAYLOADLEN+1];
    uint16  payloadlen;
}MQTT_MSG;

//topic项，用于添加或删除订阅信息
typedef struct
{
    XP_MQTT_ITEM_HANDLE handle;         //要进行的操作
    uint8 qos;
    char topic[XP_MQTT_MAX_TOPICLEN+1];
}XP_MQTT_TOPIC_ITEM;


/***********************************************************************************/
//                 函数api
/***********************************************************************************/
/*****************************************************************************
 * 函 数 名  : Get_XP_MQTT_Version
 * 函数功能  : 返回当前版本号
 * 输入参数  : 无
 * 返 回 值  : 
                char *
 * 备    注  : 无
*****************************************************************************/
const char* IFA Get_XP_MQTT_Version( void );

/*****************************************************************************
 * 函 数 名  : XP_MQTT_START
 * 函数功能  : 导入 mqtt配置表 启动mqtt任务
 * 输入参数  : 
                config : mqtt配置表
 * 返 回 值  : 
                XP_MQTT_ERR_CODE: 错误码(0:成功, other:见"xp_mqtt_state_code.h")                
 * 备    注  : 
                XP_MQTT_START 与 XP_MQTT_CloseWait 的使用是一一对应的。
*****************************************************************************/
XP_MQTT_ERR_CODE IFA XP_MQTT_START( XP_MQTT_CONFIG *config );

/*****************************************************************************
 * 函 数 名  : XP_MATT_CloseWait
 * 函数功能  : 
                关闭mqtt task
 * 输入参数  : 无
 * 返 回 值  : 无
 * 备    注  : 
                XP_MATT_CloseWait 一定运行在 XP_MQTT_START 成功之后
*****************************************************************************/
void IFA XP_MQTT_CloseWait( void );

/*****************************************************************************
 * 函 数 名  : Get_XP_MQTT_ReceiveMsg
 * 函数功能  : 
               获取mqtt接收的数据包
 * 输入参数  : 
                msg     : 输出信息块   
                timeOut : 阻塞超时时间
 * 返 回 值  : 
               -1 : 为错误
                0 : 无数据获取
              n>0 : 为数据包数
 * 备    注  : 无

*****************************************************************************/
long IFA Get_XP_MQTT_ReceiveMsg( MQTT_MSG* msg, uint32 timeOut );

/*****************************************************************************
 * 函 数 名  : XP_MQTT_SendMsg
 * 函数功能  : 
                发布消息
 * 输入参数  : 
                mqttMsg :   消息项目                                
 * 返 回 值  : 
                XP_MQTT_ERR_CODE: 错误码(0:成功, other:见"xp_mqtt_state_code.h")                
 * 备    注  : 
                mqttMsg.sendMode:       数据发送模式
                    SEND_MODE_BLOCK:    阻塞模式(需要等待xp_mqtt_task返回操作结果)
                    SEND_MODE_UNBLOCK:  无阻模式(无需等待,由队列完成,可在事件回掉得到结果)
*****************************************************************************/
XP_MQTT_ERR_CODE IFA XP_MQTT_SendMsg( MQTT_MSG *mqttMsg );

/*****************************************************************************
 * 函 数 名  : XP_MQTT_TopicTable_DeleteAll
 * 函数功能  : 
                让主题表中正在使用的项,执行删除操作
 * 输入参数  : 无                
 * 返 回 值  : 
                XP_MQTT_ERR_CODE: 错误码(0:成功, other:见"xp_mqtt_state_code.h")                
 * 备    注  : 无
*****************************************************************************/
XP_MQTT_ERR_CODE IFA XP_MQTT_TopicTable_DeleteAll( void );

/*****************************************************************************
 * 函 数 名  : XP_MQTT_TopicTable_Submit
 * 函数功能  : 
                提交主题表操作
 * 输入参数  : 
                topics : 主题项目指针
                itemNum: 项目个数
                
 * 返 回 值  : 
                XP_MQTT_ERR_CODE: 错误码(0:成功, other:见"xp_mqtt_state_code.h")                
 * 备    注  : 无
*****************************************************************************/
XP_MQTT_ERR_CODE IFA XP_MQTT_TopicTable_Submit( XP_MQTT_TOPIC_ITEM* topics, uint8 itemNum );



#endif

