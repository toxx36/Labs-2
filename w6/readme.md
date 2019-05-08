# Task

Develop two devices using STM32F4 DISCOVERY boards:

- One representing a light bulb (ZC)
- One representing a remote controller (ZED or ZR)

Remote controller has two buttons:

- One of them triggers a toggle action
- The other one triggers a step up function (“wrapping” to zero)
- Pressing both buttons at the same time forces the light bulb to change color   

Document API for light bulb control protocol.

# Lib description

### libled
Library for LED color setting (HEX, RGB). It provides:
- Init GPIO and timer in PWM mode for LED control
- Set color as RGB via array of 3 values (Red, Green and Blue) from 0 to 255
- Set color by HEX color code, for example 0xFACE8D
- Color value converts from linear value to parabolic for almost linear eye perception of brightness change.

### libbuttons
It provides button handling with debouncing. You can find out is the button:
- on press;
- was pressed;
- on hold;
- was holded;
- was released;
 
 And also click with repetition (like at conventional keyboard) and get count of fast clicks.

### libbulb_send
Library for a remote. There is optimized payload size: not used more bytes than a specific command really needs. The protocol support the following commands:
- Enabling/disabling a light bulb
- Toggling a light bulb
- Brightness: step up, step down, set to level
- Toggle the color (we’ll assume that the bulb has a predetermined set of colors)
- Used APS payload only, not clusters, endpoints, etc.

### libbulb_receive
Library for a bulb. It decodes commands from the remote.
