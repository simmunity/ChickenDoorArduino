# Chicken Coop Automatic Door Controller

Beta version developed on original DIP Arduino Uno using Seeed Studio Grove Shield and Grove light sensor, two Grove relays, a Grove LED (which can be replaced with a Grove relay to enable a coop heater when the coop door is closed), a mini 2 axis joystick with push to select switch (custom wired to Grove connectors) and a Sunfounder 4x20 display with I2C daughter board connected using a customized Grove cable. Lastly a DS3231M I2C RTC module is connected with a soldered on Grove connector and standard Grove cable.

The joystick can select MANUAL override mode forcing the door OPEN by pushing it right, or disable MANUAL mode by pushing it left. By pushing the joysticks select switch, the display changes to setup mode allowing the time, date and morning and night hours to be adjusted. Morning and night settings are there to force the door closed regardless of the light value that the light sensor produces which has a night time threshold of 100 with a multi-sampled average value over 20 seconds using a circular queue.

I have a Bud 4x6x4 case with clear lid and internal panel that the whole kit with power supply will be installed in with display and joystick and light sensor exposed. I have a 2" travel, 12 volt linear actuator that has been fitted with hinges to provide the proper amount of swing of the door. The relays are powered for approximately 10 seconds when opening or closing and assume the linear actuator has internal limit switches. The relays are wired such that when both are off, they are both connected to ground, when one is on, that determines the polarity of the DC either opening or closing the actuator. The Arduino and actuator are powered by a 1.5 amp, 12 volt power supply.

![alt text](https://github.com/simmunity/ChickenDoorArduino/blob/master/ChickenDoorDisplay.jpg "This is the display when not in setup")

![alt text](https://github.com/simmunity/ChickenDoorArduino/blob/master/ChickenDoorHardware.jpg "Here is the hardware")

![alt text](https://github.com/simmunity/ChickenDoorArduino/blob/master/ChickenPanelBack.jpg "The back of the control panel")

![alt text](https://github.com/simmunity/ChickenDoorArduino/blob/master/ChickenPanelFront.jpg "Front of the control panel")
