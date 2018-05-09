#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include "Controller.h"

int main(int argc, char *argv[])
{ 
	if (wiringPiSetup() == -1)
	{
		printf("Unable to start wiringPi: %s\n", strerror(errno));
	}
	Controller* controller = new Controller("/dev/rfcomm0", "/dev/ttyS0");//"/dev/rfcomm1");
	controller->Setup();
	try
	{
		for(;;)
		{
			if (!controller->Loop()) break;
		}
	}
	catch (const char * err)
	{
		printf("Error: %s\n", err);
	}
	delete controller;
	return 0;
}