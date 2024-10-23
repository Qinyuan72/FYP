# General Purpose Data Acquisition System Using STM32 and FreeRTOS

## LM118 – Bachelor of Engineering in Electronic and Computer Engineering

### **Author:** Qinyuan Liu  
**Supervisor:** Dr. Ian Grout  
**Project Status:** Interim Report Submission  

## **Abstract**
The growing complexity of embedded systems has increased the need for precise, lightweight, and agile data acquisition systems, particularly in industrial automation and robotics. This project aims to develop a data acquisition system using the STM32 microcontroller combined with FreeRTOS, focusing on analog and digital I/O integration and communication functionality. The project will use STM32CubeIDE (C) as the development platform and NUCLEO-L476RG as the testing platform. Target 3001 Beta-Ver will be employed for schematics and PCB layout. The project’s primary goal is to complete a working prototype system by the end of the autumn semester, followed by PCB design in the winter semester, and finalize the project presentation and report in the final semester.

## **Table of Contents**
- [Project Overview](#project-overview)
- [Motivation and Objectives](#motivation-and-objectives)
- [Technical Details](#technical-details)
  - [Software](#software)
  - [Hardware](#hardware)
- [Project Plan](#project-plan)
- [Deliverables](#deliverables)
- [Conclusions and Future Work](#conclusions-and-future-work)
- [References](#references)

---

## **Project Overview**
The main focus of this project is to design and implement a general-purpose data acquisition system using STM32 and FreeRTOS. The system will support analog-to-digital (ADC) and digital-to-analog (DAC) signal acquisition along with digital I/O interfacing and communication capabilities.

This project will be carried out in three key steps:
1. **Breadboard Prototype:** Establish a proof of concept to test system fundamentals.
2. **PCB Prototype:** Develop a prototype board to integrate the design and demonstrate advanced capabilities.
3. **Full Chip-on-Board Design:** Implement a finalized STM32-based chip-on-board solution for industry-ready robustness and reliability.

The system is expected to find applications in fields such as industrial automation, civil engineering, machinery control, and safety systems.

## **Motivation and Objectives**
With the increasing complexity of embedded systems, the need for real-time data processing and precise control in industrial environments has grown. The goal of this project is to create a reliable data acquisition system that leverages the advantages of FreeRTOS for time-sensitive scheduling and STM32 for hardware control.

### **Key Objectives:**
- Implement a robust data acquisition system capable of handling analog and digital signals.
- Design and test a PCB layout for the final prototype using Target 3001 Beta Version.
- Utilize FreeRTOS for effective task management, timing control, and communication handling.
- Develop a well-structured and efficient firmware using STM32CubeIDE.

## **Technical Details**
### **Software**
The software component of the project is built using STM32CubeIDE, which provides a comprehensive development environment for STM32 microcontrollers. CMSIS-RTOS V2 is used as the RTOS platform for task scheduling and synchronization. Key features of the software design include:
- Integration of HAL (Hardware Abstraction Layer) functions for simplified interaction with peripherals.
- Task management and communication control using CMSIS-RTOS V2.
- A modular approach to ADC/DAC handling and digital I/O interfacing.

### **Hardware**
The hardware component is developed on the NUCLEO-L476RG development board. Key components include:
- **STM32 Microcontroller:** The core processing unit of the system.
- **Analog/Digital I/O:** 16-channel ADC/DAC integration and 16x16 digital signal I/O.
- **PCB Layout:** Designed using Target 3001 for efficient integration and connectivity.
- **Shift Registers and Parallel-to-Serial Converters:** For managing digital I/O efficiently.

The breadboard prototype serves as the initial testing platform, followed by the PCB prototype for advanced design.

## **Project Plan**
The project is divided into the following stages:
1. **Initial Setup:** Familiarize with STM32CubeIDE and FreeRTOS, and develop initial schematics using Target 3001.
2. **Breadboard Prototyping:** Implement a functional prototype on a breadboard, testing core features such as ADC, DAC, and digital I/O.
3. **PCB Design:** Develop the PCB layout and fabricate a prototype board. This stage involves schematic finalization, layout design, and component integration.
4. **Software Development:** Build and test firmware using CMSIS-RTOS, focusing on task management and communication functionality.
5. **Final Prototype:** Integrate all components into a final prototype and conduct system testing for robustness and reliability.

## **Deliverables**
- **Schematics of Each Block:** Complete and review detailed schematics of the hardware blocks.
- **Gantt Chart:** Maintain a detailed Gantt chart for effective project management.
- **Project Report:** Document the project with a clear focus on technical details and implementation steps.
- **Final Presentation:** Prepare a comprehensive presentation summarizing the project’s technical scope and outcomes.

## **Conclusions and Future Work**
Significant progress has been made in developing the foundational elements of this project, including schematic design and the breadboard prototype. Key challenges include the implementation of FreeRTOS and the transition from breadboard testing to PCB integration. The next steps involve refining the schematic design, fabricating the first PCB prototype, and conducting system-level testing.

Future work will focus on:
- Completing the PCB layout and fabrication.
- Enhancing the software design for optimized task management.
- Conducting extensive system testing and performance validation.
- Preparing the final report and presentation.

## **References**
- [1] STMicroelectronics, "NUCLEO-XXXXCX, NUCLEO-XXXXRX, NUCLEO-XXXXRX-P, NUCLEO-XXXXRX-Q: STM32 Nucleo-64 boards," Data brief, Rev 19, June 2024. Available: [https://www.st.com](https://www.st.com)
- [2] IntechHouse, "Real-Time Operating System in Embedded Systems," IntechHouse Blog, June 14, 2023. Available: [https://intechhouse.com/blog/real-time-operating-system-im-embedded-systems/](https://intechhouse.com/blog/real-time-operating-system-im-embedded-systems/)

---

## **Appendix**
- **Project Gantt Chart:** Documenting key project milestones and deliverables.
- **Interim Presentation Slides:** Overview of the project progress and technical details.

