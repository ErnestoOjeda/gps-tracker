# wander-hardware

### Overview

The Wander v1.1 prototype is an adaptation of the Particle LTE line of industrialized cellular IoT modules. It is based on the Particle E-Series LTE module and includes an onboard GPS module based on the MTK-3339 chipset.

### Description

#### Front

![alt text](https://github.com/wander-inc/wander-hardware/blob/master/images/wander-v1.1%20front%20numbered.png)

#### Back

![alt text](https://github.com/wander-inc/wander-hardware/blob/master/images/wander-v1.1%20back%20numbered.png)


| ID 					    | Description                                      |
|:--------------------------|:-------------------------------------------------|
| **LiPo Battery Contacts**        | This is directly connected to the LIPO pin. The recommended input voltage range on this pin is between 3.6V to 4.4VDC. Make sure that the battery can handle currents of at least 3Amp.  |
| **PA6H GPS module**           |MTK-3339-based GPS module that can track up to 22 satellites on 66 channels, with an excellent -165 dB sensitivity receiver, and a built in antenna.|
| **GPS FIX LED**     | This LED blinks at about 1Hz while it's searching for satellites and blinks once every 15 seconds when a fix is found to conserve power.|
| **Charge status LED**     | Pretty self-explanatory.|
| **RGB LED**        | Displays cellular connectivity status.|
| **u-blox cell module**    | This is the u-blox SARA module.|
| **uFl antenna connector** |Plug the antenna here. |
| **E Module USB port**       | This is the module's microUSB port that connects to the STM32F microcontroller.|

### Components List

* u-blox SARA R410M LTE modem
  * LTE CAT M1/ NB1 module with global hardware support
  * 3GPP Release 13 LTE Cat M1
* MediaTek MTK3339
  * SBAS, Galileo, GLONASS, GPS / QZSS
  * Supports WAAS/EGNOS/MSAS/GAGAN
  * 22 tracking / 66 acquisition-channel GPS receiver
* STMElectronics LSM303ADRTR
  * 3 magnetic field channels and 3 acceleration channels
  * Embedded temperature sensor
* STMicroelectronics STM32F205RGT6 
  * 120MHz ARM Cortex M3 microcontroller
  * 1MB flash, 128KB RAM 
  * Supports DSP instructions, hardware accelerated FPU calculations 
* MAX17043 fuel gauge
* Micro USB 2.0 full speed (12 Mbps)
* FCC and PTCRB certified
* RoHS compliant (lead-free)

### Structure of this Repository
 - /bom
     + Contains the electronics bill of materials outlining the components used
 - /cad-cases
     + Contains .f3d files for various looks-like/works-like case designs
 - /datasheets
     + Contains the relevant datasheets of the components used
 - /documentation
     + Contains prototype PRD and Working Specification & Requirements sheets
 - /eagle
     + Contains the schematic and PCB files in the Eagle file format
 - /gerbers
     + Contains PCB gerber files for manufacturing
 - /images
     + Contains the images used in the description


### Change Log
#### v1.1.0 (LATEST)
* initial beta release
