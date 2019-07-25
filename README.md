# midicycle
12 tracks to sequence ORAC modules, or external MIDI devices.
Can either work with a midi controller or organelle keyboard. 

## Features:
 - Supports free timing or non-destructive quantization (¼,1/3 ½ and whole note)
 - Supports save/load (all recordings are saved as part of ORAC preset)
 - Overdubbing
 - Record up to 64 beat loops, also support any number of beats for polyrhythmic sequencing
 - Low CPU utilization as its mostly a single C++ based external
 - Visual feedback on state of all tracks 
 - LED or audio metronome 

It’s an ‘always recording’ sequencer, so all notes are being recorded, and recording a loop will save the last 1-64 beats (set by a parameter). This lets you can play a loop multiple times and commit it to the loop when you’ve played it perfectly, or decide to loop something you play after you play it.

## Usage:
**MUST BE LOADED IN SLOT S2 (the lowest Eb), replacing the 'clock' module**

![alt text](https://raw.githubusercontent.com/adbrant/midicycle/master/midicycle.png)

The first page display tells you the state of each channel and lets you record loops to any channel, as well as playing and stopping all channels. 
### Legend
Each channel is represented by a single character. The channel under the 'v' is the current active channel.
 - 'v' points to the currently active channel
 - 'o' = empty
 - '>' = playing
 - '|' = stopped

###  Main page controls:
 - Knob 1: length of the loop to record
 - Knob 2: Turns overdub on/off. When on and the channel is not empty, all notes are immediately recoreded into the loop.
 - Knob 3: Change the transposition of the active channel from -24 to +24
 - Knob 4: Change the destination of the active channel, either an ORAC modules (a1-c3) or MIDI channels (m1-m16)
 
Knobs 3 and 4 change based on the current active channel.
By default you can use the organelle keys to play/record.
When using the Organelle keys to play, the Aux button needs to be pressed to use the key controls.  
When using an external MIDI controller to play/record (setting is on the second page) the keys controls can be used without pressing Aux. 

Example:
To record a loop to channel two, press the first key to select the channel, play the phrase, then press again to commit the phrase.
It will automatically begin looping, press the 13th key, one octave up, to play or stop the loop. 
To add more notes to this loop, turn overdub on and play the notes you want to add.
To clear the loop, hold the first key for at least two beats and release it.

### Page 2:
 - Quantize : Sets quantization, (0 is off, 1 = whole note, 2 = 1/2 note, 3 = 1/3 note, 4 = 1/4 note)
 - Midi Src: -1 = organelle keys, 0 = omi mode, 1-16 = midi channel 1-16

### Page 3:
 - BPM
 - Midi clock out on/off
 - Metronome volume (left channel)
 - Metronome volume (right channel)


