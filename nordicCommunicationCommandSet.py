#!/usr/bin/env python3


class NordicCommandSet:

    def __init__(self, command_set_version):
        if command_set_version == 1:
            self.load_command_set_v1()

    def load_command_set_v1(self):
        self.BBP_VERSION = 0x02
        self.BBP_VERSION_SHORT = 0x03
        self.BBP_W_DEBUG_REG = 0x04
        self.BBP_R_DEBUG_REG = 0x05

        self.BBP_LED_ENABLE_WHITE = 0xa0
        self.BBP_LED_ENABLE_IR = 0xa1
        self.BBP_LED_ENABLE_UV = 0xa2
        self.BBP_LED_DISABLE = 0xa3
        self.BBP_LED_SET_CURRENT = 0xa4
        self.BBP_STATUS_LED_TEST = 0xac
        self.BBP_STATUS_LED_ENABLE = 0xad
        self.BBP_STATUS_LED_DISABLE = 0xae
        self.BBP_LED_SET_STATUS_LED = 0xaf

        self.BBP_BAT_GET_VOLTAGE = 0xb0

        self.BBP_PIM_REPORT_ALIVE = 0xc0
        self.BBP_PIM_SET_WD_TIMEOUT = 0xc1
        self.BBP_PIM_WAKE_UP_IN_SEC = 0xc2
        self.BBP_PIM_WAIT_FOR_SHUTDOWN = 0xc3
        self.BBP_PIM_HARD_RESET = 0xc4
        self.BBP_PIM_WAKEUP_BY_VOLTAGE = 0xc5
        self.BBP_PIM_SET_DEFAULT_OFF_TIME = 0xc6
        self.BBP_PIM_SET_BAT_VOLTAGES = 0xc7
        self.BBP_PIM_GET_BAT_VOLTAGES = 0xc8

        self.BBP_LED_SYNC_ENABLE = 0xe0
        self.BBP_LED_SYNC_DISABLE = 0xe1
        self.BBP_LED_SYNC_SET_WIDTH = 0xe2
        self.BBP_LED_SYNC_OFFSET = 0xe3

        self.BBP_TOF_LIGHT_LEVEL = 0xd0

        self.BBP_DEBUG_SYSTIME = 0xf0
        self.BBP_DEBUG_WATCHDOG = 0xf1
