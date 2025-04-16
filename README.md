# Deaf-Curler-Assistive-Devices
Curling is a sport that heavily relies on verbal communication between teammates, particularly between the skip and the sweepers. Players who are deaf cannot receive commands verbally and must rely on different aids to understand what the skip is requesting. The Shh-weeper assists deaf players in receiving visual commands. The Shh-weeper consists of the skip device (transmitter) and multiple sweeper devices (receivers). There are eight LEDs located on the sweeper device that the function relayed to the sweeper. These commands are sent using the sliding potentiometer and 3 buttons on the skip device. The result is a more accurate and reliable device for receiving and delivering commands in a fast-paced environment

![5184080433590152683](https://github.com/user-attachments/assets/a3fa430d-db2e-48c8-8c9e-9ed9abed5d39)
![5184080433590152682](https://github.com/user-attachments/assets/2003a061-b118-4ac7-bea4-3dab6545a049)

## Background of the Invention
This device was developed because there was a lack of development in this space in curling for more inclusivity. Deaf sweepers were struggling with receiving commands and skips were also struggling with issuing them. For curlers (and athletes in general) who are deaf or hard of hearing, this presents a significant barrier to competitive participation as they may be unable to hear these critical instructions.
Current adaptations for deaf players are limited and often require visual cues or physical vibrations, which can be imprecise, distracting, or introduce delays that affect performance. Other issues are devices that provide too much information and make interpreting commands difficult. As inclusivity in sports becomes increasingly important, there's a growing need for assistive technologies that allow deaf curlers to receive timely, reliable, and non-verbal communication during gameplay.
The invention presented here seeks to eliminate these issues by being as simple as possible and assures that the need for visual aids is limited. The skip device can be used without looking and the sweeper devices display simple color combinations that are easy to decode into commands.
## Detailed Description of the Invention
The sweeper device is designed to be simple and minimalist with LEDs that are inclined to match the sweepers’ inclined position. There is also a micro-USB port for charging, and a power switch located at the back. There is a Velcro strap connected to the bottom to attach the device to the broom with ease. The casing holds the esp32 board and the battery. The skip device is a simple ergonomic remote that almost feels like a handle. There are three buttons place opposing the linear potentiometer such that the thumb can control the potentiometer, and the fingers wrap around the buttons. These provide commands to the skip device. The inside of the device holds the esp32 board and battery. The bottom of the device holds the power switch and a hole to attach the wrist strap to.

![Sweeper Device (10)](https://github.com/user-attachments/assets/8d1c50d0-c92d-438b-bd8d-d10038b2525d)
![Sweeper Device (7)](https://github.com/user-attachments/assets/8f54a250-8d14-4d04-90bd-97b0b6c75fe2)
![Skip V5 Right (2)](https://github.com/user-attachments/assets/fdfd39a2-04a7-457b-8c08-6e864715541b)
![Skip V5 Right](https://github.com/user-attachments/assets/ba76e7f4-7b04-467b-b839-ca5eb26c18f5)

The code includes two seperate projects. Both implemented on ESP32 microcontrollers. The skip device controls the commands and mapping to the sweeper devices. The devices interact wirelessly and pair easily. Battery life is estimated to be 8 hours.
