#ifndef _BATTERY_VOLTAGE_SENSOR_H_
#define _BATTERY_VOLTAGE_SENSOR_H_

#include "sensor.h"

#include "pico/types.h"

class BatteryVoltageSensor : public Sensor {
    public:
        BatteryVoltageSensor(int enablePin, int measurePin, int adcInput);

        virtual void reset();
        virtual void shutdown();

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);
        virtual constexpr uint16_t getRawDataSize() const { return RAW_DATA_SIZE; }
        
        static const uint32_t RAW_DATA_SIZE = (sizeof(float));

    protected:
        virtual void doInitialization();
        virtual SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

    private:
        enum BatterySensorState {
            BATTERY_SENSOR_SLEEPING = 0,
            BATTERY_SENSOR_CHARGING = 1
        };

        const int mEnableSensePin;
        const int mBatteryMeasurePin;
        const int mADCInput;

        BatterySensorState mCurrentState;
        absolute_time_t mSensorTransitionTime;
};

#endif      // _BATTERY_VOLTAGE_SENSOR_H_
