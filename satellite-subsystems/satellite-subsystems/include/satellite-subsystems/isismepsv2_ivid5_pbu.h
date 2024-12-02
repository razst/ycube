/*
 * isismepsv2_ivid5_pbu.h
 *
 * AUTOGENERATED CODE
 * Please do not perform manual edits
 * Generated using autogen v1.0.3
 *
 * Generated from:
 *  - imepsv2_structs.yaml
 *  - imepsv2_pbu.yaml
 */

#ifndef ISISMEPSV2_IVID5_PBU_H_
#define ISISMEPSV2_IVID5_PBU_H_

#include "isismepsv2_ivid5_pbu_types.h"
#include <satellite-subsystems/common_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 *	Initialize driver for a number of ISISMEPSV2_IVID5_PBU instances. The first instance can then be referenced by using index 0, the second by using index 1, etc.
 *
 *	@param[in] isismepsv2_ivid5_pbu Pointer to array of ISISMEPSV2_IVID5_PBU instances
 *	@param[in] isismepsv2_ivid5_pbuCount Number of ISISMEPSV2_IVID5_PBU instances pointed to by isismepsv2_ivid5_pbu input parameter
 * 	@return Error code as specified in common_types.h
 */
driver_error_t ISISMEPSV2_IVID5_PBU_Init(const ISISMEPSV2_IVID5_PBU_t* isismepsv2_ivid5_pbu, uint8_t isismepsv2_ivid5_pbuCount);

/*!
 * Switches off any command enable output bus channels that have been switched on after the system powered up up. Only output bus channels that can be commanded off are affected. All force enable channels will remain enabled.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__cancel(uint8_t index, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Get the value of a configuration parameter.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] par_id_in parameter id of the parameter to get
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__getconfigurationparameter(uint8_t index, uint16_t par_id_in, isismepsv2_ivid5_pbu__getconfigurationparameter__from_t *response);

/*!
 * Prepare the response buffer with EPS housekeeping data retrieved from a battery module. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the EPS main controller.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__gethousekeepingdataeng(uint8_t index, isismepsv2_ivid5_pbu__gethousekeepingdataeng__from_t *response);

/*!
 * Prepare the response buffer with housekeeping data. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the main controller.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__gethousekeepingdataraw(uint8_t index, isismepsv2_ivid5_pbu__gethousekeepingdataraw__from_t *response);

/*!
 * Prepare the response buffer with running average housekeeping data of the system. The housekeeping data is returned in engineering values.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__gethousekeepingdatarunningavg(uint8_t index, isismepsv2_ivid5_pbu__gethousekeepingdatarunningavg__from_t *response);

/*!
 * Return system status information
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__getsystemstatus(uint8_t index, isismepsv2_ivid5_pbu__getsystemstatus__from_t *response);

/*!
 * Load all configuration parameters from non-volatile memory, discarding any changes made in volatile memory. This is performed automatically at system startup if a valid load configuration is encountered in non-volatile memory. If no (valid) configuration is found, the system will initialize using hard coded defaults.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] conf_key_in Configuration key: 0xA8. Any other value causes the reset command to be rejected with a parameter error. The reset will not be performed in that case.
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__loadconfiguration(uint8_t index, uint8_t conf_key_in, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Performs a no-operation. This is useful to check the availability of the system, without changing anything about the current configuration or operation.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__nop(uint8_t index, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Perform a software induced reset of the MCU.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__reset(uint8_t index, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Reset all configuration parameters to hard-coded defaults, discarding any changes made, in volatile memory (only!). This is performed automatically at system startup before an attempt to load a configuration is performed. If no (valid) configuration is found that can be loaded, the system will use hard coded defaults.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] conf_key_in Configuration key: 0xA8. Any other value causes the reset command to be rejected with a parameter error. The reset will not be performed in that case.
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__resetconfiguration(uint8_t index, uint8_t conf_key_in, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Reset a parameter to its default hard-coded value. All parameters have this value at system power-up or after the software reset command.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] par_id_in parameter id of the parameter to get
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__resetconfigurationparameter(uint8_t index, uint16_t par_id_in, isismepsv2_ivid5_pbu__resetconfigurationparameter__from_t *response);

/*!
 * Resets the watchdog timer keeping the system from performing a reset.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__resetwatchdog(uint8_t index, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Commit all read/write configuration parameters kept in volatile memory to non-volatile memory.
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] checksum_in To force save this value can be set to 0. The save will then proceed without performing CRC verification.
 * @param[out] reply_header_out 
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__saveconfiguration(uint8_t index, uint16_t checksum_in, isismepsv2_ivid5_pbu__replyheader_t *reply_header_out);

/*!
 * Change a configuration parameter. The change will take effect immediately and any function using the parameter will use the new value
 *
 * @param[in] index Index of ISISMEPSV2_IVID5_PBU in list provided during driver initialization
 * @param[in] params Struct with parameters for subsystem
 * @param[out] response Struct with response from subsystem
 * @return Error code as specified in common_types.h
 */
driver_error_t isismepsv2_ivid5_pbu__setconfigurationparameter(uint8_t index, const isismepsv2_ivid5_pbu__setconfigurationparameter__to_t *params, isismepsv2_ivid5_pbu__setconfigurationparameter__from_t *response);

#ifdef __cplusplus
}
#endif

#endif /* ISISMEPSV2_IVID5_PBU_H_ */

