/*!
 * @file	FRAM.h
 * @brief	Driver for reading and writing data from/to the FRAM.
 * @note	Please don't call any of those functions from an interrupt context
 */

#ifndef FRAM_H_
#define FRAM_H_

#include <hal/errors.h>

/*!
 * Used by the FRAM_protectBlocks function to protect certain blocks of the FRAM from write operations.
 * blockProtect Values: 0 = None,
 * 						1 = upper 1/4 protected,
 * 						2 = upper 1/2 protected,
 * 						3 = Entire FRAM protected
 */
typedef union _FRAMblockProtect
{
	unsigned char rawValue;
	struct
	{
		unsigned char	reserved1 : 2,
						blockProtect: 2,
						reserved2 : 4;
	} fields;
} FRAMblockProtect;

/*!
 * Initializes the FRAM driver and the SPI driver if its not already initialized.
 * @return Error code as specified in errors.h
 */
int FRAM_start(void);

/*!
 * De-initializes the FRAM driver.
 */
void FRAM_stop(void);

/*!
 * Writes data to the FRAM.
 * @param data Address where data to be written is stored.
 * @param address Location in the FRAM where data should be written.
 * @param size Number of bytes to write.
 * @return Error code as specified in errors.h
 */
int FRAM_write(const unsigned char *data, unsigned int address, unsigned int size);

/*!
 * Reads data from the FRAM.
 * @param data Address where read data will be stored, this location must be able to accommodate size bytes.
 * @param address Location in the FRAM from which the data should be read.
 * @param size Number of bytes to read.
 * @return Error code as specified in errors.h
 */
int FRAM_read(unsigned char *data, unsigned int address, unsigned int size);

/*!
 * Writes data to the FRAM and reads it back to verify that it was written correctly.
 * @param data Address where data to be written is stored.
 * @param address Location in the FRAM where data should be written.
 * @param size Number of bytes to write.
 * @return Error code as specified in errors.h
 */
int FRAM_writeAndVerify(const unsigned char *data, unsigned int address, unsigned int size);

/*!
 * Write protects or un-protects blocks of the FRAM.
 * @param blocks FRAMblockProtect structure specifying the blocks to protect.
 * @return Error code as specified in errors.h
 */
int FRAM_protectBlocks(FRAMblockProtect blocks);

/*!
 * Reads the write protection status of the blocks of the FRAM.
 * @return blocks FRAMblockProtect structure specifying the blocks that are protected.
 * @return Error code as specified in errors.h
 */
int FRAM_getProtectedBlocks(FRAMblockProtect* blocks);

/*!
 * Retrieves the Device ID of the FRAM chip.
 * @param deviceID Pointer to where the retrieved device ID should be stored
 * @return Error code as specified in errors.h
 *
 * @note This function will copy 9 bytes to the memory location indicated by deviceID
 */
int FRAM_getDeviceID(unsigned char *deviceID);

/*!
 * Returns the highest address that is available in the FRAM memory. Please note that
 * this does not take protected areas into account.
 *
 * @return The highest available address in FRAM memory
 */
unsigned int FRAM_getMaxAddress(void);

#endif /* FRAM_H_ */
