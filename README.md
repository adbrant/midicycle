# midicycle
12 tracks to sequence ORAC modules, or external MIDI devices.
Can either work with a midi controller or organelle keyboard. 

Allows fast recording of loops to multiple instruments/channels
Overdubbing
Supports free timing or non-destructive quantization (¼,1/3 ½ and whole note)
Supports save/load (all recordings are saved as part of ORAC preset)
Record up to 64 beat loops, also support any number of beats for polyrhythmic sequencing
Low CPU utilization as its mostly a single C++ based external
Visual feedback on state of all tracks 

It’s an ‘always recording’ sequencer, so all notes are being recorded, and recording a loop will save the last 1-64 beats (set by a parameter).

you can play a loop multiple times and commit it to the loop when you’ve played it perfectly, or decide to loop something you play after you play it.

![alt text](https://raw.githubusercontent.com/adbrant/midicycle/master/midicycle.png)
