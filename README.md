## "TrainingsProgramm 42" - A Lasertag target shooting game -
<img width="1280" height="640" alt="Thumbnail" src="https://github.com/user-attachments/assets/5c5d9131-902d-4d7c-a1f8-6ec16b052e0b" />

## Trailer
[Watch the demo](https://github.com/VMHengo/TrainingsProgramm-42/media/presentation/Trailer.mp4)

## Description
This was a university project about screenless games. We decided to build a fast paced reactive multiplayer game about shooting rapid blinking targets with lasertag pistols.

## Project Realization
Inside the 3D printed gun, we used an Infrared-LED to send signals which are received by each target individually by an IR-receiver. We expanded on this basic idea with focus on user experience by including inside the gun: a speaker for audio feedback, vibration motor for haptic feedback and laser pointer for optic feedback. All components are controlled by a microcontroller, the Arduino Nano.
Inside each target box, which were lasercut and glued together, there is a Arduino Nano controlling a RGB-LED, a battery and an IR-Receiver.
All target boxes are linked by I^2C Communication to the Master, the Pybadge. It controls the game logic and sends and requests data to and from each target box.

## Visuals
Depending on what you are making, it can be a good idea to include screenshots or even a video (you'll frequently see GIFs rather than actual videos). Tools like ttygif can help, but check out Asciinema for a more sophisticated method.
