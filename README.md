# USB2MDIO

This is a simple USB serial to MDIO controller.

# Building the Firmware
Based on ChibiOS, it requires the toolchain `arm-none-eabi-gcc`.

```bash
make
```

Loading the firmware can be done via `dfu-util` utility, leveraging the built-in USB bootloader.
It requires pushing the BOOT0 button on the board, then resetting the MCU. Once this is done, the dfu loader can be called
```bash
make flash-dfu
```

Now the MCU reboots and enmerates on the USB, ready to be used.

# Basic Software usage
The UsbMdio adapter handles serial communication and provides a simple read/write reg API:
```python
from usb2mdio.adapter import UsbMdio

adapter = UsbMdio("COM5")
adapter.write_reg(0x0F, 1<<14)  # Software Reset
print(f"ID= {adapter.read_reg(2)}")
```

# Datasheet parsing and html visualisation
Some PHY can have hundereds of registers and multiple hundreds bitfields, making it cumbersome to peek/poke at them one by one.
A small utility parses HTML formatted documentation (such as the ones from ti.com) to extract register definition, reads all register values, compares them to the reset values then display every register, their bitfields and modified values in an HTML file, making it quick to have a global picture of the PHY's state.

```bash
python dp83tc813_dump.py
```
