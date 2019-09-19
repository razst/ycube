
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_avg = 0;		// y[i-1]
float alpha = 0;			//<! smoothing constant
int error;
unsigned char i2c_address = EPS_I2C_ADDR;


voltage_t eps_threshold_voltages[NUMBER_OF_THRESHOLD_VOLTAGES];	// saves the current EPS logic threshold voltages

int GetBatteryVoltage(voltage_t *vbatt)
{
	ieps_enghk_data_cdb_t hk_tlm;
	ieps_statcmd_t cmd;
	ieps_board_t board = ieps_board_cdb1;
	error = IsisEPS_getEngHKDataCDB(EPS_I2C_BUS_INDEX, board, &hk_tlm, &cmd);
	*vbatt = hk_tlm.fields.bat_voltage;
	return error;
}

int EPS_Init()
{

	error = IsisEPS_initialize(&i2c_address , 1);
	logError(error);

	error = IsisSolarPanelv2_initialize(slave0_spi);
	logError(error);
//	IsisSolarPanelv2_sleep(); cheek

	error = GetThresholdVoltages(&eps_threshold_voltages);
	logError(error);

	error = GetAlpha(&alpha);
	if(error != E_NO_SS_ERR){
		alpha = DEFAULT_ALPHA_VALUE;
	}

	prev_avg = 0;
	error = GetBatteryVoltage(&prev_avg);
	logError(error);

	EPS_Conditioning();

	return 0;
}

int EPS_Conditioning()
{
	return 0;
}

int UpdateAlpha(float new_alpha)
{
	return 0;
}

int UpdateThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	return 0;
}

int GetThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	return 0;
}

int GetAlpha(float *alpha)
{
	return 0;
}

int RestoreDefaultAlpha()
{
	return 0;
}

int RestoreDefaultThresholdVoltages()
{
	return 0;
}

