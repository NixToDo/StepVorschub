# StepVorschub
An electronic feed for my manual lathe.

My lathe have an feed and thread gearbox. But I have to change the gears
manualy, there is no Norton gearbox. So every time when I want to change the
feed rate or the thread pitch I have to disasseble the current gears
and mount the new ones. Maybe only for a short movement. Thats a lot of work!

Now I have replaced these gears by this electronic feed and thread controller.
It consists of:
- An Arduino Nano with an 0.9" GLCD and a manual rotary encoder for human interface.
- An quadrature encoder with high resolution to track the position of the main spindle.
- An stepper motor and the driver attached to the original feed / thread gearbox.

The handling is now much easier!
I only have to select the needed feed or thread pitch value on the controller.
And of course the right settings on the lathe. (Bed slide engagement, leadscrew or towbar, ...)

Written by Dirk Radloff, with contributions from the open source community.
GNU V3 license, check license.txt for more information. All text above must be
included in any redistribution.

Hosted on Github:
  https://github.com/NixToDo/StepVorschub

This software uses following libraries:
- For the display Adafruit SSD1306 and the top level GFX library:</br>
  https://github.com/adafruit/Adafruit_SSD1306</br>
  https://github.com/adafruit/Adafruit-GFX-Library
- For the manual quadrature encoder:</br>
  https://github.com/MajicDesigns/MD_REncoder
- For debouncing the manual encoder push button:</br>
  https://github.com/thomasfredericks/Bounce2
  
