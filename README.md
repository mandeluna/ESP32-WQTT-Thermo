# ESP32-WQTT-Thermo
An Arduino sketch for ESP32 that reads temperature from a type-k thermocouple and publishes it

## Hardware
The following parts were used to test this code, but you could make substitutions with minor effort:
1. Dev Kit for ESP-WROOM-32 [Digi-Key](https://www.digikey.ca/en/products/detail/ESP32-DEVKITC-32D/1965-1000-ND/9356990)
2. A generic 16x2 Blue Character LCD with Backlight [Tayda Electronics](https://www.taydaelectronics.com/lcd-display-16-x-2-blue-character-with-backlight.html) — I bought this way back in 2013, AdaFruit has heaps of them
3. Adafruit i2c / SPI character LCD backpack [In Stock](https://www.adafruit.com/product/292) — I also bought this in 2013, 10 years later did something with it
4. MAX31850K Thermocouple Amplifier [Adafruit](https://www.adafruit.com/product/1727)
5. 4-channel Logic Level Converter [Adafruit](https://www.adafruit.com/product/757)
6. K-type Thermocouple (you can use the one from Adafruit, but I ordered a cheap one from Amazon)

## Tutorials
I mostly followed the Adafruit tutorials and examples from the various libraries.

1. [Adafruit I2C Liquid Crystal](https://learn.adafruit.com/i2c-spi-lcd-backpack/arduino-i2c-use)

If you're using the ESP32 note that the CLK and DAT pins are labeled SCL (GPIO 22) and SDA (GPIO 21). The sketch will fail to initialize if you use the Arduino pins in the above tutorial.

2. [Thermocouple Amplifier](https://learn.adafruit.com/adafruit-1-wire-thermocouple-amplifier-max31850k/wiring-and-test)

I found that if I used parasitic power the temperature measured was about 6-7ºC warmer than with external power. In my office external power recorded a temperature of 24ºC but with parastic power it was recording 31ºC). Similarly if you set Vin to 3.3V it was still too warm. When I put it to the 5V rail I got much better results. I didn't find out why.

3. [PubSub Client](https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_basic/mqtt_basic.ino)

I used Nick O'Leary's MQTT Pub Sub client and tested it using Home Assistant's MQTT plugin. The details of getting that working were interesting but not in the scope of this sketch.

The tutorial I followed is by [Rui Santos](https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/) but I didn't use Node-RED.
