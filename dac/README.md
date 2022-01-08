# Example VGA DAC implementations.
## About VGA
VGA (Vectored graphics array) uses 3 analog wires for the colors RED, GREEN, BLUE. and 2 digital control wires for Horizontal and Vertical sync (HSYNC,VSYNC).
The analog input(s) are termintated on the monitor with a 75-ohm pull-down resistor to GND (AGND).  The peak-to-peak voltage level on RGB is 0.7v.

Below are examples on how to create very simple digital to analog converters using digital logic.  These assume you're driving 3.3V TTL logic in push-pull (not open drain aka Hi-Z).

Modeled with [Falstad circuit simulator](https://www.falstad.com/circuit/circuitjs.html).
### Simple 2-BIT D to A converter
![Simple 2-bit VGA DAC](2-bit%20VGA%20DAC%20simple.png)
### Better 2-BIT D to A converter
![Better 2-bit VGA DAC](2-bit%20VGA%20DAC.png)
### 3-BIT D to A converter
![3-bit VGA DAC](3-bit%20VGA%20DAC.png)
### 4-BIT D to A converter
![4-bit VGA DAC](4-bit%20VGA%20DAC.png)

