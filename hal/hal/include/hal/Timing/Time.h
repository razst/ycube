/*!
 * @file	Time.h
 * @brief	Implements a redundant timing mechanism. The external RTC is used as
 * the reference clock. If the RTC fails, the time is maintained using just the RTT.
 * Synchronises the on-chip RTT and the DS3234 RTC and provides a unified time-keeping interface.
 *
 * @section DESCRIPTION
 * 1. Initialises all timing related peripherals
 * 2. Synchronises time across peripherals.
 * 3. Exposes combined timing control :
 * 3.1 Any set will directly reflect across RTT(resetting it) and RTC(storing time) + stores the local time
 * 3.2 Any get will retrieve time based on last retrieved RTC time(during sync or set) + offset from RTT
 */

#ifndef TIME_H_
#define TIME_H_

#include "hal/boolean.h"

#define UNIX_TIME_AT_Y2K	946684800 ///< Unix timestamp at 01-01-2000 00:00:00 UTC
#define TIMEDIFF_INVALID	0xFFFFFFFF

// Don't change the order of elements here
/*!
 * Structure containing time values in both human readable as well as machine form.
 * The fields seconds, minutes, hours, day, date, month and year express time in human
 * readable form.
 *
 * An additional field of secondsOfYear is provided. This is the number of seconds since
 * start of the year.
 * The fields (year, secondsOfYear) can be used to express time in a compact but complete
 * form.
 */
typedef struct __attribute__ ((__packed__)) _Time {
	unsigned char seconds; ///< Seconds of the minute.
	unsigned char minutes; ///< Minutes in the hour.
	unsigned char hours; ///< Hour of the day.
	unsigned char day;  ///< Day of the week (Sunday=1, Monday=2, ..., Saturday=7)
	unsigned char date; ///< Date of the month.
	unsigned char month; ///< Month of the year (January = 1).
	unsigned char year; ///< Year since 2000 (2000 = 0).
	unsigned int  secondsOfYear; ///< Number of seconds since the start of year.
} Time;

/*!
 * @brief Initialises the RTC and RTT and sets the given time at the RTC.
 * Also starts a task that automatically performs time-synchronisation.
 *
 * @param[in] time Pointer to a LocalTime structure with all members provided
 * except secondsOfYear. If this pointer is null, time is not set, just synchronised.
 * @param[in] syncInterval Time interval between synchronising RTC and RTT in seconds.
 * 0 means don't sync.
 * @return A combined status of RTC and initialisation.
 * 0 on success; 2 if setTime fails (input time was out-of-range);
 * 3 if FreeRTOS scheduler is not running (precedes all functionality so time won't be set etc.);
 * 1 if the RTT is not ticking (severe error, leads to no timing functionality).
 * If the RTC is not functioning, 0xFF00 is added to the above values.
 * However this doesn't necessarily mean that the RTC is dead. For example,
 * in case of error 2, its just not running because the input time was wrong.
 *
 * @see Time_setSyncInterval
 */
int Time_start(const Time *time, const unsigned int syncInterval);

/*!
 * @brief Sets the given time at the RTC and resets the RTT.
 * @param[in] time Pointer to a LocalTime structure with all members provided
 * except secondsOfYear.
 * @return 1 if setting time at the RTC fails (invalid input time),
 * 0 on success.
 */
int Time_set(const Time *time);

/*!
 * @brief Same as Time_set but takes the time input in seconds since 00:00:00 on Thursday, 1 January 1970.
 * However, the oldest time that can be set is 00:00:00 on Saturday, 1 January 2000.
 * This is a limitation of the RTC.
 * @param epochTime time input value.
 * @return 1 if setting time at the RTC fails (invalid input time),
 * 0 on success.
 */
int Time_setUnixEpoch(const unsigned int epochTime);

/*!
 * @brief Provides the current time by adding the value at RTT as an offset
 * over the last sync/set value at the RTC. This is done to avoid frequent
 * communication with the RTC which is slow.
 * @param[in,out] time Pointer to a LocalTime structure to be filled with time values.
 * @return 1 if Time has not been initialised yet, 2 if pointer is invalid, 0 otherwise.
 */
int Time_get(Time *time);

/*!
 * @brief Convert a Unix epoch to a Time struct
 * @param[in] epoch Epoch to be converted, this has to be >= 2000-01-01 00:00:00
 * @param[out] time A time struct corresponding to the input Epoch
 * @note This function does not modify any of the system timers
 * @return 0 if conversion went ok, 1 if epoch could not be converted
 */
int Time_convertEpochToTime(unsigned int epoch, Time* time);

