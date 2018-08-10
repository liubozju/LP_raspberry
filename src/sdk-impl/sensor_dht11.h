#ifndef __DHT11_H__
#define __DHT11_H__

#include <wiringPi.h>
typedef enum
{
	DHTLIB_ERROR_TIMEOUT = 0,
	DHTLIB_ERROR_CHECKSUM,
	DHTLIB_OK
}dht11_status;

dht11_status dht11_read(int pin,float * humidity,float * temperature);

#endif
