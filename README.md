What you need is an ESP32-CAM (AI Thinker compatible for this specific project)
An FTDI programmer.
A motion detector, I used one from AZ-Delivery (https://www.az-delivery.de/en/products/bewegungsmelde-modul)
I found those ones reliable. (Not the cheapest on the market though but in my opinion the best)
I presume you are somewhat familiar with how to connect an ESP32 to a FTDI programmer. If not this is a quick way.
Programming mode:
FTDI -> ESP
RX -> TX
TX -> RX
5V -> 5V
GND -> GND

On ESP32-CAM
IO0 -> GND Lock in programming mode
RST -> so it is ready for use Just a quick touch so you see on a serial monitor that it is ready for programming.

Upload the code.
Remove IO0 from GND and a quick reset then it should be running.

Now over to the more important things.
Since there is absolutely no useful GPIO's on the ESP32-Cam it is almost difficult to use it with an external sensor if you want to be able to use camera, SD-card, and WiFi!
I despaired for several hours trying to find a solution. Then I did a quick test on the GPIO's that I had to see if there was anything useful. I recommend you to check the schematics for your ESP.
After a few hours of self pity I started to think, do I really need serial communication? It's good to have when testing but if I get it working I don't need it. But I needed it to see if my idea should work. So TX on the ESP I needed, it's an output anyway, but the RX? Not when the programming is done. Perhaps use that one? I did and that worked!

The project kept on rolling. I am sorry if you don't understand my remarks in the code since some are in Swedish, originally all where, but translating them to English seems a bit boring. (And what is the fun if you don't have to use google translate?)

The code itself is rather simple, I am not at the moment in the condition to do any advanced programming.
Oh! I use a 16GB SD-Card I read somewhere that an ESP32 only handles 4 GB and on other places that it can handle up to 64 GB I checked and the smallest I had was 16GB and it works perfect.
GPIO 3 is marked on my board as UOR and on some UORX and so on.

This is a rather confused and jumping around reade file, but I hope you get the picture.

Ok over to the program it self!
The libraries portion looks like this:
#include "esp_camera.h"
#include <FS.h>
#include "SD_MMC.h"
#include "EEPROM.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
I use the EEPROM to keep count of the filenames. img1.jpg and so on and this line : EEPROM.begin(9); sets it to a 9 bits counter.
I don't want to many pictures in the root. or "/" for those that isn't used to the root. I try to have as much as possible in functions because I find it a lot easier to mess around with the code then. So the initialization of the SD-card is done in one and the reading of the files in another and the moving of the files when the counter hits 511 in yet another.

To reset the EEPROM counter to 0 I use a simple call to a function
void resetEEPROM(){
unsigned int resetValue = 0;
EEPROM.put(0, resetValue);
EEPROM.commit(); // Write changes to the EEPROM.

Serial.println("Counter is at 0.");
}
Since I might need the old files they are moved to another Directory on the SD-Card. This process is rather straight forward and is full in the complete code. (Function moveJpgToOldPictures ):
Question? Ask me I don't promise I will be able to answer but I'll try.
