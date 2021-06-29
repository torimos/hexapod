#include "Platform.h"
#include "HexConfig.h"
#include "HexModel.h"
#include "IKSolver.h"
#include "Stopwatch.h"
#include "BLEInputDriver.h"
#include "SerialProtocol.h"
#include "ServoDriver.h"
#include "Controller.h"
#include <ble_dev.h>

ServoDriver sd(&STM32uart);
BLEInputDriver inputDrv;
SerialProtocol* debugSP = new SerialProtocol(&DEBUGuart);
Controller controller(&inputDrv, &sd, debugSP);

// todo:
// 1) calibration thrugh DEBUGuart
// 2) figure out why 10 packets when debug is on, and why only 4 in idle when off

void bleDataCallback(std::string source, uint8_t id, uint8_t* data, size_t length) {
  if (inputDrv.incomingDataProcessed)
	{
		if (id == 0x1 && sizeof(joy_data_t) == length)
		{
			memcpy(&inputDrv.incomingData, data, sizeof(joy_data_t));
			inputDrv.incomingDataProcessed = false;
		}
	}
}

void setup() {
  Platform_Init();
  StateLed.Flash(CRGB(4,0,0), 3, 250);
  inputDrv.Setup();
  sd.Init();
  sd.Reset();
  controller.Setup();
  delay(200);
  Log.println("Setup is done.");
  ble_begin(bleDataCallback);
  StateLed.Flash(CRGB(0,0,8), 2, 150);
}

void loop() {
  ble_run();
  controller.Loop();
}