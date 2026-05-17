OpenRGB Adalight Legacy Plugin  
A plugin for OpenRGB that restores support for classic Adalight / Ardulight serial‑based devices.
Many older DIY Ambilight setups rely on the original Adalight protocol (as used by Prismatik/Ambibox and early Arduino sketches), but modern RGB ecosystems no longer support these devices natively.

This project includes code derived from the Prismatik (Lightpack) project by Mike Shatohin (brunql), licensed under GPL‑2.0.  
Original source: https://github.com/psieg/Lightpack

This plugin brings them back to life by providing:
Full compatibility with the original Adalight serial protocol
Support for any LED count
Standard Adalight header (Ada, LED count, checksum)
RGB frame packing identical to Prismatik/Adalight
Serial output via libserialport
Integration with the OpenRGB ecosystem (effects, profiles, SDK)

Goal  
Keep classic Adalight hardware usable and seamlessly integrate it into modern OpenRGB setups.

This project is intended for users who want to continue using their existing Adalight hardware without giving up the flexibility and features of OpenRGB.

Current Status:
Initial working Version with options for com Port, Baud Rate, LED Count and Color Order.

<img width="542" height="466" alt="adalightplugin" src="https://github.com/user-attachments/assets/7f3c1de9-7bb7-4343-b7c2-d1811e0d28ce" />

Install:
Copy the .dll into the OpenRGB Plugins Folder 
In most cases: C:\Users\your_user_name\AppData\Roaming\OpenRGB\plugins


I recommend using the Artemis2 program for Ambilight screen capturing. It includes an OpenRGB plugin that allows you to route the Ambilight signal to OpenRGB.
https://artemis-rgb.com/
This also lets you sync older or DIY Adalights with modern devices like keyboards or motherboards.
Artemis also offers better performance for screen capturing than most other programs.


Be sure to check out our gaming radio streams. We feature the most beautiful games here, and they’re perfect for enjoying Ambilight :)
https://www.youtube.com/@NeoGamingVibes/streams



