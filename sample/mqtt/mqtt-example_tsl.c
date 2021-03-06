/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "sensor_dht11.h"
#define DATA_MSG_FORMAT		 "{\"IndoorTemperature\":%.1f, \"RelativeHumidity\":%.1f}"

#define ALINK_BODY_FORMAT        	 "{\"id\":\"%d\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST    	 "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define ALINK_TOPIC_PROP_POSTRSP 	 "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post_reply"
#define ALINK_TOPIC_PROP_SET     	 "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define ALINK_METHOD_PROP_POST   	 "thing.event.property.post"
#define ALINK_METHOD_EVENT_POST   	 "thing.event.Alarm.post"

#define ALINK_TOPIC_SER_SUB     	 "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/ClearAlarm"
#define ALINK_TOPIC_EVENT_PUB     	 "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/Alarm/post"

#define PRODUCT_KEY             "a1x6pMlAkXS"
#define DEVICE_SECRET           "jDwCAb4j0ZBGpi6sExp58EqqiAjgBJWR"
#define DEVICE_NAME             "test"

char __product_key[PRODUCT_KEY_LEN + 1];
char __device_name[DEVICE_NAME_LEN + 1];
char __device_secret[DEVICE_SECRET_LEN + 1];
#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

extern int alarm_status;
static int      user_argc;
static char   **user_argv;
static int temp_val = 32;
static int led_fre_upload_status = 1;
static int alarm_clear = 0;
void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void mqtt_sub_callback(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("packetId: %d", ptopic_info->packet_id);
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");

	if(NULL != strstr(ptopic_info->payload,"thing.service.ClearAlarm"))	
	{
		alarm_clear = 1;
	}
	if(NULL != strstr(ptopic_info->payload,"\"Frequency\":"))
	{
		int fre=0;
		char * result = strstr(ptopic_info->payload,"\"Frequency\":");
		result += strlen("\"Frequency\":");
		if(*(result+1)=='}')
		{
			fre = *result-'0';
		}
		 else if(*(result+2)=='}')
		{
			fre = (*result-'0')*10+(*(result+1)-'0');
		}

    	printf("Frequency %d\n",fre);

        set_led_fre(fre);
	    led_fre_upload_status=1;

	}

				   

}

#ifndef MQTT_ID2_AUTH
int mqtt_client(void)
{
    int pin = 4;//LED Pin
    int rc = 0, msg_len = 0, cnt = 0,t_count=0;
    float humidity = 0,temperature = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[150];
    char param[100];
    char *msg_buf = NULL, *msg_readbuf = NULL;

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    HAL_GetProductKey(__product_key);
    HAL_GetDeviceName(__device_name);
    HAL_GetDeviceSecret(__device_secret);

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(__product_key, __device_name, __device_secret, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    led_glimmer(&pin);
    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    
    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_PROP_SET, IOTX_MQTT_QOS1, mqtt_sub_callback, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

    rc = IOT_MQTT_Subscribe(pclient, ALINK_TOPIC_SER_SUB, IOTX_MQTT_QOS1, mqtt_sub_callback, NULL);

    IOT_MQTT_Yield(pclient, 200);
    /* Initialize topic information */
        memset(msg_pub, 0x0, sizeof(msg_pub));
	memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
	topic_msg.qos = IOTX_MQTT_QOS1;
	topic_msg.retain = 0;
	topic_msg.dup = 0;
	topic_msg.payload_len = msg_len;
    do {
        /* Generate topic message */
        cnt++;
	    dht11_read(1,&humidity,&temperature);	
	    
        if(temperature>temp_val)
	    {		
		    alarm_status=1;				    
		    if(alarm_clear==0)							
		    {									     											    
		    	sprintf(param, "{\"Temperature\":%f}",temperature);
			    int msg_len = snprintf(msg_pub,sizeof(msg_pub), ALINK_BODY_FORMAT, cnt, ALINK_METHOD_EVENT_POST, param);	
			    if (msg_len < 0) EXAMPLE_TRACE("Error occur! Exit program");
        	    	topic_msg.payload = (void *)msg_pub;
       			    topic_msg.payload_len = msg_len;	
			    rc = IOT_MQTT_Publish(pclient,ALINK_TOPIC_EVENT_PUB,&topic_msg);			
			    if (rc < 0) EXAMPLE_TRACE("Error occur when publish");														
		        EXAMPLE_TRACE("Alink:\n%s\n",msg_pub);
		    }						       
        }
        else		
    	{
	    	alarm_status=0;
        }
	
    	if(alarm_clear==1)
    	{
    		if(temperature<temp_val)
    		{
    			alarm_clear = 0;
                alarm_status = 0;
	    	}
    	}
        if(led_fre_upload_status==1)
	    {
		    led_fre_upload_status = 0;
		    int fre = get_led_fre();
	       	printf("get_led_fre :%d\n",fre);
		    sprintf(param, "{\"Frequency\":%d}",fre);
		    int msg_len = snprintf(msg_pub,sizeof(msg_pub), ALINK_BODY_FORMAT, cnt, ALINK_METHOD_PROP_POST, param);
		    if (msg_len < 0) EXAMPLE_TRACE("Error occur! Exit program");
																                
        	topic_msg.payload = (void *)msg_pub;
       		topic_msg.payload_len = msg_len;	
		    rc = IOT_MQTT_Publish(pclient,ALINK_TOPIC_EVENT_PUB,&topic_msg);			
	    	if (rc < 0) EXAMPLE_TRACE("error occur when publish");
																		
		    EXAMPLE_TRACE("Alink:\n%s\n",msg_pub);
    	}   
	
	           
        if(t_count>=20)
        {
	        t_count=0;
	  	    float temp = (float)temperature;
		    sprintf(param, "{\"Temperature\":%f}",temp);
            int msg_len = snprintf(msg_pub,sizeof(msg_pub),ALINK_BODY_FORMAT, cnt, ALINK_METHOD_PROP_POST, param);
	        if (msg_len < 0) EXAMPLE_TRACE("Error occur! Exit program");
        	    topic_msg.payload = (void *)msg_pub;
       		    topic_msg.payload_len = msg_len;	
		    rc = IOT_MQTT_Publish(pclient,ALINK_TOPIC_EVENT_PUB,&topic_msg);			
	        if (rc < 0) EXAMPLE_TRACE("error occur when publish");
		    EXAMPLE_TRACE("Alink:\n%s\n",msg_pub);
																						    
       }    
	    t_count++;

#ifdef MQTT_ID2_CRYPTO
        EXAMPLE_TRACE("packet-id=%u, publish topic msg='0x%02x%02x%02x%02x'...",
                      (uint32_t)rc,
                      msg_pub[0], msg_pub[1], msg_pub[2], msg_pub[3]
                     );
#endif

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            HAL_SleepMs(2000);
            cnt = 0;
        }

    } while (1);
        
    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}
