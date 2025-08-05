#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include "Application.h"
#include <I2SMEMSSampler.h>
#include <ADCSampler.h>
#include "transports/WebSocketTransport.h"
#include "transports/TCPSocketTransport.h"
#include "config.h"

void Application::begin()
{
#ifdef USE_I2S_MIC_INPUT
    this->input = new I2SMEMSSampler(I2S_NUM_0, i2s_mic_pins, i2s_mic_Config);
#else
    this->input = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, i2s_adc_config);
#endif
    this->transport1 = new WebSocketTransport();
    this->transport2 = new TCPSocketTransport();
    this->input->start();
    this->transport1->begin();
    this->transport2->begin();
    
    this->taskRunning = true;
    xTaskCreate(Application::streamer_task, "streamer_task", 8192, this, 1, &this->taskHandle);
}

Application::~Application()
{
    Serial.println("[Application] Destructor called - cleaning up resources");
    
    // Signal the task to stop
    this->taskRunning = false;
    
    // Wait for the task to finish (with timeout)
    if (this->taskHandle != nullptr) {
        // Give the task time to exit gracefully
        for (int i = 0; i < 100 && eTaskGetState(this->taskHandle) != eDeleted; i++) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        // Force delete if still running
        if (eTaskGetState(this->taskHandle) != eDeleted) {
            Serial.println("[Application] Force deleting streaming task");
            vTaskDelete(this->taskHandle);
        }
        this->taskHandle = nullptr;
    }
    
    // Stop and cleanup input
    if (this->input) {
        this->input->stop();
        delete this->input;
        this->input = nullptr;
    }
    
    // Cleanup transports
    if (this->transport1) {
        delete this->transport1;
        this->transport1 = nullptr;
    }
    
    if (this->transport2) {
        delete this->transport2;
        this->transport2 = nullptr;
    }
    
    Serial.println("[Application] Cleanup complete");
}

void Application::streamer_task(void *param)
{
    Application *app = (Application *)param;
    Serial.println("[Application] Streaming task started");
    
    // Allocate samples buffer
    int16_t *samples = (int16_t *)malloc(sizeof(int16_t) * 1024);
    if (!samples) {
        Serial.println("[Application] Failed to allocate samples buffer");
        vTaskDelete(nullptr);
        return;
    }
    
    while (app->taskRunning)
    {
        // Read from the microphone
        int samples_read = app->input->read(samples, 1024);
        
        // Send to the two transports
        if (samples_read > 0) {
            app->transport1->send(samples, samples_read * sizeof(int16_t));
            app->transport2->send(samples, samples_read * sizeof(int16_t));
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    // Cleanup and exit
    free(samples);
    Serial.println("[Application] Streaming task exiting");
    vTaskDelete(nullptr);
}
