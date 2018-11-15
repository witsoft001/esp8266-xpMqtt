#include "mqtt_user_tasks.h"
#include "xp_mqtt.h"
#include "driver.h"

/***********************************************************************************/
//                  宏
/***********************************************************************************/
//MQTT服务器设置
//MQTT ip地址或域名
#define MQTT_BROKER_HOST     "mqtt.heclouds.com"
//端口号
#define MQTT_BROKER_PORT     6002
//mqtt连接超时时间(单位:ms)
#define MQTT_COMMAND_TIMEOUT 5000

//上传间隔
#define UPLOAD_INTERVAL      5*60

//设备上报数据点,主题字符串
#define MQTT_DATA_PUBLISH           "$dp"

//onenect平台的系统指令订阅与回复
#define ONENET_CMD_TOPIC  "$creq/#"
#define ONENET_RPY_TOPIC  "$crsp/%s"

#define SUB_TOPIC_1 "topic_1"
#define SUB_TOPIC_2 "topic_2"
#define SUB_TOPIC_3 "topic_3"
#define SUB_TOPIC_4 "topic_4"
#define SUB_TOPIC_5 "topic_5"

uint8 uartIsSend = 0;
uint8 uartPkt[128] = { 0 };
uint16 uartPktLen = 0;

//onenet数据流
/*
 RSSI
 NetReconnect
 RunTime
 xpMqttVersion
 */

/***********************************************************************************/
//                  内部数据结构
/***********************************************************************************/
//用户事件
typedef enum {
	EVENT_MQTT_NULL = 0,        //空事件
	EVENT_MQTT_FISRT_RUN,       //第一次连接成功
	EVENT_MQTT_CONNECT_SUCC,    //mqtt连接成功
	EVENT_MQTT_RECONNECT,       //mqtt重连
	EVENT_MQTT_HAS_NEW_MSG,     //有新的mqtt数据
	EVENT_MQTT_UPLOAD_NETINFO,  //上传网络状态信息
} USER_EVENT;

/***********************************************************************************/
//                  内部全局变量
/***********************************************************************************/
static xQueueHandle userEventQueue = NULL; //用户事件队列
static uint8 receiveEventEn = 0;    //接收事件使能(若用户接收事件未被处理,则不发送新的接收事件)
static os_timer_t sysRunTimer;

static uint32 runTime = 0;
static uint32 runTImeS = 0;
static uint32 netRestart = 0;

/***********************************************************************************/
//                  内部宏
/***********************************************************************************/

//复位用户事件队列
#define RESET_USER_EVENT_QUEQUE()       xQueueReset( userEventQueue )
//发送用户事件
#define SEND_USER_EVENT(event)          xQueueSend( userEventQueue, event, 0 )
//等待用户事件
#define WAIT_USER_EVENT(event)          xQueueReceive( userEventQueue, event, 10 )

/***********************************************************************************/
//                  内部函数定义
/***********************************************************************************/
//上传版本信息
static void IFA upload_onenet_verData(void) {
	MQTT_MSG msg;
	uint8* tempS = &msg.payload[3];
	sprintf(msg.topic, MQTT_DATA_PUBLISH);
	sprintf(tempS, "{\"xpMqttVersion\":\"%s\"}", Get_XP_MQTT_Version());

	msg.payload[0] = 0x03;
	msg.payload[1] = 0x00;
	msg.payload[2] = (uint8) strlen(tempS);

	msg.payloadlen = msg.payload[2] + 3;
	msg.qos = QOS1; //这里最好是QOS1
	msg.retained = 0;
	msg.dup = 0;
	//无阻发送
	msg.sendMode = SEND_MODE_UNBLOCK;
	XP_MQTT_SendMsg(&msg);

}

//上传网络状态数据流
static void IFA upload_onenet_netData(void) {
	MQTT_MSG msg;
	uint8* tempS = &msg.payload[3];
	sprintf(msg.topic, MQTT_DATA_PUBLISH);
	sprintf(tempS,
			"{\"RSSI\":%d,\"NetReconnect\":%d,\"RunTime\":%d,\"freeHeap\":%d}",
			wifi_station_get_rssi(), netRestart, runTime,
			system_get_free_heap_size());

	msg.payload[0] = 0x03;
	msg.payload[1] = 0x00;
	msg.payload[2] = (uint8) strlen(tempS);

	msg.payloadlen = msg.payload[2] + 3;
	msg.qos = QOS1; //这里最好是QOS1
	msg.retained = 0;
	msg.dup = 0;
	//无阻发送
	msg.sendMode = SEND_MODE_UNBLOCK;
	XP_MQTT_SendMsg(&msg);
}

