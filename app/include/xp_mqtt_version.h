#ifndef __XP_MQTT_VERSION_H__
#define __XP_MQTT_VERSION_H__

/***********************************************************************************/
//                  xp_mqtt版本    
/***********************************************************************************/
//#define XP_MQTT_VER_STR     "xp_mqtt_v1.0.0"
/*
更新内容:
1、SDK版本: ESP8266_RTOS_SDK_2.0.0_740857
2、首次发布。
*/

//************************************************************************
//#define XP_MQTT_VER_STR     "xp_mqtt_v1.1.0"
/*
更新内容:
1、修改xp_mqtt_err_code.h为xp_mqtt_state_code.h，把xp_mqtt_task事件定义移到xp_mqtt_state_code.h中。
2、增加提交订阅表API(XP_MQTT_TopicTable_Submit)，把订阅和去订阅做成提交订阅表的形式。
3、增加订阅表状态(用来表示执行状况)
    TABLE_STATE_IDLE = 0, //订阅表空闲状态
    TABLE_STATE_CHANGE,   //订阅表有数据变更(根据订阅项的nextState做出变更)
    TABLE_STATE_IMPORT,   //订阅表导入(用于网络连接成功后)(根据订阅项的state做出操作)
4、增加xp_mqtt_updata_topicTable内部函数做mqtt主题表的订阅和去订阅处理
5、增加数据接收事件	
    XP_MQTT_EVENT_RECEIVE_SUCC,     //数据接收成功
    XP_MQTT_EVENT_RECEIVE_FAIL,     //数据接收失败
6、增加mqtt信息发布事件
    XP_MQTT_EVENT_PUBLISHING,       //mqtt信息发布中
    XP_MQTT_EVENT_PUBLISHING_SUCC,  //发布成功
    XP_MQTT_EVENT_PUBLISHING_FAIL,  //发布失败
7、增加XP_MQTT_SendMsg的发送模式
    (1)阻塞发布(要等待xp_mqtt处理结果)
    (2)无阻发布(无需等待，可以在回调事件中得到处理结果))
8、把XP_MQTT_BROKER_HOST，XP_MQTT_BROKER_PORT，XP_MQTT_COMMAND_TIMEOUT 
移出xp_mqtt_config.h，在XP_MQTT_CONFIG做赋值输入
*/

//************************************************************************
//#define XP_MQTT_VER_STR     "xp_mqtt_v1.1.1"
/*
更新内容:
1、修改阻塞发布使用队列的形式
*/
    
//************************************************************************
//#define XP_MQTT_VER_STR     "xp_mqtt_v1.1.2"
/*
更新内容:
1、优化主题表更新的触发，传入空操作不会触发主题表更新。
2、优化xp_mqtt_updata_topicTable函数(xp_mqtt内部函数)
    (1)进行"导入"更新前先判断是否有主题项要更新，若无则直接退出，不产生任何事件。        
3、修改xp_mqtt_task任务堆栈为640
*/

//************************************************************************
#define XP_MQTT_VER_STR     "xp_mqtt_v1.1.3"
/*
更新内容:
1、增加"topic项操作状态"
   (1)ITEM_HANDLE_ADD_UNSUB //该项进行增加操作(topic加入过滤器，但不向borker订阅)。用于解决接收onenet平台的下发指令。
2、增加XP_MQTT_ITEM_STATE(主题使用)
*/





#endif

