# HeartrateDriver
I2C Based 32-bit Driver for the MAX30100 Heartrate Sensor 


Driver purpose:​
Uses SERCOM7 I2C PLIB to interface with the Maxim MAX30100 heartbeat sensor mounted on a Mikroe Click​

Features:​
- Automatic initialisation of sensor parameters​
- Processing and filtering of raw data​
- Driver is single instance, multiple client​
- Single instance because there is no specific need for multiple heartrate sensors working concurrently​
- Multiple client​

Application example:​
   - SAME54 XPlained Pro​
     
   ![image](https://github.com/Eduard-Epurica/HeartrateDriver/assets/64744850/6deda805-b961-4b6e-ac05-fd79195a7f08)
   - Mikroe Click board with adapter connected on EXT2/EXT3 ​
     
   ![image](https://github.com/Eduard-Epurica/HeartrateDriver/assets/64744850/0072629d-04d1-4daa-9d76-802eefc3c924)

Features heartbeat calculation, measurement and display on UART console
