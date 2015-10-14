# taciturn-prune
Arduino Temperature Sensor<br>
<br>
This project creates a temperature controller that can be programmed with a temperature schedule. The schedule can also be overridden by changing the current setting, in which case it becomes just like a commercial temperature controller that you have to update anytime you want the temperature to switch. The schedule is currently hardcoded in the code, so the Arduino must be programmed from a computer to update it. This also included a Real Time Clock chip that allows the controller to resume the schedule from where it would be if the power goes out unexpectedly. Also, the user can reset the chip and start the schedule over if they are brewing the same recipe and want to execute the same schedule from the beginning by holding down button 3 for 5 seconds.<br>
<br>
Parts:<br>
Arduino UNO<br>
<br>
DS18B20 Waterproof temperature sensor<br>
4.7 kOhm pull-up resistor<br>
<br>
16x2 LCD screen<br>
10k Ohm Potentiometer<br>
1N4004 rectifier diode<br>
22 kOhm resistor<br>
2N2222 NPN transistor<br>
<br>
3 tact switches (buttons)<br>
100 kOhm resistor<br>
47 kOhm resistor<br>
22 kOhm resistor<br>
10 kOhm resistor<br>
<br>
DS3231 RTC Module<br>
2Channel Relay module<br>
<br>
Extension cord - split the live wire with the relay bridging the connection when activated. Please be very sure of what you are doing. Do NOT electricute yourself accidentally!<br>

![alt tag](https://raw.github.com/taciturn-prune/blob/master/tempsensor_bb.png)
