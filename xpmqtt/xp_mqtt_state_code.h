#ifndef __XP_MQTT_STATE_CODE_H__
#define __XP_MQTT_STATE_CODE_H__

//xp_mqtt_task事件
typedef enum
{
    //xp mqtt事件起始ID
    XP_MQTT_EVENT_START__ = 0,   
    
    XP_MQTT_EVENT_NET_CONNECTING,   //网络连接中
    XP_MQTT_EVENT_NET_CONN_SUCC,    //网络连接成功
    XP_MQTT_EVENT_NET_CONN_FAIL,    //网络连接失败
    XP_MQTT_EVENT_NET_DISCONN,      //网络断开
    XP_MQTT_EVENT_NET_RECONN,       //网络重连
    
    XP_MQTT_EVENT_MQTT_CONNECTING,  //与mqtt服务器连接中
    XP_MQTT_EVENT_MQTT_CONN_SUCC,   //与mqtt服务器连接成功
    XP_MQTT_EVENT_MQTT_CONN_FAIL,   //与mqtt服务器连接失败
        
    XP_MQTT_EVENT_FIRST_CONN,       //第一次连上mqtt服务器

    XP_MQTT_EVENT_TOPICTABLE_UPDATING,      //主题表更新中
    XP_MQTT_EVENT_TOPICTABLE_UPDATA_SUCC,   //主题表更新成功
    XP_MQTT_EVENT_TOPICTABLE_UPDATA_FAIL,   //主题表更新失败

    XP_MQTT_EVENT_RECEIVE_SUCC,     //数据接收成功
    XP_MQTT_EVENT_RECEIVE_FAIL,     //数据接收失败

    XP_MQTT_EVENT_PUBLISHING,       //mqtt信息发布中
    XP_MQTT_EVENT_PUBLISH_SUCC,     //发布成功
    XP_MQTT_EVENT_PUBLISH_FAIL,     //发布失败
        
    XP_MQTT_EVENT_TASK_CLOSE,      //删除xp_mqtt任务前的最后一条事件

    //xp mqtt事件结束ID
    XP_MQTT_EVENT_END__,   
}XP_MQTT_EVENT;

//错误代码
typedef enum
{
    XP_MQTT_SUCCESS          =  0,   //成功    
    XP_MQTT_ERR_MALLOC_FAIL  = -1,   //申请内存失败	
    XP_MQTT_ERR_NO_START     = -2,   //未执行XP_MQTT_START,
    
//xpmqtt配置错误
    XP_MQTT_ERR_CONFIG_IS_NULL              = -11,  //传入配置表为空
    XP_MQTT_ERR_CONFIG_HAS_NULL             = -12,  //传入配置表存在空项
    XP_MQTT_ERR_CONFIG_CELLBACK_HAS_NULL    = -13,  //传入配置表中的回掉函数存在空项
    XP_MQTT_ERR_CONFIG_CREATE_TASK_FAIL     = -14,  //创建xp_mqtt任务失败

//xpmqtt主题增加或删除错误
    XP_MQTT_ERR_TOPICTAB_FULL               = -30,  //主题表已满

//xpmqtt任务错误
    XP_MQTT_ERR_TASK_PROC_ERR               = -50,  //状态机出错
    XP_MQTT_ERR_TASK_SUBTABLE_ERR           = -51,  //订阅表执行出错           

//xpmqtt数据发布错误
    XP_MQTT_ERR_SEND_DISCONNECT             = -70,  //未连接mqtt
    XP_MQTT_ERR_SEND_FULL                   = -71,  //发送队列已满
    XP_MQTT_ERR_SEND_FAIL                   = -72,  //发送失败
}XP_MQTT_ERR_CODE;


#endif

