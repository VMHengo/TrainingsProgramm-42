<h2>"TrainingsProgramm 42" - A Lasertag target shooting game</h2>
<a href="https://youtu.be/itRIaU-dj6U">
  <img src="https://img.youtube.com/vi/itRIaU-dj6U/0.jpg" width="600"/>
</a>

## Description
This was a university project about screenless games. We decided to build a fast paced reactive multiplayer game about shooting rapid blinking targets with lasertag pistols.

## Project Realization
Inside the 3D printed gun, we used an Infrared-LED to send signals which are received by each target individually by an IR-receiver. We expanded on this basic idea with focus on user experience by including inside the gun: a speaker for audio feedback, vibration motor for haptic feedback and laser pointer for optic feedback. All components are controlled by a microcontroller, the Arduino Nano.
Inside each target box, which were lasercut and glued together, there is a Arduino Nano controlling a RGB-LED, a battery and an IR-Receiver.
All target boxes are linked by I^2C Communication to the Master, the Pybadge. It controls the game logic and sends and requests data to and from each target box.

## Installation/Construction Overview
Details on installation and construction are in the [manual](Manual.pdf)

To build the game yourself, you need to construct three parts:
### Guns (2x)
- Print the gun from the [3D print file](media/3Dprint/all_parts.gcode.3mf) provided.
- Connect all parts as described in the [manual](Manual.pdf)
- Connect the USB cable to the Nano
- Seal the gun with the three M3 screws

### Receiver Boards (<127x)
- Each Receiver Board can be constructed from glued together lasercut wood boards
  - [Front panel](media/lasercut/frontPanel.pdf)
  - [Side panels](media/lasercut/sidePanels.pdf)
  - [Back panel](media/lasercut/backPanel.pdf)
- These are later connected via I^2C to the central game station

### Central game station
- The case contains the Pybadge. The case is constructed with the following lasercut files:
  - [box](media/lasercut/pybox.pdf)
  - [engraving](media/lasercut/GravurPybox.pdf)
- Additional to the case, we need a seperate box to contain the LED strip which shows the score
  - [score box](media/lasercut/GravurBox.pdf)
- The following files are optional decorations to better game clarity
  - [Gamemode 1 "Standoff" sign](media/lasercut/GravurStandoff.pdf)
  - [Gamemode 2 "Tag A Mole" sign](media/lasercut/GravurTagAMole.pdf)
 
## Outlook and possible improvements
As we kept the project modular and easily expandable in gamemodes there are severals ways to extend the project scope
- We could expand the possible playing field by connecting more receiver boards (currently 5) to the I^2C Bus
- More gamemodes could be added with possible coop, multiple choice quiz etc.
- We could expand the range of the guns to more than 3m with better IR-LEDs and receivers
- We could reduce cost of construction by replacing each Nano in the receiver board with a simpler and cheaper microcontroller
- We could make the parts in the gun less prone to loose connections with cable holders and better designed 3d print models
