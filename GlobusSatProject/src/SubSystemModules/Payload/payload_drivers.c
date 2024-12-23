#include "payload_drivers.h"
#include "hal/Drivers/I2C.h"
#include <string.h>
#include <hal/Timing/Time.h>
#include "utils.h"

#define EPS_INDEX 0
#define PAYLOAD_BUS_CHANNEL 4

// Macro to convert endianess
#define CHANGE_ENDIAN(x) ((x) = ((x) >> 24 & 0xff) | ((x) << 8 & 0xff0000) | ((x) >> 8 & 0xff00) | ((x) << 24 & 0xff000000))


SoreqResult payloadInit() {
    return payloadTurnOn();
}

SoreqResult payloadRead(int size, unsigned char *buffer) {
    unsigned char wtd_and_read[] = {CLEAR_WDT};
    int i;
    for (i = 0; i < TIMEOUT / READ_DELAY; ++i) {
        if (I2C_write(PAYLOAD_I2C_ADDRESS, wtd_and_read, 1) != 0) return PAYLOAD_I2C_WRITE_ERROR;
        vTaskDelay(READ_DELAY);
        if (I2C_read(PAYLOAD_I2C_ADDRESS, buffer, size) == 0 && buffer[3] == 0) return PAYLOAD_SUCCESS;
    }
    return PAYLOAD_TIMEOUT;
}

SoreqResult payloadSendCommand(char opcode, int size, unsigned char *buffer, int delay) {
    if (I2C_write(PAYLOAD_I2C_ADDRESS, (unsigned char *)&opcode, 1) != 0) return PAYLOAD_I2C_WRITE_ERROR;
    vTaskDelay(delay);
    return payloadRead(size, buffer);
}


#define ADC_TO_VOLTAGE(R) ((2 * 4.096 * (R)) / (2 << 23))
#define VOLTAGE_TO_TEMPERATURE(V) (100 * ((V) * (5 / 2.0) - 2.73))

SoreqResult payloadReadEnvironment(PayloadEnvironmentData *environment_data) {
    unsigned char buffer[12];
    SoreqResult res;

    // Read RADFET voltages
	Time_getUnixEpoch(&environment_data->radfet_time);
    res = payloadSendCommand(READ_RADFET_VOLTAGES, sizeof(buffer), buffer, 1250 / portTICK_RATE_MS);
    if (res != PAYLOAD_SUCCESS) {
        return res;
    }
    memcpy(&environment_data->adc_conversion_radfet1, buffer + 4, 4);
    memcpy(&environment_data->adc_conversion_radfet2, buffer + 8, 4);
    CHANGE_ENDIAN(environment_data->adc_conversion_radfet1);
    CHANGE_ENDIAN(environment_data->adc_conversion_radfet2);

    // Read temperature ADC value
    int raw_temperature_adc;
    Time_getUnixEpoch(&environment_data->temp_time);
    res = payloadSendCommand(MEASURE_TEMPERATURE, sizeof(buffer), buffer, 845 / portTICK_RATE_MS);
    if (res != PAYLOAD_SUCCESS) {
        return res;
    }
    memcpy(&raw_temperature_adc, buffer + 4, 4);
    CHANGE_ENDIAN(raw_temperature_adc);

    // Extract and process ADC value
    int remove_extra_bits = (raw_temperature_adc & (~(1 << 29))) >> 5; // Mask and shift to remove redundant bits
    double voltage = ADC_TO_VOLTAGE(remove_extra_bits); // Convert ADC value to voltage
    double temperature = VOLTAGE_TO_TEMPERATURE(voltage); // Convert voltage to temperature
    environment_data->temperature = temperature;

    return PAYLOAD_SUCCESS;
}


SoreqResult payloadReadEvents(PayloadEventData *event_data) {
    unsigned char buffer[12];
    SoreqResult res;

	Time_getUnixEpoch(&event_data->time);

    // Read SEL count
    if ((res = payloadSendCommand(READ_PIC32_SEL, sizeof(buffer), buffer, 10 / portTICK_RATE_MS)) != PAYLOAD_SUCCESS)
        return res;
    memcpy(&event_data->sel_count, buffer + 4, 4);
    if (event_data->sel_count == 0) memcpy(&event_data->sel_count, buffer + 8, 4);
    CHANGE_ENDIAN(event_data->sel_count);

    // Read SEU count
    if ((res = payloadSendCommand(READ_PIC32_SEU, sizeof(buffer), buffer, 100 / portTICK_RATE_MS)) != PAYLOAD_SUCCESS)
        return res;
    memcpy(&event_data->seu_count, buffer + 4, 4);
    CHANGE_ENDIAN(event_data->seu_count);

    FRAM_read((unsigned char*)&event_data->sat_reset_count,
    		NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

    FRAM_read((unsigned char*)&event_data->eps_reset_count,
    		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);

    return PAYLOAD_SUCCESS;
}

SoreqResult payloadSoftReset() {
    return payloadSendCommand(SOFT_RESET, 0, NULL, 0);
}

