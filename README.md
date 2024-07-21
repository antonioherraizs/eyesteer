# EyeSteer
Control a King Marine AP2000 autopilot with eye gaze technology through an Arduino, allowing a kid with celebral palsy steer a boat.

This repository contains the code that runs on the Arduino and explains how the solution works, but first a video demoing the end result:

(short video of Agustinito steering)

# Navegando con la mirada

Primero, el video de mi primo Agustin girando el timon de su barco "Punta Negra" con la mirada:

[video]

Mi primo Agustin tiene paralisis cerebral y se comunica a traves de una tablet que en lugar de raton o dedo maneja con la mirada gracias a [Grid 3 de Smartbox](https://thinksmartbox.com/grid/), una solucion para personas con dificultades en la comunicacion.

Hace un tiempo nos preguntamos si a traves de su tablet Agustinito podria pilotar su barco, al que le encanta ir siempre que puede con su padre. Para lograrlo seria necesario que Agustinito pudiera enviar la orden de giro a un dispositivo que girase el timon. Este dispositivo tendria que ser un arduino o similar, y a su vez tendria que ser posible que el piloto automatico pudiese recibir ordenes de una manera digital.

# Architecture
This diagram shows the components of the solution and describes the three-step sequence that leads to wirelessly steering the boat through eye gaze tech:

![architecture diagram](https://github.com/user-attachments/assets/dc9fc6ea-f8e1-4e62-971b-60a92f87ff08)

At a high level, the user sends a web request that reaches the arduino and instructs a digital potentiometer to simulate the turn of a knob on the IA2000 which translates into a turn of direction on the AP2000 autopilot.

Step 1: Web Request

My cousin Agustin controls a tablet running SmartGrid Grid 3 through eye sight. He connects to the Arduino's access piont and browses 192.168.4.1, a web page loads that allows for the steering of the boat. The webpage contains two buttons, each with a command to turn port or starboard:

![website interface](https://github.com/user-attachments/assets/2a2691f9-de93-4789-9a44-b31b86cdb956)

Step 2: IA2000 Control

The King Marine AP2000 autopilot just like any other boat autopilot lets users set a direction and then steers the helm to follow that direction using a compas. An additional component called the King Marine IA2000 interface can perform actions on top of the autopilot, modifying its behavior. In our case the most interesting functionality is the "Heading Trim", which overrides the autopilot's direction 10 degrees port or starboard (20 degrees total) by turning a knob on the IA2000.

This solution replaces the physical control of the Heading Trim knob on the IA2000 with a digital potentiometer controlled by an arduino:

![IA 2000 showing "HDG TRIM" knob](https://github.com/user-attachments/assets/2bd83b36-faa4-4c67-b1c2-4db5aebaff6c)

The wiring between the Arduino and the IA2000 consists of four cables:

| Wire Color | Description |
| ---------- | ----------- | 
| Red | 11-16V power supply, connected directly to the power received by the interface. The Arduino then regulates it to 3.3V |
| Black |	Ground (GND) |
| White | VCC, approximately 9V, the voltage at which the interface operates |
| Green | Digital potentiometer output |

The web server on the Arduino receives the request via HTTP and uses a digital potentiometer to act on the IA2000 interface by modifying the "HDG TRIM" knob value, replacing the existing analog potentiometer. This allows remote steering of up to 10 degrees to each side.

An alternative solution could be to control the "HDG TRIM" analog potentiometer mechanically instead of digitally, for example with a servo mounted on the "HDG TRIM" knob. This solution was not implemented since it clearly was less fun than messing with analog electronics and inserting a digipot in the circuit.

Another alternative would be to act on the "DODGE" switch of the IA2000 for a quick turn. This solution was tested but deemed unsuitable because the "dodge" function immediately returns to the original position after completion. This is by design.

Step 3: AP2000 Control

The IA2000 receives the instruction to "trim" port or starboard and sends the signal to the AP2000 autopilot to turn in the indicated direction.
