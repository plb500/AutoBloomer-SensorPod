#include "stemma_soil_sensor.h"


const uint8_t SEESAW_STATUS_BASE            = 0x00;
const uint8_t SEESAW_STATUS_HW_ID           = 0x01;
const uint8_t SEESAW_STATUS_SWRST           = 0x7F;
const uint8_t SEESAW_HW_ID_CODE             = 0x55;
const uint8_t SEESAW_STATUS_VERSION         = 0x02;
const uint8_t SEESAW_TOUCH_BASE             = 0x0F;
const uint8_t SEESAW_TOUCH_CHANNEL_OFFSET   = 0x10;


StemmaSoilSensor::StemmaSoilSensor(I2CInterface& i2cInterface, uint8_t address) :
    mI2CInterface(i2cInterface),
    mAddress(address)
{}

I2CInterface::I2CResponse StemmaSoilSensor::init() {
    uint8_t response = 0x33;

    // Scan bus for device at given address
    bool found = false;
    for (int retries = 0; retries < 10; retries++) {
        if(mI2CInterface.checkI2CAddress(mAddress) == I2CInterface::I2CResponse::I2C_RESPONSE_OK) {
            found = true;
            break;
        }
        sleep_ms(2);
    }

    if (!found) {
        return I2CInterface::I2CResponse::I2C_RESPONSE_DEVICE_NOT_FOUND;
    }

    // Reset device
    found = false;
    for (int retries = 0; retries < 10; retries++) {
        if(mI2CInterface.checkI2CAddress(mAddress) == I2CInterface::I2CResponse::I2C_RESPONSE_OK) {
            found = true;
            break;
        }
        sleep_ms(2);
    }

    if (!found) {
        return I2CInterface::I2CResponse::I2C_RESPONSE_DEVICE_NOT_FOUND;
    } else {
        reset();
    }

    sleep_ms(2);

    // Get hardware ID from device
    found = false;
    for (int retries = 0; !found && retries < 10; retries++) {
        mI2CInterface.readFromI2CRegister(mAddress,  SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID, &response, 1, 4);

        if(response == SEESAW_HW_ID_CODE) {
            found = true;
            break;
        }
    }

    return I2CInterface::I2CResponse::I2C_RESPONSE_OK;
}

I2CInterface::I2CResponse StemmaSoilSensor::reset() {
    const uint8_t resetBuffer[] = {
        0xFF
    };

    return mI2CInterface.writeToI2CRegister(mAddress, SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, resetBuffer, 1);
}

uint32_t StemmaSoilSensor::getVersion() {
    uint8_t buf[4];

    if(mI2CInterface.readFromI2CRegister(
        mAddress, 
        SEESAW_STATUS_BASE,
        SEESAW_STATUS_VERSION, 
        buf,
        4,
        100
    ) != I2CInterface::I2CResponse::I2C_RESPONSE_OK) {
        return STEMMA_SOIL_SENSOR_INVALID_READING;
    }

    return (((uint) buf[0] << 24) | 
            ((uint) buf[1] << 16) |
            ((uint) buf[2] << 8) |
            (uint) buf[3]);
}

uint16_t StemmaSoilSensor::getCapacitiveValue() {
    static const uint16_t READ_DELAY_MS = 5;
    static const uint16_t NUM_RETRIES = 3;
    static const int READING_BUFFER_SIZE = 2;
    
    uint8_t buf[READING_BUFFER_SIZE];
    uint16_t ret = STEMMA_SOIL_SENSOR_INVALID_READING;

    for(uint8_t retry = 0; retry < NUM_RETRIES; retry++) {
        if(mI2CInterface.readFromI2CRegister(
            mAddress,
            SEESAW_TOUCH_BASE, 
            SEESAW_TOUCH_CHANNEL_OFFSET, 
            buf, 
            READING_BUFFER_SIZE, 
            READ_DELAY_MS
        ) == I2CInterface::I2CResponse::I2C_RESPONSE_OK) {
            ret = ((uint16_t) buf[0] << 8) | buf[1];
        }
    }
    return ret;

}
