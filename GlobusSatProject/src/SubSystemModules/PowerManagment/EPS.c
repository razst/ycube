
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>
#include <utils.h>
#include <freertos/task.h>
#include <string.h>
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"

#include "EPS.h"
#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
	#include <satellite-subsystems/isismepsv2_ivid7_piu_types.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_avg = 0;		// y[i-1]
float alpha = 0;			//<! smoothing constant


// holds all 6 default values for eps_threshold
EpsThreshVolt_t eps_threshold_voltages = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};	// saves the current EPS logic threshold voltages

int GetBatteryVoltage(voltage_t *vbatt)
{
	isismepsv2_ivid7_piu__gethousekeepingengincdb__from_t hk_tlm;

	if(logError(isismepsv2_ivid7_piu__gethousekeepingengincdb(EPS_I2C_BUS_INDEX, &hk_tlm) ,"GetBatteryVoltage-isis_eps__gethousekeepingengincdb__tm"))return -1;

	*vbatt = hk_tlm.fields.batt_input.fields.volt;

	return 0;
}

int EPS_Init()
{


	// Init EPS
	ISISMEPSV2_IVID7_PIU_t subsystem[1]; // One instance to be initialised.
	subsystem[0].i2cAddr = EPS_I2C_ADDR; // I2C address defined to 0x20.

	if(logError(ISISMEPSV2_IVID7_PIU_Init( subsystem, 1),"EPS_Init-ISIS_EPS_Init")) return -1;


	// Init solar panels

	Pin solarpanelv2_pins[2] = {_SOLAR_PIN_RESET, _SOLAR_PIN_INT};

	if(logError(IsisSolarPanelv2_initialize(slave0_spi,&solarpanelv2_pins[0], &solarpanelv2_pins[1]) ,"EPS_Init-IsisSolarPanelv2_initialize")) return -1;
//	IsisSolarPanelv2_sleep(); cheek


	if(GetThresholdVoltages(&eps_threshold_voltages)) return -1;


	if(GetAlpha(&alpha)){
		alpha = DEFAULT_ALPHA_VALUE;
	}


	prev_avg = 0;
	GetBatteryVoltage(&prev_avg);

	logError(Payload_Safety(),"Payload_Safety");

	EPS_Conditioning();

	return 0;
}

#define GetFilterdVoltage(curr_voltage) (voltage_t) (alpha * curr_voltage + (1 - alpha) * prev_avg)

int EPS_Conditioning()
{

	voltage_t curr_voltage = 0;

	GetBatteryVoltage(&curr_voltage);

	voltage_t filtered_voltage = 0;					// the currently filtered voltage; y[i]
	filtered_voltage = GetFilterdVoltage(curr_voltage);
	printf("filtered_voltage =%d  curr_voltage=%d \n\r",filtered_voltage,curr_voltage);
	if(filtered_voltage < prev_avg){
		if(filtered_voltage  <  eps_threshold_voltages.fields.Vdown_safe ){
			 EnterCriticalMode();
		 }else if(filtered_voltage < eps_threshold_voltages.fields.Vdown_cruise){
			 EnterSafeMode();
		 }else if(filtered_voltage < eps_threshold_voltages.fields.Vdown_full){
			 EnterCruiseMode();
		 }else if(filtered_voltage > eps_threshold_voltages.fields.Vup_full){
			 EnterFullMode();
		 }

		}else {
			if(filtered_voltage > eps_threshold_voltages.fields.Vup_full){
				EnterFullMode();
			}else if(filtered_voltage > eps_threshold_voltages.fields.Vup_cruise){
				EnterCruiseMode();

			}else if(filtered_voltage > eps_threshold_voltages.fields.Vup_safe){
				EnterSafeMode();
			}
		prev_avg = filtered_voltage;
		}
	printf("state=%d\n\r",GetSystemState());
	return 0;
}

