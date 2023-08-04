# Handbrake Controller

This project is a handbrake simulator built using Arduino. It uses the HX711 amplifier for load cells and the Arduino Joystick library to simulate the action of a handbrake. 

The simulator reads settings from a serial port and uses these settings to adjust parameters such as the curve type, minimum and maximum raw handbrake, and curve factor. This information is then used to adjust the response of the handbrake.

GUI software for this project can be found [https://github.com/andersalavik/simrig-controller-gui](https://github.com/andersalavik/simrig-controller-gui)

## Requirements

- Arduino Board
- HX711 amplifier
- Load cell
- Arduino IDE

## Usage

Upload the Arduino sketch to your Arduino board using the Arduino IDE.

Once the sketch is uploaded and the Arduino is connected to your PC, you can send commands over the serial port to adjust the handbrake's settings. 

Commands include:

- `c`: Change curve type (send integer value between 0-2, where 0 = LINEAR, 1 = EXPONENTIAL, 2 = LOGARITHMIC)
- `m`: Change minimum raw handbrake value (send float value)
- `t`: Change maximum raw handbrake value (send float value)
- `f`: Change curve factor (send integer value, which is divided by 10 in the program)
- `s`: Save current settings to EEPROM
- `e`: Enable setup mode
- `w`: Disable setup mode
- `r`: Read and print current settings

When setup mode is enabled (`e` command), the raw and processed handbrake values are printed to the serial port.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.