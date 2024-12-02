/*
 * @file	EPS.h
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */
#ifndef EPS_H_
#define EPS_H_

#include "GlobalStandards.h"
#include "EPSOperationModes.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include <stdint.h>

/*
 	 	 	 	  ______
			  ___|		|___
 	 	 	 |				|
 	 	 	 |	FULL MODE	|
 	 	 	 |- - - - - - -	|	-> FULL UP = 7400
 	 	 	 |- - - - - - - |	-> FULL DOWN = 7300
 	 	 	 |				|
 	 	 	 |	CRUISE MODE	|
 	 	 	 |- - - - - - -	|	-> CRUISE UP = 7200
 	 	 	 |- - - - - - - |	-> CRUISE DOWN = 7100
 	 	 	 |				|
 	 	 	 |	SAFE MODE	|
 	 	 	 |- - - - - - -	| 	-> SAFE UP = 6600
 	 	 	 |- - - - - - - |	-> SAFE DOWN = 6500
 	 	 	 |				|
 	 	 	 |	CRITICAL	|
 	 	 	 |______________|
 */
#define DEFAULT_ALPHA_VALUE 0.3

#define NUMBER_OF_SOLAR_PANELS			6
#define NUMBER_OF_THRESHOLD_VOLTAGES 	6 		///< first 3 are discharging voltages, last 3 are charging  voltages
#define DEFAULT_EPS_THRESHOLD_VOLTAGES 	{(voltage_t)6500, (voltage_t)7100, (voltage_t)7300,	 \
										  (voltage_t)6600, (voltage_t)7200, (voltage_t)7400}


#define _SOLAR_PIN_RESET PIN_GPIO08
#define _SOLAR_PIN_INT   PIN_GPIO00


typedef enum __attribute__ ((__packed__)){
	INDEX_DOWN_SAFE,
	INDEX_DOWN_CRUISE,
	INDEX_DOWN_FULL,
	INDEX_UP_SAFE,
	INDEX_UP_CRUISE,
	INDEX_UP_FULL
}EpsThresholdsIndex;

typedef union __attribute__ ((__packed__)){
	voltage_t raw[NUMBER_OF_THRESHOLD_VOLTAGES];
	struct {
		voltage_t Vdown_safe;
		voltage_t Vdown_cruise;
		voltage_t Vdown_full;
		voltage_t Vup_safe;
		voltage_t Vup_cruise;
		voltage_t Vup_full;
	}fields;
}EpsThreshVolt_t;
typedef union __attribute__ ((__packed__)){
struct {
	int16_t H1_MIN;
	int16_t H1_MAX;
	int16_t H2_MIN;
	int16_t H2_MAX;
	int16_t H3_MIN;
	int16_t H3_MAX;
}value;
}HeaterValues;
/*!
 * @brief initializes the EPS subsystem.
 * @return	0 on success
 * 			-1 on failure of init
 */
int EPS_Init();

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @return	0 on success
 * 			-1 on failure setting state of channels
 */
int EPS_Conditioning();

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat);

/*!
 * @brief setting the new EPS logic threshold voltages on the FRAM.
 * @param[in] thresh_volts an array holding the new threshold values
 * @return	0 on success
 * 			-1 on failure setting new threshold voltages
 * 			-2 on invalid thresholds
 * 			ERR according to <hal/errors.h>
 */
int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts);

/*!
 * @brief getting the EPS logic threshold  voltages on the FRAM.
 * @param[out] thresh_volts a buffer to hold the threshold values
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES]);

/*!
 * @brief getting the smoothing factor (alpha) from the FRAM.
 * @param[out] alpha a buffer to hold the smoothing factor
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetAlpha(float *alpha);

/*!
 * @brief setting the new voltage smoothing factor (alpha) on the FRAM.
 * @param[in] new_alpha new value for the smoothing factor alpha
 * @note new_alpha is a value in the range - (0,1)
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * 			-2 on invalid alpha
 * @see LPF- Low Pass Filter at wikipedia: https://en.wikipedia.org/wiki/Low-pass_filter#Discrete-time_realization
 */
int UpdateAlpha(sat_packet_t *cmd);

/*!
 * @brief setting the new voltage smoothing factor (alpha) to be the default value.
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * @see DEFAULT_ALPHA_VALUE
 */
int RestoreDefaultAlpha();

/*!
 * @brief	setting the new EPS logic threshold voltages on the FRAM to the default.
 * @return	0 on success
 * 			-1 on failure setting smoothing factor
  * @see EPS_DEFAULT_THRESHOLD_VOLTAGES
 */
int RestoreDefaultThresholdVoltages();

int CMDGetHeaterValues(sat_packet_t *cmd);

int CMDSetHeaterValues(sat_packet_t *cmd);


#endif
