# heltec-lora-boatreadings
reading sensors from my boat to determine bilge water level remotely


<img width="2559" height="1571" alt="image" src="https://github.com/user-attachments/assets/3fcaf7df-5e79-4d4f-9bc6-f95a40567a9a" />


## Why this project?
I found out the hard way that a bilge pump setup can falter, which in my case resulted in a sunken boat (sloop, in Dutch: sloep).
My bilge pump is fed by a 12V battery, which is charged by a small solar panel. Usually this works fine: my solar charge controller lets ths solar panel charge the battery, and protects the battery from getting depleted below a certain level (when there was a lot of rain and no sun and the bilge used a lot of power). 
As long as there is enough sunshine, and no electricity connection is broken due to storm, rain or bad luck, the bilge pump automatically pumps rain water out of the boat. 
Because electricy connections can break, or battery level can get too low, I needed to remotely measure the battery level and the bilge water level.

## How does the measurement look like?
###No rain: 
during night, the battery level slowly drops by a volt or so. When the sun is out, it is slowly charged to nominal level during day, while the bilge water level stays the same.

###Rain: 
the bilge water level in the boat slowly rises, this is seen by the "distance" getting lower, until the battery level shows a sudden dip in voltage when the bilge pump kicks in. After this happend, the bilge water level is back to nominal (distance = nominal again).


![rain](https://github.com/user-attachments/assets/691e04c9-6ebb-45cb-835e-a7ef8f7fecb4)



# How is this built?
## Sensors used in this project:
Besides water level monitoring and the battery level, this project also monitors the air humidity, temperature and pressure as a bonus ;-).

Distance is measured via a transponder as drawn above. Below is a picture of the tube which sticks into the bilge.
Battery level is measured through a voltage devider, which is measured by the arduino board.
Bonus measurements humidity, pressure and temperature are measured by a BME280 connected to the arduino board

## Lora and Arduino board:
Heltect AB01 V2 board.

## Lora network:
The Things Network

## Home Assistant
Data is displayed through Hone Assistant, using Node Red to retriev data from TTN and store this in HA entities which are made visible in a lovelace UI.

Node Red in Home Assistant:
TTN → MQTT → Node-RED → auto discovery in HA.

