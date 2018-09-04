#include <stdint.h>
#include "sensor_dht11.h"
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>

int alarm_status = 0;
// int alarm_status;
pthread_t led_thread;
static int led_fre = 2;
static void * ledcontrol(void * pin)
{
    struct timeval temp;
    int * p = (int*)pin;
    if(led_fre <= 0){
        digitalWrite(*p,LOW);
        return ((int *)0);
    }
 
    while(1){
        if(alarm_status != 1) continue;
        digitalWrite(*p,LOW);
        temp.tv_sec = 0;
        temp.tv_usec = 1000000/led_fre;
        select(0,NULL,NULL,NULL,&temp);

        digitalWrite(*p, HIGH);
        temp.tv_sec = 0;
        temp.tv_usec = 1000000/led_fre;
        select(0,NULL,NULL,NULL,&temp);
    }
        digitalWrite(*p,LOW);
    return ((int *)0);
}

static dht11_status dht11_read_data(int pin,float * humidity,float * temperature)
{
        uint8_t readbuf[5] = {0};
        uint8_t cnt = 7;
        uint8_t idx = 0;
        uint32_t loopcnt = 10000;

        wiringPiSetup();

        pinMode(pin,OUTPUT);
        digitalWrite(pin, LOW);
        delay(20);
        digitalWrite(pin , HIGH);
        pinMode(pin , INPUT);
        delayMicroseconds(40);

        while(digitalRead(pin) == LOW)
                if (loopcnt-- == 0) return DHTLIB_ERROR_TIMEOUT;

        loopcnt = 10000;

        while(digitalRead(pin) == HIGH)
                if (loopcnt-- == 0) return DHTLIB_ERROR_TIMEOUT;
	
	for(int i=0; i<40; i++)
	{
		loopcnt = 10000;
		while(digitalRead(pin) == LOW)
                	if (loopcnt-- == 0) return DHTLIB_ERROR_TIMEOUT;
		
		uint32_t t = micros();

		loopcnt = 10000;
      	        while(digitalRead(pin) == HIGH)
              		if (loopcnt-- == 0) return DHTLIB_ERROR_TIMEOUT;		
		if((micros() - t) > 40) readbuf[idx] |= (1 << cnt);

		if(cnt == 0)
		{
			cnt = 7;
			idx++;
		}
		else cnt--;
	}

	*humidity = readbuf[0]*10 + readbuf[1];

	if(readbuf[3]&0x80)
	{
		*temperature = 0-(readbuf[2]*10 + ((readbuf[3]&0x7F)));
	}
	else
	{
		*temperature = readbuf[2]*10 + readbuf[3];
	}
	
	//temperature range: -20°C~60°C,humidity range:5%RH~95%RH
	
	if(*humidity>950)
	{
		*humidity = 950;
	}
	if(*humidity < 50)
	{
		*humidity = 50;
	}
	if(*temperature > 600)
	{
		*temperature = 600;
	}
	if(*temperature < -200)
	{
		*temperature = -200;
	}

	*temperature = *temperature/10;
	*humidity = *humidity/10;

	if(readbuf[4] != readbuf[0]+readbuf[1]+readbuf[2]+readbuf[3] ) 
		return DHTLIB_ERROR_CHECKSUM;
	return DHTLIB_OK;
}

dht11_status dht11_read(int pin,float * humidity,float * temperature)
{
	int i = 10;
	float temp_humidity=0,temp_temperature=0;
	for(i=0;i<10;i++)
	{
		dht11_read_data(pin,humidity,temperature);
		temp_humidity += *humidity;
		temp_temperature += *temperature;
	}

	*humidity = temp_humidity/10;
	*temperature = temp_temperature/10;
	return 0;
}

int led_glimmer(int *pin)
{
    int err = 0;
    wiringPiSetup();

    pinMode(*pin,OUTPUT);

    err =  pthread_create(&led_thread,NULL,ledcontrol,pin);
    if(err != 0) {
 //       printf("led thread create fail :%s\n",strerror(err));
        return 1;
    }
    return 0;
}
/*static int led_glimmer_stop()
{
   // pthread_create(&led_thread);
    return 0;
}*/
int set_led_fre(int fre)
{
    if((fre>=0)&&(fre<25)) {
        led_fre = fre;
    }
    return 0;
}
int get_led_fre()
{
    return led_fre;
}
