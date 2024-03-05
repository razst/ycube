/*
 * imepsv2_piu.h
 *
 * AUTOGENERATED CODE
 * Please do not perform manual edits
 *
 * Generated from: imepsv2_piu.yaml
 */

#ifndef IMEPSV2_PIU_H_
#define IMEPSV2_PIU_H_

#include "imepsv2_piu_types.h"
#include <satellite-subsystems/common_types.h>

/*!
 *	Initialize IMEPSV2_PIU instances
 *
 *	@param[in] Pointer to array of IMEPSV2_PIU instances.
 *	@param[in] Count of IMEPSV2_PIU instances.
 * 	@return driver_error_t
 */
int IMEPSV2_PIU_Init(IMEPSV2_PIU_t* imepsv2_piu, uint8_t imepsv2_piuCount);

/*!
 * Switches off any command enable output bus channels that have been switched on after the system powered up up. Only output bus channels that can be commanded off are affected. All force enable channels will remain enabled.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__cancel(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Get the value of a configuration parameter.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__getconfigurationparameter(uint8_t index, uint16_t par_id_in, imepsv2_piu__getconfigurationparameter__from_t *response);

/*!
 * Prepare the response buffer with housekeeping data. The housekeeping data is returned in engineering values.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingeng(uint8_t index, imepsv2_piu__gethousekeepingeng__from_t *response);

/*!
 * Prepare the response buffer with housekeeping data. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the main controller.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingengincdb(uint8_t index, imepsv2_piu__gethousekeepingengincdb__from_t *response);

/*!
 * Prepare the response buffer with running average housekeeping data. The housekeeping data is returned in engineering values.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingengrunningavgincdb(uint8_t index, imepsv2_piu__gethousekeepingengrunningavgincdb__from_t *response);

/*!
 * Prepare the response buffer with housekeeping data. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the main controller.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingraw(uint8_t index, imepsv2_piu__gethousekeepingraw__from_t *response);

/*!
 * Prepare the response buffer with housekeeping data. The housekeeping data is returned in raw form, as received from the hardware, unaltered by the main controller.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingrawincdb(uint8_t index, imepsv2_piu__gethousekeepingrawincdb__from_t *response);

/*!
 * Prepare the response buffer with running average housekeeping data. The housekeeping data is returned in engineering values.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__gethousekeepingrunningavg(uint8_t index, imepsv2_piu__gethousekeepingrunningavg__from_t *response);

/*!
 * Prepare the response buffer with output bus over current events. Over current fault counters are incremented each time a bus is latched off due to an overcurrent event.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__getovercurrentfaultstate(uint8_t index, imepsv2_piu__getovercurrentfaultstate__from_t *response);

/*!
 * Return system status information
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__getsystemstatus(uint8_t index, imepsv2_piu__getsystemstatus__from_t *response);

/*!
 * Load all configuration parameters from non-volatile memory, discarding any changes made in volatile memory. This is performed automatically at system startup if a valid load configuration is encountered in non-volatile memory. If no (valid) configuration is found, the system will initialize using hard coded defaults.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__loadconfiguration(uint8_t index, uint8_t conf_key_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Performs a no-operation. This is useful to check the availability of the system, without changing anything about the current configuration or operation.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__nop(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Turn a single output bus channel off using the bus channel index. Index 0 represents channel 0 (OBC0), index 1 represents channel 1 (OBC1), etc.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__outputbuschanneloff(uint8_t index, imepsv2_piu__imeps_channel_t obc_idx_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Turn a single output bus channel on using the bus channel index. Index 0 represents channel 0 (OBC0), index 1 represents channel 1 (OBC1), etc.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__outputbuschannelon(uint8_t index, imepsv2_piu__imeps_channel_t obc_idx_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Turn-off output bus channels that are marked with a 1-bit, leave bus channels that are not marked unaltered. The least-significant bit corresponds to bus channel 0 (OBC0), the next bit corresponds to channel 1 (OBC1), etc.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__outputbusgroupoff(uint8_t index, uint16_t obc_bf_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Turn-on output bus channels that are marked with a 1-bit, leave bus channels that are not marked unaltered. The least-significant bit corresponds to bus channel 0 (OBC0), the next bit corresponds to channel 1 (OBC1), etc.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__outputbusgroupon(uint8_t index, uint16_t obc_bf_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Turn-on bus channels that are marked with a 1-bit, turn-off bus channels that are not marked (i.e. 0-bit). The least-significant bit corresponds to bus channel 0 (OBC00), the next bit corresponds to channel 1 (OBC01), etc.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__outputbusgroupstate(uint8_t index, uint16_t obc_bf_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Perform a software induced reset of the MCU.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__reset(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Reset all configuration parameters to hard-coded defaults, discarding any changes made, in volatile memory (only!). This is performed automatically at system startup before an attempt to load a configuration is performed. If no (valid) configuration is found that can be loaded, the system will use hard coded defaults.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__resetconfiguration(uint8_t index, uint8_t conf_key_in, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Reset a parameter to its default hard-coded value. All parameters have this value at system power-up or after the software reset command.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__resetconfigurationparameter(uint8_t index, uint16_t par_id_in, imepsv2_piu__resetconfigurationparameter__from_t *response);

/*!
 * Resets the watchdog timer keeping the system from performing a reset.
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__resetwatchdog(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Commit all read/write configuration parameters kept in volatile memory to non-volatile memory.
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__saveconfiguration(uint8_t index, imepsv2_piu__saveconfiguration__to_t *params, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * Change a configuration parameter. The change will take effect immediately and any function using the parameter will use the new value
 *
 * @param [in] Parameters sent to subsystem.
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__setconfigurationparameter(uint8_t index, imepsv2_piu__setconfigurationparameter__to_t *params, imepsv2_piu__setconfigurationparameter__from_t *response);

/*!
 * To be defined
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__switchtonominal(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

/*!
 * To be defined
 *
 * @param [out] Response received from subsystem.
 * @return driver_error_t
 */
int imepsv2_piu__switchtosafety(uint8_t index, imepsv2_piu__replyheader_t *reply_header_out);

#endif /* IMEPSV2_PIU_H_ */

