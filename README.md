# Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-
This project is cuffless cNIBP research platform with ECG(two-electrode, without Right Leg Drive(RLD) ) and PPG(MAX30100)

![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/v3%20demo.gif)

## Hardware

### V4

coming soon...

- STM32F405
- ECG : INA333+AD8669(without RLD)
- Electrode : PCB Pads and NeuroSky EEG dry 6mm electrode(Pinhole PAD1 & PAD2) [NeuroSky](http://neurosky.com)
- PPG : LArm
- LCM : 128x64 SSD1306
- OUTPUT : USB-FS VCP
- Charger : BQ24014

### V3

- STM32F405
- ECG : INA827+LM324PWR(without RLD)
- Electrode : PCB Pads and NeuroSky EEG dry 6mm electrode(Pinhole PAD1 & PAD2) [NeuroSky](http://neurosky.com)
- PPG : LArm
- LCM : 128x64 SSD1306
- OUTPUT : USB-FS VCP
- Charger : TP4056*

#### Note
- TP4056 have mV level peak-to-peak noise... so, jump VBUS to switch... ![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/without%20charger.png?raw=true)
- Replace D1 to 0R resistor, reduce LDO VDDA noise.
- Unplug the NB AC adapter.

</br>

![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/v3%20pic.jpg?raw=true)

### V1
- STM32F407
- ECG : INA827+LM324PWR(without RLD)
- Electrode : PCB Pads
- PPG : LArm and RArm
- LCM : 128x64 SSD1306
- OUTPUT : UART

## Firmware
This firmware only for testing hardware, includes USB-FS VCP, Max30100 Spo2 Mode, ADC-DMA and LCM.</br>

## Software
### [wxECGAnalyzer](https://github.com/GCY/wxECGAnalyzer) - cross platform ECG signal process tool and QRS-Complex detection algorithm validation
### [Pulse Oximeter MAX3010X](https://github.com/GCY/Pulse-Oximeter-with-MAX3010X) - Spo2 r-ratio finetune tool

</br>

![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/example.png?raw=true)

## What's cuff-less blood pressure monitor and difference between Pulse Arrival Time(PAT) and Pulse Transit Time(PTT)
In cuffless non-invasive blood pressure monitor field, we with the accurate calibration of PAT to BP, beat-to-beat BP can be estimated from PAT. On the basis of the theoretical relationship between PAT and BP and their experimental or empirical relationship, various models that correlate PAT with BP have been established.</br>
</br>
PAT and BP with Regression: </br>
![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/PAT%20and%20Regression.png?raw=true)

</br>
</br>
Pulse transit time (PTT) is the time it takes for the pressure or flow wave to propagate between two arterial sites.
PTT measured as the time delay between invasive proximal and distal blood pressure or flow, and PTT(propagate time) can be converted to Pulse Wave Velocity(PWV is speed unit).

the most used is the R-wave(QRS Complex) in the electrocardiogram (ECG) and combine pulsemeter(piezo or PPG), leading to the so called pulse arrival time (PAT). 
PAT is not exactly the PTT, as it includes the time interval between ventricular depolarization and the opening of the aortic valve which is known as pre-ejection period (PEP) and it varies beat-to-beat.

Most efforts have employed the time delay between ECG and finger photoplethysmography (PPG) waveforms as a convenient surrogate of PTT. 

However, these conventional pulse arrival time (PAT) measurements include the pre-ejection period (PEP) and the time delay through small,
muscular arteries and may thus be an unreliable marker of BP.


Shortcoming is that PAT includes the pre-ejection period (PEP) in addition to PTT. 
Since the PEP component depends on the electromechanical functioning of the heart, it can change independently of PTT and thus BP. 

Because the ECG QRS-Complex is not the starting point for blood to actually enter the radial artery, it is a biopotential to depolarization of the sinus node

</br>

## This project only for research</br>
</br>

![alt text](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-/blob/master/res/ecg%20osc.png?raw=true)

</br>
</br>

[![Audi R8](http://img.youtube.com/vi/lEdaMjn-bFg/0.jpg)](https://youtu.be/lEdaMjn-bFg)

</br>
</br>

### Reference :
- [1] Continuous Blood Pressure Measurement from Invasive to Unobtrusive: Celebration of 200th Birth Anniversary of Carl Ludwig</br>
- [2] Cuff-Less and Continuous Blood Pressure Monitoring: A Methodological Review</br>
- [3] https://www.egr.msu.edu/classes/ece480/capstone/spring13/group03/documents.html
 
