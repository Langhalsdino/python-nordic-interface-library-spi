/*
 * spi_commands.h
 *
 *  Created on: 27.06.2019
 *      Author: thartman
 */

#ifndef LIB_SPIS_SPI_COMMANDS_H_
#define LIB_SPIS_SPI_COMMANDS_H_

// @formatter:off
/*!					|	7		6		5		4		3		2		1		0
 *		STATUS_RET	|	CTS		RTR										BATL	ERROR
 */
// @formatter:on

//! @brief Indicates that the controller is ready to receive commands
#define BBP_CTS_FLAG		(1 << 7)
//! @brief Indicates that data is ready to read / command has been executed
#define BBP_RTR_FLAG		(1 << 6)
//! @brief Indicates that the battery is low
#define BBP_BAT_LOW_FLAG	(1 << 1)
//! @brief Indicates that a function failed to execute
#define BBP_ERROR_FLAG		(1 << 0)

/*!
 * Commands
 */
//! @brief NOP Command for status requesting
#define BBP_NOP				0x00
//! @brief Readout command
#define BBP_READOUT			0x01

#define BBP_VERSION			0x02

/* LED Control */
#define BBP_LED_ENABLE_WHITE		0xa0
#define BBP_LED_ENABLE_IR		0xa1
#define BBP_LED_ENABLE_UV		0xa2
#define BBP_LED_DISABLE			0xa3
#define BBP_LED_SET_CURRENT		0xa4

/* Battery control */
#define BBP_BAT_GET_VOLTAGE		0xb0

/* Pi Management */
#define BBP_PIM_REPORT_ALIVE		0xc0
#define BBP_PIM_SET_WD_TIMEOUT		0xc1
#define BBP_PIM_WAKE_UP_IN_SEC		0xc2
#define BBP_PIM_WAIT_FOR_SHUTDOWN		0xc3

/* ToF values */
#define BBP_TOF_LIGHT_LEVEL         0xd0

/* Debugging */
#define BBP_DEBUG_DELAY			0xf9
#define BBP_DEBUG_SYSTIME		0xf0

#endif /* LIB_SPIS_SPI_COMMANDS_H_ */
