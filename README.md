# python-nordic-interface-library-spi 

This project contains a lib to control the nordic co processor.

## Install

```bash
pip3 install .

# make sure SPI is enabled
sudo raspi-config
```

## Usage

```python
import nordicUtils
import nordicCommunicationCommandSet

# Initialize command set V1
nordic = nordicCommunicationCommandSet.NordicCommandSet(1)

# Read the battery voltage / input voltage
v_in = nordicUtils.read_bytes(nordic.BBP_BAT_GET_VOLTAGE, 2)
print(f"Input voltage: {v_in} mV")

# Disable LEDs
nordicUtils.exec_command(nordic.BBP_LED_DISABLE)

# Turn on IR LEDs at 1400 mA
nordicUtils.write_bytes(nordic.BBP_LED_SET_CURRENT, 1400, 2)
nordicUtils.exec_command(nordic.BBP_LED_ENABLE_IR)

# Please make sure only one LED type is turned on.
# The library and firmware does not ensure this!
# Multiple LEDs can be turned on, but this might overload the PSU :see-no-evil:

# Send report alive to nordic watchdog 
from time import sleep
while True:
    nordicUtils.exec_command(nordic.BBP_PIM_REPORT_ALIVE)
    # abitary interval. <1min is recommended.
    sleep(1)
```