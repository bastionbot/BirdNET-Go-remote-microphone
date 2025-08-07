#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class Transport;
class I2SSampler;

class Application
{
private:
//    Transport *transport1 = NULL;
    Transport *transport2 = NULL;
    I2SSampler *input = NULL;
    TaskHandle_t taskHandle = NULL;
    volatile bool taskRunning = false;

public:
    void begin();
    ~Application();  // Destructor
    static void streamer_task(void *param);
};
