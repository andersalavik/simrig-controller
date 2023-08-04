# Handbrake Controller

This project is a handbrake simulator built using Arduino and PlatformIO. It uses the HX711 amplifier for load cells and the Arduino Joystick library to simulate the action of a handbrake. 

The simulator reads settings from a serial port and uses these settings to adjust parameters such as the curve type, minimum and maximum raw handbrake, and curve factor. This information is then used to adjust the response of the handbrake.

GUI software for this project can be found [here](https://github.com/andersalavik/simrig-controller-gui)


## Credits

A lot of credits go to ChatGPT and CoPilot for helping me with this project.

## Requirements

- Arduino Board
- HX711 amplifier
- Load cell
- PlatformIO IDE

## Usage

1. Open the project in PlatformIO IDE.
2. Upload the Arduino sketch (located in `src/main.cpp`) to your Arduino board using PlatformIO.
3. Once the sketch is uploaded and the Arduino is connected to your PC, you can send commands over the serial port to adjust the handbrake's settings.

## Serial Commands

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

## Troubleshooting/FAQ

- **Serial communication is not working**: Make sure your Arduino is correctly connected and the correct COM port is selected in PlatformIO. Also ensure that the baud rate in your Arduino IDE matches the baud rate set in the code (9600).
  
- **The curve factor is not changing**: Remember to multiply your desired factor by 10 before sending, as the program divides the input by 10. For example, if you want to set the factor to 1.5, send 15.

- **Values aren't being saved after power off**: After adjusting the settings, remember to send the 's' command to save the current settings to the EEPROM.

- **The handbrake is not responsive or behaving unexpectedly**: Make sure the load cell and HX711 amplifier are correctly connected and working. Check your wiring and try calibrating the load cell.

## Contributing

Contributions are welcome and greatly appreciated! You can help improve the Handbrake Controller project in many ways. Here are some examples:

- Submit bugs and feature requests, and help us verify as they are checked in
- Review the source code changes
- Engage with other Handbrake Controller users and developers on GitHub
- Contribute bug fixes or new features

Before you contribute, please read through the contributing guide. Here is a quick guide on how to contribute:

1. Fork the repository (https://github.com/andersalavik/simrig-controller/fork)
2. Clone the project to your own machine (`git clone https://github.com/your-username/simrig-controller.git`)
3. Create a branch on your local machine to make your changes (`git checkout -b feature/fooBar`)
4. Commit your changes (`git commit -am 'Add some fooBar'`). Please follow good coding standards and styles.
5. Push your changes back to your fork (`git push origin feature/fooBar`)
6. Submit your changes as a Pull Request, ensuring that the PR description clearly describes your changes.

Please note that this project is released with a [Contributor Code of Conduct](CODE_OF_CONDUCT.md). By participating in this project, you agree to abide by its terms.

## License

This project is licensed under the GNU General Public License v3.0. You can use, modify, and distribute this project, even for commercial purposes. However, you cannot sell it. For more information, see the [LICENSE](LICENSE) file in this repository or visit [https://www.gnu.org/licenses/gpl-3.0.html](https://www.gnu.org/licenses/gpl-3.0.html).
