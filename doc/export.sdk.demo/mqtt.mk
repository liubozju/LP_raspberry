all ::
	$(CC) $(CFLAGS) -o ext.mqtt mqtt-example.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o ext.mqtt mqtt-example_tsl.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o ext.mqtt mqtt-example_robot.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o ext.mqtt mqtt-example_sensor.c $(LDFLAGS)

clean ::
	rm -vf ext.mqtt

