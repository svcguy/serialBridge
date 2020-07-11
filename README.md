# serialBridge
A Qt application to use the ST-LINK V3 Bridge

**** WORK IN PROGRESS ****  
** USE AT YOUR OWN RISK **  

A complete Qt Creator project that builds the ST-LINK V3 BRIDGE code from ST found [here](https://www.st.com/en/development-tools/stlink-v3-bridge.html) (registration required to download)  
It also builds an (currently only Windows) application that allows the user to interact with the bridge and ultimately the target attached to the bridge  
  
Current functionality:
+ Builds the ST-LINK-V3-BRIDGE.dll
+ Builds the serialBridgeApp
  The app currently:
    + Loads the STLinkUSBDriver.dll
    + Enumerates the attached devices
    + Connects/Disconnects from a device
    + Gathers verison info from the DLLs, firmware versions from the device
    + Provides basic GPIO control functionality
    + Provides basic I2C read/write functionality
  
Plans are to:
+ Implement the various protocols such as ~~GPIO~~, ~~I2C~~, SPI, and CAN (UART is provided through the ST-LINK VCP)
+ Make it cross platform
