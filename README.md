# radio_tool

TYT/BaoFeng/(Others?) Firmware tool

# Download

For windows you can download the build artifacts from [AppVeyor](https://ci.appveyor.com/project/v0l/radio-tool)

Linux/Mac should use the build instructions below

# Building
Dependencies Linux (Ubuntu/Debian)
:
```
sudo apt install libusb-1.0-0-dev cmake gcc
```

Dependencies Mac:
```
brew install libusb cmake
```

Build:
```
git clone https://github.com/v0l/radio_tool
cd radio_tool
mkdir build && cd build
cmake ..
make -j4
./radio_tool --help
```

# Usage
```
Usage:
  ./radio_tool [OPTION...]

 General options:
  -h, --help            Show this message
      --list            List devices
  -d, --device <index>  Device to use
  -i, --in <file>       Input file
  -o, --out <file>      Output file

 Programming options:
  -f, --flash    Flash firmware
  -p, --program  Upload codeplug

 Firmware options:
      --fw-info   Print info about a firmware file
      --wrap      Wrap a firmware bin
      --unwrap    Unwrap a fimrware file

 All radio options:
      --info                 Print some info about the radio
      --write-custom <data>  Send custom command to radio
      --get-status           Print the current DFU Status

 TYT Radio options:
      --get-time             Gets the radio time
      --set-time             Sets the radio time
      --dump-reg <register>  Dump a register from the radio
      --reboot               Reboot the radio
      --dump-bootloader      Dump bootloader (Mac only)

 Codeplug options:
      --codeplug-info  Print info about a codeplug file
```

## Flash firmware
```
./radio_tool -d 0 -f -i new_firmware.bin
```