int UpdateAlpha(sat_packet_t *cmd)
{
	float new_alpha = *(float*)cmd->data;
	if(new_alpha < 0 || new_alpha > 1){
		return logError(-2 , "UpdateAlpha");
	}

	int err = logError(FRAM_write((unsigned char*) &new_alpha , EPS_ALPHA_FILTER_VALUE_ADDR , EPS_ALPHA_FILTER_VALUE_SIZE) ,"UpdateAlpha-FRAM_write");
	if (err == E_NO_SS_ERR){
		GetAlpha(&alpha);
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if(NULL == thresh_volts){
		return logError(E_INPUT_POINTER_NULL ,"UpdateThresholdVoltages");
	}

	Boolean valid_dependancies = (thresh_volts->fields.Vup_safe 	< thresh_volts->fields.Vup_cruise
		                           && thresh_volts->fields.Vup_cruise	< thresh_volts->fields.Vup_full);

		Boolean valid_regions = (thresh_volts->fields.Vdown_full 	< thresh_volts->fields.Vup_full)
							&&  (thresh_volts->fields.Vdown_cruise	< thresh_volts->fields.Vup_cruise)
							&&  (thresh_volts->fields.Vdown_safe	< thresh_volts->fields.Vup_safe);

		if (!(valid_dependancies && valid_regions)) {
			return logError(-2 ,"UpdateThresholdVoltages");
		}

	if(logError(FRAM_write((unsigned char*) thresh_volts , EPS_THRESH_VOLTAGES_ADDR , EPS_THRESH_VOLTAGES_SIZE) ,"UpdateThresholdVoltages-FRAM_write"))return -1;

	GetThresholdVoltages(&eps_threshold_voltages);


	return 0;
}

// check: what happens first time when there are no values in the FRAM
int GetThresholdVoltages(EpsThreshVolt_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	if(NULL == thresh_volts){
		return logError(E_INPUT_POINTER_NULL ,"GetThresholdVoltages");
	}

	if(logError(FRAM_read((unsigned char*) thresh_volts , EPS_THRESH_VOLTAGES_ADDR , EPS_THRESH_VOLTAGES_SIZE) ,"GetThresholdVoltages-FRAM_read"))return -1;
	return 0;
}

int GetAlpha(float *alpha)
{
	if(NULL == alpha){
		return logError(E_INPUT_POINTER_NULL ,"GetAlpha");
	}
	if(logError(FRAM_read((unsigned char*) (unsigned char*) alpha , EPS_ALPHA_FILTER_VALUE_ADDR , EPS_ALPHA_FILTER_VALUE_SIZE) ,"GetAlpha-FRAM_read"))return -1;

	return 0;

}

int RestoreDefaultAlpha()
{

	/*
	float def_alpha = DEFAULT_ALPHA_VALUE;
	if(logError(UpdateAlpha(def_alpha)))return -1;
*/
	return 0;
}

int RestoreDefaultThresholdVoltages()
{
	EpsThreshVolt_t def_thresh = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	if(logError(UpdateThresholdVoltages(&def_thresh) ,"RestoreDefaultThresholdVoltages-UpdateThresholdVoltages"))return -1;
	return 0;
}

// TODO check the code and output of this function!
int CMDGetHeaterValues(sat_packet_t *cmd){
	isismepsv2_ivid7_piu__getconfigurationparameter__from_t from;
	HeaterValues values;
	int err;
	// get current LOTHR_BAT_HEATER values

	err = isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3000, &from);
	memcpy(&values.value.H1_MIN,from.fields.par_val,sizeof(int16_t));
	vTaskDelay(4000);
	err = isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3001, &from);
	memcpy(&values.value.H2_MIN,from.fields.par_val,sizeof(int16_t));
	vTaskDelay(4000);
	err = isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3002, &from);
	memcpy(&values.value.H3_MIN,from.fields.par_val,sizeof(int16_t));

	// get current HITHR_BAT_HEATER value
	vTaskDelay(4000); //
	err += isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3003, &from);
	memcpy(&values.value.H1_MAX,from.fields.par_val,sizeof(int16_t));
	vTaskDelay(4000); //
	err += isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3004, &from);
	memcpy(&values.value.H2_MAX,from.fields.par_val,sizeof(int16_t));
	vTaskDelay(4000); //
	err += isismepsv2_ivid7_piu__getconfigurationparameter(EPS_I2C_BUS_INDEX, 0x3005, &from);
	memcpy(&values.value.H3_MAX,from.fields.par_val,sizeof(int16_t));

	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &values, sizeof(values));
	}

	return err;
}

// TODO check the code and output of this function!
int CMDSetHeaterValues(sat_packet_t *cmd){
	int err;
	isismepsv2_ivid7_piu__setconfigurationparameter__to_t setTo;
	isismepsv2_ivid7_piu__setconfigurationparameter__from_t setFrom;
	int16_t min;
	int16_t max;
	memcpy(&min,&cmd->data,sizeof(int16_t));
	memcpy(&max,&cmd->data[2],sizeof(int16_t));

	// set all min values
	memcpy(&setTo.fields.par_val[0],&min,sizeof(int16_t));
	setTo.fields.par_id = 0x3000;
	err = isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);
	vTaskDelay(4000); //
	setTo.fields.par_id = 0x3001;
	err += isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);
	vTaskDelay(4000); //
	setTo.fields.par_id = 0x3002;
	err += isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);

	// set all max values
	memcpy(&setTo.fields.par_val[0],&max,sizeof(int16_t));
	vTaskDelay(4000); //
	setTo.fields.par_id = 0x3003;
	err += isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);
	vTaskDelay(4000); //
	setTo.fields.par_id = 0x3004;
	err += isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);
	vTaskDelay(4000); //
	setTo.fields.par_id = 0x3005;
	err += isismepsv2_ivid7_piu__setconfigurationparameter(EPS_I2C_BUS_INDEX,&setTo, &setFrom);

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}

	return err;
}