/*!
 * @brief Convert a Time struct to Unix epoch
 * @param[in] time Pointer to a Time structure filled with time values, which has to be >= 2000-01-01 00:00:00
 * @return Unix epoch corresponding to the input time structure, if the returned
 * epoch is 0 then the input time was invalid
 * @note This function does not modify any of the system timers
 */
unsigned int Time_convertTimeToEpoch(const Time* time);

/*!
 * @brief Converts Unix epoch into RTT time
 * @param[in] epoch Epoch to be converted, this has to be > larger than last reset of RTT
 * @note This function is mostly intended to convert Unix epoch into RTT time to be used as an alarm. One shortcut that's been taken is assuming that RTT time == 0 is invalid
 * @return 0 if epoch is too old or unable to convert, > 0 if successfully converted
 */
unsigned int Time_convertEpochToRTT(unsigned int epoch);

/*!
 * @brief Converts RTT time into Unix epoch
 * @return 0 if RTT is invalid or unable to convert, > 0 if successfully converted
 */
unsigned int Time_convertRTTToEpoch(unsigned int rtt);

/*!
 * Returns the uptime of this program in seconds retrieved using FreeRTOS libraries.
 * This is a simple helper function that simply returns (xTaskGetTickCount() / configTICK_RATE_HZ).
 * @note Due to clock drifting, the uptime is not accurate to the second. A baseline time and Time_get(UnixEpoch) should be used instead if accurate uptime is required
 */
unsigned int Time_getUptimeSeconds(void);

/*!
 * Returns the uptime of this program in seconds retrieved using FreeRTOS libraries, but safe to use in interrupt context.
 * This is a simple helper function that simply returns (xTaskGetTickCountFromISR() / configTICK_RATE_HZ).
 * @note Due to clock drifting, the uptime is not accurate to the second.
 */
unsigned int Time_getUptimeSecondsFromISR(void);

/*!
 * Provides the current time in seconds since 00:00:00 on Thursday, 1 January 1970.
 * This is done by converting the time retrieved by Time_get.
 * @param epochTime Pointer to the location where the time should be stored.
 * @return 1 if Time has not been initialised yet, 0 otherwise.
 */
int Time_getUnixEpoch(unsigned int *epochTime);

/*!
 * @brief Retrieves the time at the RTC and updates the RTT offset. The time
 * retrieved from the RTC is stored locally and used by getTime as a baseline.
 * The RTT offset is then applied to provide the current time.
 * @return 1 if retrieving time from the RTC fails, 0 on success.
 */
int Time_sync(void);

/*!
 * Synchronises the time between the RTC and RTT if SyncInterval seconds have passed
 * since last time a sync was made.
 * @return 1 if retrieving time from the RTC fails, 0 on success.
 */
int Time_syncIfNeeded(void);

/*!
 * @brief Sets the time to pass between automatic synchronisation of time.
 * @param[in] seconds Time in seconds between automatic time synchronisation. This needs to be >= 10
 * @note The recommended sync interval is 60 seconds
 */
void Time_setSyncInterval(const unsigned int seconds);

/*!
 * @brief Takes year value and returns TRUE if the given year is a leap year,
 * FALSE otherwise. You may use an actual year value or an appropriately offset
 * one (divisible by 400). For example, using 2000 as year is OK.
 * @param[in] year to be checked.
 * @return 1 if it is a leap year, 0 otherwise.
 * @note input is an unsigned char so maximum value is 255.
 */
Boolean Time_isLeapYear(const unsigned int year);

/*!
 * @brief Returns the difference between the two input time values in seconds.
 * @note newTime must be newer than oldTime.
 * @param[in] newTime Location of a LocalTime structure containing the new time.
 * @param[in] oldTime Location of a LocalTime structure containing the old time.
 * @return TIMEDIFF_INVALID (0xFFFFFFFF) is returned when:
 * a) If the year values of the input times differ by more than 1, b) newTime is
 * older than oldTime.
 * Otherwise, the difference in the time values is returned in unit of seconds.
 * @note The function only uses the year and secondsOfYear fields of the Time
 * inputs.
 */
unsigned int Time_diff(const Time *newTime, const Time *oldTime);

/*!
 * @brief Returns an important system health flag indicating if the RTC
 * is working or not. This check is for redundancy purposes only.
 *
 * If the RTC is not functioning correctly, only the internal RTT is used
 * to keep time.
 *
 * @return TRUE = everything OK. FALSE = RTC not working.
 */
Boolean Time_isRTCworking(void);

#endif /* TIME_H_ */
