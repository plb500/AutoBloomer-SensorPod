#include "stemma_soil_sensor.h"
#include "hardware/gpio.h"

#include <stdio.h>


const uint8_t SEESAW_STATUS_BASE            = 0x00;
const uint8_t SEESAW_STATUS_HW_ID           = 0x01;
const uint8_t SEESAW_STATUS_SWRST           = 0x7F;
const uint8_t SEESAW_HW_ID_CODE             = 0x55;
const uint8_t SEESAW_STATUS_VERSION         = 0x02;
const uint8_t SEESAW_TOUCH_BASE             = 0x0F;
const uint8_t SEESAW_TOUCH_CHANNEL_OFFSET   = 0x10;


bool soil_sensor_begin(StemmaSoilSensor *soilSensor) {
    uint8_t response = 0x33;

    // Scan bus for device at given address
    bool found = false;
    for (int retries = 0; retries < 10; retries++) {
        if (check_i2c_address(soilSensor->mInterface, soilSensor->mAddress)) {
            found = true;
            break;
        }
        sleep_ms(10);
    }

    if (!found) {
        return false;
    }

    // Reset device
    reset_soil_sensor(soilSensor);
    found = false;
    for (int retries = 0; retries < 10; retries++) {
        if (check_i2c_address(soilSensor->mInterface, soilSensor->mAddress)) {
            found = true;
            break;
        }
        sleep_ms(10);
    }

    if (!found) {
        return false;
    }

    // Get hardware ID from device
    found = false;
    for (int retries = 0; !found && retries < 10; retries++) {
        read_from_i2c_register(soilSensor->mInterface, soilSensor->mAddress, SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &response, 1, 4);

        if(response == SEESAW_HW_ID_CODE) {
            found = true;
            break;
        }
    }

    return found;
}

bool reset_soil_sensor(StemmaSoilSensor *soilSensor) {
    const uint8_t resetBuffer[] = {
        0xFF
    };

    return write_to_i2c_register(soilSensor->mInterface, soilSensor->mAddress, SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, resetBuffer, 1);
}

uint32_t get_soil_sensor_version(StemmaSoilSensor *soilSensor) {
    uint8_t buf[4];
    read_from_i2c_register(soilSensor->mInterface, soilSensor->mAddress, SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4, 100);

    return (((uint) buf[0] << 24) | 
            ((uint) buf[1] << 16) |
            ((uint) buf[2] << 8) |
            (uint) buf[3]);
}

uint16_t get_soil_sensor_capacitive_value(StemmaSoilSensor *soilSensor) {
    static const uint16_t READ_DELAY_MS = 5;
    static const uint16_t NUM_RETRIES = 3;
    static const int READING_BUFFER_SIZE = 2;
    
    uint8_t buf[READING_BUFFER_SIZE];
    uint16_t ret = STEMMA_SOIL_SENSOR_INVALID_READING;

    for(uint8_t retry = 0; retry < NUM_RETRIES; retry++) {
        if(read_from_i2c_register(
            soilSensor->mInterface, 
            soilSensor->mAddress, 
            SEESAW_TOUCH_BASE, 
            SEESAW_TOUCH_CHANNEL_OFFSET, 
            buf, 
            READING_BUFFER_SIZE, 
            READ_DELAY_MS)
        ) {
            ret = ((uint16_t) buf[0] << 8) | buf[1];
        }
    }
    return ret;
}