#endif /* MQTT_ID2_AUTH */

#ifdef MQTT_ID2_AUTH
#include "tfs.h"
char __device_id2[TFS_ID2_LEN + 1];
int mqtt_client_secure()
{
    int rc = 0, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;
    char  topic_update[IOTX_URI_MAX_LEN] = {0};
    char  topic_error[IOTX_URI_MAX_LEN] = {0};
    char  topic_get[IOTX_URI_MAX_LEN] = {0};
    char  topic_data[IOTX_URI_MAX_LEN] = {0};

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    HAL_GetProductKey(__product_key);
    HAL_GetID2(__device_id2);

    /* Device AUTH */
    rc = IOT_SetupConnInfoSecure(__product_key, __device_id2, __device_id2, (void **)&pconn_info);
    if (rc != 0) {
        EXAMPLE_TRACE("AUTH request failed!");
        goto do_exit;
    }

    HAL_Snprintf(topic_update,IOTX_URI_MAX_LEN,TOPIC_UPDATE_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_error,IOTX_URI_MAX_LEN,TOPIC_ERROR_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_get,IOTX_URI_MAX_LEN,TOPIC_GET_FMT,__product_key,__device_id2);
    HAL_Snprintf(topic_data,IOTX_URI_MAX_LEN,TOPIC_DATA_FMT,__product_key,__device_id2);

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_ConstructSecure(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, topic_data, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

    HAL_SleepMs(1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = IOT_MQTT_Publish(pclient, topic_data, &topic_msg);
    EXAMPLE_TRACE("rc = IOT_MQTT_Publish() = %d", rc);

    do {
        /* Generate topic message */
        cnt++;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);
        if (msg_len < 0) {
            EXAMPLE_TRACE("Error occur! Exit program");
            rc = -1;
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        rc = IOT_MQTT_Publish(pclient, topic_data, &topic_msg);
        if (rc < 0) {
            EXAMPLE_TRACE("error occur when publish");
            rc = -1;
            break;
        }
        EXAMPLE_TRACE("packet-id=%u, publish topic msg='0x%02x%02x%02x%02x'...",
                      (uint32_t)rc,
                      msg_pub[0], msg_pub[1], msg_pub[2], msg_pub[3]
                     );

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            HAL_SleepMs(2000);
            cnt = 0;
        }

    } while (cnt < 1);

    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);

    HAL_SleepMs(200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;

}
#endif /* MQTT_ID2_AUTH*/

int main(int argc, char **argv)
{
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = argc;
    user_argv = argv;
    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);

#ifndef MQTT_ID2_AUTH
    mqtt_client();
#else
    mqtt_client_secure();
#endif
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");

    return 0;
}

