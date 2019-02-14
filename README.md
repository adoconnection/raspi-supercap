# raspi-supercap
Open source Raspberry Pi supercapacitor UPS hat

## Goal 
Make reliable turn on / shutdown shield for Raspberry Pi

## Status
PoC assembly

(!) Not yet tested ;)
![v1](https://github.com/adoconnection/raspi-supercap/blob/master/PCBv1.0.PNG)
![v1](https://github.com/adoconnection/raspi-supercap/blob/master/PCB-B.1.PNG)

## BOM
* PCB – https://easyeda.com/adoconnection/raspi-supercap
* 20F x 2 supercapacitors
* ATMega328
* resistors
* diodes
* MT3608 DC-DC Step up converter board

## Open questions
- [x] Use special supercapacitor chargers or not? (https://www.analog.com/en/ltc3225) – not in v1

## To-Do checklist 
- [ ] Firmware
- [ ] Monitor PI state
- [ ] Call shutdown pin
- [ ] On power on wait for capacitors to be charged and turn on Raspberry
- [ ] I2C slave to query expternal power status and capacitors charge level
- [ ] Reset Raspberry pi if shutdown was activated and power is restored
- [ ] Customizable power off behavior

![v1](https://github.com/adoconnection/raspi-supercap/blob/master/Workflow.png)

## I2C 
Default bus address is 0x77
Registers:

| Address  | Mode | Default value | Info |
| ------------- | ------------- | ------------- | ------------- |
| 0x01  | R  | 0x64 | Test register, always return 0x64  |
| 0x02  | R  |  | External power connected. Return 0x00 or 0x01  |
| 0x03  | R  |  | Capacitor charge in percents 0 - 100  |
| 0x04  | RW  | 3 | Turn off timeout in seconds. Wait for N seconds before issue a turn off command.  |
| 0x05  | RW  | 50 | Turn off threshold. Shutdown pi at 50% charge left. 0xFF - disable shutdown |
| 0x06  | RW  | 30 | Turn on threshold. Pi boot at 40% charged capacitors. Valid values 0 - 100  |
| 0x50  | RW  | 0x77 | I2C address |



## Sources of inspiration
* https://juice4halt.com/
* https://www.saveyourpi.com/
* https://www.youtube.com/watch?v=m20o3P-S5s8
* http://www.hackerspace-ffm.de/wiki/index.php?title=Raspi_EDLC_UPS
* https://raspberrypi.stackexchange.com/questions/74641/how-to-make-a-simple-reliable-15-second-ups-for-shutdown
* https://raspberrypi.stackexchange.com/questions/22579/pi-headless-how-to-confirm-that-the-pi-is-shut-down
* https://raspberrypi.stackexchange.com/questions/5489/how-do-i-turn-on-my-raspberry-pi-after-shutdown
* https://code.google.com/archive/p/tinkerit/wikis/SecretVoltmeter.wiki
* https://www.arduino.cc/en/Tutorial/MasterWriter
* https://raspberrypi.stackexchange.com/questions/3704/i2c-library-for-mono-c