/*****************************************************************************
 * 函 数 名  : sysRunTimer_cb
 * 函数功能  :
 系统运行时间计时器回调函数
 定时时间(1s)
 * 输入参数  :
 arg : 定时器初始化时的回掉参数
 * 返 回 值  : 无
 * 备    注  : 无
 *****************************************************************************/
static void sysRunTimer_cb(void *arg) {
	runTime++;
	if ((runTime - runTImeS) > UPLOAD_INTERVAL) {
		uint8 userEvent;
		runTImeS = runTime;
		userEvent = EVENT_MQTT_UPLOAD_NETINFO;
		SEND_USER_EVENT(&userEvent);
	}
}

static void IFA smartconfig_done(sc_status status, void *pdata) {
	switch (status) {
	case SC_STATUS_LINK:
		d_printf("SC_STATUS_LINK");
		struct station_config staConf = *((struct station_config *) pdata);
		//开始wifi连接任务
		wifi_station_set_config(&staConf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		d_printf("SC_STATUS_LINK_OVER");
		smartconfig_stop();
		break;
	}
}

//错误信息回调函数
//注:不要在该回调函数做阻塞操作或执行长时间的代码
static void mqtt_error_cellback(XP_MQTT_ERR_CODE err) {
	d_printf("xp mqtt err(%d)!", err);
	switch (err) {
	//申请内存失败
	case XP_MQTT_ERR_MALLOC_FAIL:
		break;
		//状态机出错
	case XP_MQTT_ERR_TASK_PROC_ERR:
		break;
	default:
		break;
	}
}

//事件回调函数
//注:不要在该回调函数做阻塞操作或执行长时间的代码
//请用用户事件发布的形式进行
static void mqtt_event_cellback(XP_MQTT_EVENT event) {
	uint8 userEvent = EVENT_MQTT_NULL;
	if (event > XP_MQTT_EVENT_START__ && event < XP_MQTT_EVENT_END__) {
		d_printf("xp mqtt event(%02d)!", event);
	}
	switch (event) {
	//网络连接中
	case XP_MQTT_EVENT_NET_CONNECTING:
		break;
		//网络连接成功
	case XP_MQTT_EVENT_NET_CONN_SUCC:
		break;
		//网络连接失败
	case XP_MQTT_EVENT_NET_CONN_FAIL:
		break;
		//网络断开
	case XP_MQTT_EVENT_NET_DISCONN:
		break;
		//网络重连
	case XP_MQTT_EVENT_NET_RECONN: {
		//发送用户事件
		userEvent = EVENT_MQTT_RECONNECT;
		SEND_USER_EVENT(&userEvent);
		break;
	}
		//与mqtt服务器连接中
	case XP_MQTT_EVENT_MQTT_CONNECTING:
		break;
		//与mqtt服务器连接成功
	case XP_MQTT_EVENT_MQTT_CONN_SUCC: {
		receiveEventEn = 1;
		//发送用户事件
		userEvent = EVENT_MQTT_CONNECT_SUCC;
		SEND_USER_EVENT(&userEvent);
		break;
	}
		//与mqtt服务器连接失败
	case XP_MQTT_EVENT_MQTT_CONN_FAIL:
		break;
		//第一次连上mqtt服务器
	case XP_MQTT_EVENT_FIRST_CONN: {
		//发送用户事件
		userEvent = EVENT_MQTT_FISRT_RUN;
		SEND_USER_EVENT(&userEvent);
		break;
	}
		//订阅表更新中
	case XP_MQTT_EVENT_TOPICTABLE_UPDATING:
		break;
		//订阅表更新成功
	case XP_MQTT_EVENT_TOPICTABLE_UPDATA_SUCC: {
		//打印xp_mqtt_task剩余堆栈
		d_printf("xp_mqtt_task free heap: %d",
				uxTaskGetStackHighWaterMark(NULL));
		break;
	}
		//订阅表更新失败
	case XP_MQTT_EVENT_TOPICTABLE_UPDATA_FAIL:
		break;
		//数据接收成功
	case XP_MQTT_EVENT_RECEIVE_SUCC: {
		//打印xp_mqtt_task剩余堆栈
		d_printf("xp_mqtt_task free heap: %d",
				uxTaskGetStackHighWaterMark(NULL));
		if (receiveEventEn == 1) { //不让回调多次发送HAS_NEW_MSG事件
			receiveEventEn = 0;
			//发送用户事件
			userEvent = EVENT_MQTT_HAS_NEW_MSG;
			SEND_USER_EVENT(&userEvent);
		}
		break;
	}
		//数据接收失败
	case XP_MQTT_EVENT_RECEIVE_FAIL:
		break;
		//发布消息中
	case XP_MQTT_EVENT_PUBLISHING:
		break;
		//消息发布成功
	case XP_MQTT_EVENT_PUBLISH_SUCC: {
		//打印xp_mqtt_task剩余堆栈
		d_printf("xp_mqtt_task free heap: %d",
				uxTaskGetStackHighWaterMark(NULL));
		break;
	}
		//消息发布失败
	case XP_MQTT_EVENT_PUBLISH_FAIL:
		break;
		//删除xp_mqtt任务前的最后一条事件
	case XP_MQTT_EVENT_TASK_CLOSE:
		break;
		break;
	default:
		d_printf("unknow event(%d)", event);
		break;
	}
}

//用户mqtt业务逻辑任务
static void mqtt_user_task(void *pvParameters) {
	int rc = 0;
	uint8 userEvent;
	uint8* tempS;
	uint8* tempE;
	uint16 len;

	MQTT_MSG rMsg;
	MQTT_MSG wMsg;

	//创建用户事件队列
	userEventQueue = xQueueCreate(5, sizeof(uint8));

	//设置定时器
	//参数一：要设置的定时器；参数二：定时器到时间要执行的回调函数；参数三：回调函数的参数
	os_timer_setfn(&sysRunTimer, sysRunTimer_cb, NULL);

//使能（启动）定时器
//参数一：要使能的定时器；参数二：定时时间（单位：ms）；
//参数三：是否重复执行，即执行完1次就停止还是一直执行下去
	os_timer_arm(&sysRunTimer, 1000, 1); //1：重复  0：只一次

	d_printf("ver: %s", Get_XP_MQTT_Version());

//这里是xp_mqtt的初始化
	XP_MQTT_CONFIG xpMqttConfig = {
			//定义mqtt版本：3 = 3.1 ， 4 = 3.1.1
			.MQTTVersion = 4,
			.borkerHost = MQTT_BROKER_HOST,
			.borkerPort =MQTT_BROKER_PORT,
			.mqttCommandTimeout = MQTT_COMMAND_TIMEOUT,
			.usermame = "183306",
			.password = "esp8266device1",
			.clientID ="503068936",

			.keepAliveInterval = 60,
			.cleansession = false,
			.xp_mqtt_error_cellback = mqtt_error_cellback,
			.xp_mqtt_event_cellback = mqtt_event_cellback };

	rc = XP_MQTT_START(&xpMqttConfig);
	if (rc != XP_MQTT_SUCCESS) {
		d_printf("XP_MQTT_START errCode(%d)", rc);
		while (1)
			;
	}
	//块级作用域
	XP_MQTT_TOPIC_ITEM topics[5] = { { ITEM_HANDLE_ADD, QOS0, SUB_TOPIC_1 }, {
			ITEM_HANDLE_ADD, QOS1, SUB_TOPIC_2 }, { ITEM_HANDLE_ADD, QOS2,
	SUB_TOPIC_3 }, { ITEM_HANDLE_ADD, QOS0, SUB_TOPIC_4 }, {
			ITEM_HANDLE_ADD_UNSUB, QOS1, ONENET_CMD_TOPIC }, };
	//提交主题表操作
	//这里是xp_mqtt的主题表操作*******************
	rc = XP_MQTT_TopicTable_Submit(topics, 5);
	d_printf("XP_MQTT_TopicTable_Submit = %d", rc);

	while (1) {
		//等待用户事件
		if (WAIT_USER_EVENT(&userEvent)) {
			switch (userEvent) {
			case EVENT_MQTT_FISRT_RUN: {
				upload_onenet_verData();
				break;
			}
			case EVENT_MQTT_CONNECT_SUCC:
			case EVENT_MQTT_UPLOAD_NETINFO: {        //上传网络状态
				upload_onenet_netData();
				break;
			}
			case EVENT_MQTT_RECONNECT: {        //网络重连加加
				netRestart++;
				break;
			}
			case EVENT_MQTT_HAS_NEW_MSG: {        //有新的mqtt数据消息
												  //这里是接收**************************
				while (Get_XP_MQTT_ReceiveMsg(&rMsg, 10)) {
					//收到消息
					d_printf("topic:\"%s\"", rMsg.topic);
					d_printf("payload(%3d):\"%s\"", rMsg.payloadlen,
							rMsg.payload);
					if (strcmp(rMsg.payload, "info") == 0) {    //往topic_2发送设备信息
						sprintf(wMsg.topic, "topic_2");
						sprintf(wMsg.payload, "msg");
						wMsg.payloadlen = strlen(wMsg.payload);
						wMsg.qos = QOS1; //这里最好是QOS1
						wMsg.retained = 0;
						wMsg.dup = 0;
						//阻塞发送
						wMsg.sendMode = SEND_MODE_BLOCK;
						XP_MQTT_SendMsg(&wMsg);
					}
					if (strncmp(rMsg.topic, "$creq/", strlen("$creq/")) == 0) { //往onenet平台发送回复信息
						len = strlen("$creq/");
						tempS = &rMsg.topic[len];
						sprintf(wMsg.topic, ONENET_RPY_TOPIC, tempS);
						sprintf(wMsg.payload, "reply!");
						wMsg.payloadlen = strlen(wMsg.payload);
						wMsg.qos = QOS1; //这里最好是QOS1
						wMsg.retained = 0;
						wMsg.dup = 0;
						//阻塞发送
						wMsg.sendMode = SEND_MODE_BLOCK;
						XP_MQTT_SendMsg(&wMsg);
					}
				}
				//收完打开接收事件使能
				receiveEventEn = 1;
				break;
			}
			} //end switch( userEvent )
		} //end if( WAIT_USER_EVENT(&userEvent) )

		/**
		 *   这里是发送
		 *   串口发数据格式 XXXX,XXXXXXXX~ 例:topic_1,Hello world!~
		 *   串口数据结束符: '~'(记得发这个)
		 *   本人只做了串口数据的简单拼接,(没做保护)~囧~
		 *   串口乱发数据导致复位，本人概不负责~~(手动滑稽)
		 */
		if (uartIsSend) { //串口发送: 1~,2~,3~,4~....分别执行不同的订阅和去订阅操作
			if (uartPkt[0] == '1') { //去订阅(SUB_TOPIC_1,SUB_TOPIC_2)
				XP_MQTT_TOPIC_ITEM topics[2] = { { ITEM_HANDLE_DEL, QOS1,
				SUB_TOPIC_1 }, //删除
						{ ITEM_HANDLE_DEL, QOS2, SUB_TOPIC_2 }, //删除
						};
				//提交主题表操作
				rc = XP_MQTT_TopicTable_Submit(topics, 2);
				d_printf("XP_MQTT_TopicTable_Submit result = %d", rc);
			} else if (uartPkt[0] == '2') { //订阅(SUB_TOPIC_3,SUB_TOPIC_4,SUB_TOPIC_5)
				XP_MQTT_TOPIC_ITEM topics[3] = { { ITEM_HANDLE_ADD, QOS0,
				SUB_TOPIC_3 }, //订阅
						{ ITEM_HANDLE_ADD, QOS1, SUB_TOPIC_4 }, //订阅
						{ ITEM_HANDLE_ADD, QOS2, SUB_TOPIC_5 }, //订阅
						};
				//提交主题表操作
				rc = XP_MQTT_TopicTable_Submit(topics, 3);
				d_printf("XP_MQTT_TopicTable_Submit result = %d", rc);

			} else if (uartPkt[0] == '3') { //订阅和去订阅混合操作
				XP_MQTT_TOPIC_ITEM topics[5] = { { ITEM_HANDLE_DEL, QOS0,
				SUB_TOPIC_1 }, //删除
						{ ITEM_HANDLE_ADD, QOS1, SUB_TOPIC_2 }, //订阅
						{ ITEM_HANDLE_DEL, QOS2, SUB_TOPIC_3 }, //删除
						{ ITEM_HANDLE_ADD, QOS0, SUB_TOPIC_4 }, //订阅
						{ ITEM_HANDLE_DEL, QOS1, SUB_TOPIC_5 }, //删除
						};
				//提交主题表操作
				rc = XP_MQTT_TopicTable_Submit(topics, 5);
				d_printf("XP_MQTT_TopicTable_Submit result = %d", rc);
			} else if (uartPkt[0] == '4') { //全部删除
				rc = XP_MQTT_TopicTable_DeleteAll();
				d_printf("XP_MQTT_TopicTable_DeleteAll result = %d", rc);
			} else if (uartPkt[0] == '5') {
				XP_MQTT_TOPIC_ITEM topics[1] = { { ITEM_HANDLE_ADD_UNSUB, QOS0,
				ONENET_CMD_TOPIC }, //加入过滤器，不订阅
						};
				//提交主题表操作
				rc = XP_MQTT_TopicTable_Submit(topics, 1);
				d_printf("XP_MQTT_TopicTable_Submit result = %d", rc);
			} else if (uartPkt[0] == '6') {
				XP_MQTT_TOPIC_ITEM topics[1] = { { ITEM_HANDLE_DEL, QOS0,
				ONENET_CMD_TOPIC }, };
				//提交主题表操作
				rc = XP_MQTT_TopicTable_Submit(topics, 1);
				d_printf("XP_MQTT_TopicTable_Submit result = %d", rc);
			} else if (strcmp(uartPkt, "wifi") == 0) { //uart: wifi~
				smartconfig_start(smartconfig_done);
			} else
			//这里，按主题发布消息*************
			{ //uart mqtt数据发送格式,例: topic_1,Hllo world!~
				memset((uint8*) &wMsg, 0, sizeof(MQTT_MSG));
				//在这里我全都没做保护,使用请小心机器复位,~~~嘿嘿嘿
				tempS = uartPkt;
				tempE = strchr(tempS, ',');
				len = tempE - tempS;
				strncpy(wMsg.topic, tempS, len);
				wMsg.topic[len] = '\0';
				d_printf("uart topic: %s", wMsg.topic);

				tempS = tempE + 1;
				tempE = strchr(tempS, '\0');
				//这里开始组包
				strcpy(wMsg.payload, tempS);
				wMsg.payloadlen = tempE - tempS;
				wMsg.qos = QOS1; //这里最好是QOS1
				wMsg.retained = 0;
				wMsg.dup = 0;
				if (strcmp(wMsg.topic, "topic_1") == 0) { //topic_1为阻塞发送
					wMsg.sendMode = SEND_MODE_BLOCK;
					rc = XP_MQTT_SendMsg(&wMsg);
				} else { //无阻发送
					wMsg.sendMode = SEND_MODE_UNBLOCK;
					rc = XP_MQTT_SendMsg(&wMsg);
				}
				d_printf("XP_MQTT_SendMsg result = %d", rc);
			}

			//算了我还是做一下拼接处理吧,但这只是简单的处理
			uartPktLen = 0;
			uartIsSend = 0;
		}
	}
}

/***********************************************************************************/
//                  外部函数定义
/***********************************************************************************/
/*****************************************************************************
 * 函 数 名  : MQTT_USER_TAST_Init
 * 函数功能  : 初始化mqtt用户逻辑任务
 * 输入参数  :
 usStackDepth    : 分配堆栈大小
 uxPriority      : 优先级(越大优先级越高)
 * 返 回 值  : 无
 * 备    注  : 无
 *****************************************************************************/
xTaskHandle xHandlerMQTT = NULL;
void MQTT_USER_TAST_Init(uint16 usStackDepth, uint16 uxPriority) {

	if (xHandlerMQTT == NULL)
		xTaskCreate(mqtt_user_task, "mqtt_user", usStackDepth, NULL, uxPriority,
				&xHandlerMQTT);
}

