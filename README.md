# EyeSteer
Control a King Marine AP2000 autopilot with eye gaze technology, allowing a kid with celebral palsy steer a boat.

# Architecture
This diagram shows the components of the solution and describes the three-step sequence that leads to maneuvering the boat:

(arquitecture diagram)

Step 1: Web Request

Agustinito's Grid 3 includes a web browser (https://grids.thinksmartbox.com/en/sensory-software/web-browser). When the browser visits 192.168.4.1, a web page loads that allows for the maneuvering of the autopilot. The interface contains 2 buttons, each with a command to turn port or starboard:

Step 2: IA2000 Control

The digital autopilot physically controls the "HDG TRIM" wheel using four cables:
Wire Color	Description
Red	11-16V power supply, connected directly to the power received by the interface. The Arduino then regulates it to 3.3V.
Black	Ground (GND).
White	VCC, approximately 9V, the voltage at which the interface operates.
Green	Digital potentiometer output.

The web server on the Arduino receives the maneuvering request via HTTP and uses a digital potentiometer to act on the IA2000 interface by modifying the "HDG TRIM" wheel value, replacing the existing analog potentiometer. This allows remote turning up to 10 degrees to each side.

An alternative solution would be with an additional mechanism that controls the "HDG TRIM" analog potentiometer mechanically instead of digitally, for example with a servo mounted on the "HDG TRIM" wheel. This solution was not tested.

Another alternative would be to act on the "DODGE" switch for a quick turn. This solution was tested but deemed unsuitable because the "dodge" function immediately returns to the original position after completion. This is by design.

Step 3: AP2000 Control

The IA2000 receives the action and sends the signal to the AP2000 autopilot to turn in the indicated direction.
