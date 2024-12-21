#ifndef PAYLOAD_DRIVERS_H_
#define PAYLOAD_DRIVERS_H_

#include <hal/boolean.h>
#include "GlobalStandards.h"

/**
 * @file payload_drivers.h
 * @brief Header file for payload driver functions and structures.
 *
 * This file contains function declarations and data structures used
 * to interface with the payload, including environment and event data
 * management, as well as control operations.
 */


#define PAYLOAD_I2C_ADDRESS 0x55
#define CLEAR_WDT 0x3F
#define SOFT_RESET 0xF8
#define READ_RADFET_VOLTAGES 0x33
#define MEASURE_TEMPERATURE 0x77
#define READ_PIC32_SEU 0x47
#define READ_PIC32_SEL 0x66
#define TIMEOUT 4000
#define READ_DELAY 200



/**
 * @enum SoreqResult
 * @brief Result codes for payload operations.
 */
typedef enum {
    EPS_ERROR,              /**< General error in the EPS system */
    PAYLOAD_SUCCESS,        /**< Operation completed successfully */
    PAYLOAD_I2C_WRITE_ERROR,/**< Error during I2C write operation */
    PAYLOAD_I2C_READ_ERROR, /**< Error during I2C read operation */
} SoreqResult;

/**
 * @struct PayloadEnvironmentData
 * @brief Structure to hold RADFET and temperature readings.
 */
typedef struct {
	time_unix radfet_time;
    int adc_conversion_radfet1; /**< ADC conversion result for RADFET 1 */
    int adc_conversion_radfet2; /**< ADC conversion result for RADFET 2 */
	time_unix temp_time;
    double temperature;            /**< Temperature measurement in degrees Celsius */
} PayloadEnvironmentData;

/**
 * @struct PayloadEventData
 * @brief Structure to hold SEU and SEL event readings.
 */
typedef struct {
    int seu_count; /**< Count of Single Event Upsets (SEUs) */
    int sel_count; /**< Count of Single Event Latchups (SELs) */
} PayloadEventData;

/**
 * @brief Initializes the payload system.
 *
 * This function performs all necessary setup for the payload, including
 * configuring interfaces and performing initial checks.
 *
 * @return SoreqResult indicating success or failure of the initialization.
 */
SoreqResult payloadInit();

/**
 * @brief Reads RADFET voltages and temperature measurements.
 *
 * This function retrieves RADFET ADC conversion results and the current
 * temperature measurement.
 *
 * @param[out] environment_data Pointer to a PayloadEnvironmentData structure
 *             where the readings will be stored.
 *
 * @return SoreqResult indicating success or failure of the read operation.
 */
SoreqResult payloadReadEnvironment(PayloadEnvironmentData *environment_data);

/**
 * @brief Reads SEU and SEL event counts.
 *
 * This function retrieves the count of Single Event Upsets (SEUs) and
 * Single Event Latchups (SELs) recorded by the payload.
 *
 * @param[out] event_data Pointer to a PayloadEventData structure where the
 *             event counts will be stored.
 *
 * @return SoreqResult indicating success or failure of the read operation.
 */
SoreqResult payloadReadEvents(PayloadEventData *event_data);

/**
 * @brief Performs a soft reset of the payload system.
 *
 * This function resets the payload without affecting its power state, allowing
 * recovery from transient errors or inconsistent states.
 *
 * @return SoreqResult indicating success or failure of the reset operation.
 */
SoreqResult payloadSoftReset();


#endif /* PAYLOAD_DRIVERS_H_ */
