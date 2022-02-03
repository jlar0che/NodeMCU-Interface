# NodeMCU Graphical and Software Interface
Provides WiFi connectivity for the NodeMCU with numberous on-screen feedback  elements

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Downloads Stats][github-downloads]][github-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

---
<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage-example">Usage Example</a></li> 
    <li><a href="#release-history">Release History</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contact-and-info">Contact and Info</a></li>
    <li><a href="#acknowledgments">Acknowledgements</a></li>
    <li><a href="#contributing">Contributing</a></li>
  </ol>
</details>

---

<!-- ABOUT THE PROJECT -->
## About The Project

<img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/NodeMCU.jpg?raw=true">


Modified UI and logic for the Kube project using the ESP8266 and DHT22 temperature/humidity sensor. A webpage will be served by the unit where you can view sensor data without using the MQTT network.


### Built With

Below are all the parts, tools libraries and frameworks used in this project. Additional details can be found in the acknowledgements section.

* [NodeMCU ESP8266 v0.9](https://www.nodemcu.com/index_en.html)
* [NodeMCU Breakout Board](https://www.tindie.com/products/bkpsu/nodemcu-breakout-board/)
* [DHT22](https://www.adafruit.com/product/385)
* [OLED Screen (.96" | 128x64 pixels | Yellow and Blue text)](https://www.amazon.com/HiLetgo-Serial-128X64-Display-Yellow/dp/B06XRCQZRX/ref=sr_1_1?keywords=HiLetgo%2B0.96%22%2BI2C%2BIIC%2BSPI%2BSerial%2B128X64%2BOLED%2BLCD%2BDisplay%2B4%2BPin%2BFont%2BColor%2BYellow%26Blue&qid=1643806789&sr=8-1&th=1)
* [3D Printed Case](https://www.thingiverse.com/thing:2539897)
* [Arduino IDE](https://www.arduino.cc/en/software)
* [Adafruit Unified Sensor by Adafruit (v 1.0.2 or higher)](https://www.arduinolibraries.info/libraries/adafruit-unified-sensor)
* [ArduinoOTA by Juraj Andrassy (v 1.0.0 or higher)](https://www.arduinolibraries.info/libraries/arduino-ota)
* [DHT Sensor Library by Adafruit (v 1.3.0 or higher)](https://www.arduinolibraries.info/libraries/dht-sensor-library)
* [ESP8266 and ESP32 Oled Driver for SSD1306 Display by ThingPulse, Fabrice Weinberg (v 4.0.0 or higher)](https://www.arduinolibraries.info/libraries/esp8266-and-esp32-oled-driver-for-ssd1306-displays)
* [ESP8266_mDNS by Mr. Dunk (v 1.17 or higher)](https://www.arduinolibraries.info/libraries/esp8266_mdns)
* [WiFiManager by tzapu and maintained by tablatronix (v 0.12.0 or higher)](https://www.arduinolibraries.info/libraries/wi-fi-manager)


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

This repo assumes that you have an ESP8266 housed in a specially configued "Kube" 3D Printed case according to the instructions found on BK Hobby's [YouTube page](https://www.youtube.com/watch?v=fA91LcJRbhI&t=368s).

### Installation

- **Download and install the newest version of the Arduino IDE:** <br>
Navigate to https://www.arduino.cc/en/software in your preferred web browser, then download and install the Arduino IDE version for your Operating System.


- **Add the ESP8266 Board Manager:** <br>
Launch the Arduino IDE and open the Preferences menu by navigating to "File" &rarr; "Preferences" or by pressing <kbd>Ctrl</kbd> + <kbd>,</kbd>. In the "Additional Board Manager URLs" field enter the following URL (note that you can add multiple URLs by separating them with a <kbd>,</kbd>):
```
https://arduino.esp8266.com/stable/package_esp8266com_index.json
```
<img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/Arduino%20Preferences%20Screenshot%20-%20Additional%20Boards%20Manager%20URL%20entered.jpg?raw=true" alt="Arduino-Preferences-Screenshot-Additional-Boards-Manager-URL-entered" width="640"><br>

- **Install ESP8266 Board in the Boards Manager:** <br>
Open the Boards Manager by clicking on "Tools" &rarr; "Boards" &rarr; "Boards Manager". In the search field type in "ESP8266" and then click on the "Install" button. Make sure to select your ESP8266 board from the menu after installation.<br>
<br><img src="https://i.ibb.co/nfxpn1r/Arduino-Board-Installation-Cropped.gif" width="1000"><br>

- **Install the newest versions of all necessary Arduino Libraries (**see "Built With" section above for complete list**):** <br>
Open the Library Manager by clicking on "Tools" &rarr; "Manage Libraries", or by pressing <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>i</kbd>. In the search field type in "ESP8266" and then click on the "Install" button. Make sure to select your ESP8266 board from the menu after installation. <p> <img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/Arduino-Library-Installation.gif?raw=true" width="1000">

- **Upload the Arduino sketch to your ESP8266:** <br>
Download the Arduino sketch from this Github Repository, open it in your Arduino IDE and modify the pertinent parameters -- WiFi SSID name and password along with the name of the location where the device will live. Connect your ESP8266 to your computer with a USB cable. Determine what port your ESP8266 is occupying on your machine, then choose that port in the Arduino IDE by going to "Tools" &rarr; "Port" and clicking on the correct port. Please note that if you are having trouble connecting to the ESP8266 - especially if you are in the Windows OS and see an unknown device in Device Manager - this oddly enough can be caused by the actual USB cable you are using. Try another USB cable until you can confirmation that you can successfully communicate with the ESP8266 in Arduino IDE by clicking on "Tools" &rarr; "Get Board Info" <br><br> <img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/Arduino-Uploading-Sketch.gif?raw=true" width="1000">

- **Get the IP Address of the Unit:** <br>
When the unit powers on it will tell you which Wireless SSID it is attempting to connect to and what IP address it has obtained after connecting. Jot down the IP address. Note that if you either don't want to connect to a WiFi hotspot or for any reason you are unable to you will still be able to see the realtime Temperature and Humidity picked up by the DHT22 via the OLED display.   
  
  [![Turning on the unit](https://img.youtube.com/vi/s76lNv9_MeE/0.jpg)](https://youtu.be/s76lNv9_MeE)

- **Connect to the Unit via Web Browser:** <br>
Now you can simply open your preferred web browser and enter the unit's IP address in the address bar. 
<br>
<img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/NodeMCU_ESP8266_-Temp-Humid-Webpage.gif?raw=true">
<p align="right">(<a href="#top">back to top</a>)</p>

## Usage Example

Because the component parts are inexpensive and building these units is relatively simple, for IT personnel an ideal usecase for this project is to liberally gather temperature and humidity readings of spaces not normally measured. That is to say, multiple locations in server rooms, within server racks and throughout key locations at sensitive sites.    
  
A dashboard could then easily be built to view the aggregate readings.
  
Please note that modifying the web interface can be done by editing the Skech  within the "// Web Server Code" section. Specifically, the String ptr defined in the the String SendHTML block is where the action happens.  
  
HTML pages can be converted to C/C++ strings via various tools like [this online string converter](http://tomeko.net/online_tools/cpp_text_escape.php?lang=en) at tomeko.net. After converting the HTML you would just need to introduce it to the sketch as the String ptr within the String SendHTML block just like the current page is. Each line is added with ptr += "new line of C/C++ converted string"; with the return ptr; statement finishing things off.

<img src="https://github.com/jlar0che/NodeMCU-Interface/blob/main/README-files/HTML-Code-in-Sketch.gif?raw=true" width="1000"><br>          

<p align="right">(<a href="#top">back to top</a>)</p>

## Release History

* 0.5.6
    * CHANGE: Small fixes to webserver design. Updates to Library requirements.
* 0.5.5
    * CHANGE: Substantial modifications to the webserver design.
* 0.5.4
    * general optimizations.
* 0.5.3
    * Added webserver.
* 0.5.0
    * The first proper release.
* 0.1.0
    * Work in progress.

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

- [x] Add Webserver
- [x] Add OLED Screen Info about IP Address
- [x] Add OLED Screen Info about initial SSID Connection Status
- [ ] Add Email notifications
- [ ] Add Ability to save sensor history

See the [open issues](https://github.com/jlar0che/NodeMCU-Interface/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#top">back to top</a>)</p>

## Contact and Info

Jacques Laroche – Twitter [@jlar0che](https://twitter.com/jlar0che)

Project Link: [https://github.com/jlar0che/NodeMCU-Interface](https://github.com/jlar0che/NodeMCU-Interface)

Distributed under the GPLV3 license. See [GPLV3 LICENSE DETAILS](https://choosealicense.com/licenses/gpl-3.0/) for more information.


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

Big thanks to the following makers, resources and tools. Couldn't have built this project without you!

* [BK Hobby Github Page for NodeMCU](https://github.com/bkpsu/NodeMCU-Home-Automation-Sensor)
* [BK Hobby YouTube Channel](https://www.youtube.com/channel/UC82xXciVxsQKthONKeYbhnw)
* [BK Hobby's Thingiverse Page for The Kube](https://www.thingiverse.com/thing:2539897)
* [W3Schools CSS Templates](https://www.w3schools.com/w3css/w3css_templates.asp)
* [Tomeko.net Text to C/C++ String Converter](http://tomeko.net/online_tools/cpp_text_escape.php?lang=en)

<p align="right">(<a href="#top">back to top</a>)</p>

## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork it (<https://github.com/jlar0che/NodeMCU-Interface>)
2. Create your feature branch (`git checkout -b feature/Branchname`)
3. Commit your changes (`git commit -am 'Add a message'`)
4. Push to the branch (`git push origin feature/Branchname`)
5. Create a new Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
<!-- ----------------------------------------------------------------- -->

<!-- BADGES SECTION -->
[contributors-shield]: https://img.shields.io/github/contributors/jlar0che/NodeMCU-Interface?style=for-the-badge
[contributors-url]: https://github.com/jlar0che/NodeMCU-Interface/contributors

[forks-shield]: https://img.shields.io/github/forks/jlar0che/NodeMCU-Interface?style=for-the-badge
[forks-url]: https://github.com/jlar0che/NodeMCU-Interface/network/members

[github-downloads]: https://img.shields.io/github/downloads/jlar0che/NodeMCU-Interface/total?style=for-the-badge
[github-url]: https://github.com/jlar0che/NodeMCU-Interface


[stars-shield]: https://img.shields.io/github/stars/jlar0che/NodeMCU-Interface?style=for-the-badge
[stars-url]: https://github.com/jlar0che/NodeMCU-Interface/stargazers

[issues-shield]: https://img.shields.io/github/issues/jlar0che/NodeMCU-Interface?style=for-the-badge
[issues-url]: https://github.com/jlar0che/NodeMCU-Interface/issues

[license-shield]: https://img.shields.io/github/license/jlar0che/NodeMCU-Interface?style=for-the-badge
[license-url]: https://github.com/jlar0che/NodeMCU-Interface/blob/main/LICENSE

[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/jacques-laroche-07032b174/

<!-- SCREENSHOT below "About The Project" -->
[product-screenshot]: https://i.ibb.co/cLWVJ4Q/NodeMCU.jpg

<!-- WIKI LINK in "Usage Example" -->
[wiki]: https://github.com/yourname/yourproject/wiki
