/**
 *	@file	errors.h
 *	@author	JWAR, Innovative Solutions In Space B.V.<br/>
 *			Copyright 2014 Innovative Solution In Space B.V. All rights reserved.
 * 	@brief	Header file containing error messages and their values
 */
#ifndef ERRORS_H_
#define ERRORS_H_


#define E_NO_SS_ERR              0 ///< No error.
#define E_NOT_INITIALIZED       -18 ///< Attempt to use something that is not yet initialized.
#define E_IS_INITIALIZED        -19 ///< Attempt to initialize something that has already been initialized.
#define E_GET_SEMAPHORE_FAILED  -20 ///< Failed to obtain semaphore for communication.
#define E_INDEX_ERROR           -22 ///< Incorrect index specified.
#define E_BITRATE_INPUT_ERROR   -23 ///< Failed to set the bitrate of the TRXUV.
#define E_CWCHAR_INPUT_ERROR    -24 ///< Failed to set the cwcharrate of the TRXUV.
#define E_IDLE_STATE_ERROR      -25 ///< Failed to go into idle mode of the TRXUV.
#define E_OUTPUT_MODE_ERROR     -26 ///< Failed to go into outmode of the TRXUV.
#define E_TRXUV_COMPONENT_ERROR -27 ///< Failed to choose a device in the TRXUV.
#define E_WRONG_ADC_CHANNEL     -28 ///< Failed to choose a correct ADC channel TRXUV.
#define E_RESET_SYSTEM          -29 ///< Failed to reset both microcontrollers in the TRXUV.
#define E_MEM_ALLOC             -30 ///< Failed to allocate memory.
#define E_ATT_ERROR             -31 ///< Failed to set attenuation value in the TXS.
#define E_PARAM_OUTOFBOUNDS     -32 ///< A parameter is out of bounds.
#define E_TRXUV_FRAME_LENGTH    -33 ///< Input frame length was greater than the maximum frame length of TRXUV.
#define E_INPUT_POINTER_NULL    -34 ///< A pointer input to the function was NULL.
#define E_COMMAND_NACKED    	-35 ///< A command sent did not get acknowledged.
#define E_COMMS_ERROR           -36 ///< Communication with component failed.
#define E_COMPARISON_ERROR      -37 ///< Input data and output data were not equal.
#define E_TYPE_ERROR     		-38 ///< Unsupported type.

#endif /* ERRORS_H_ */
