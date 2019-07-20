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

The first page display tells you the state of each channel and lets you record loops to any channel, as well as playing and stopping all channels.




### Legend

 - 'v' points to the currently active channel
 - 'o' = empty
 - '>' = playing
 - ''| = stopped

Main page controls:
Knob 1: length of the loop to record
Knob 2: Turns overdub on/off
Knob 3: Change the transposition of the active channel from -24 to +24
Knob 4: Change the destination of the active channel, either an ORAC modules (a1-c3) or MIDI channels (m1-m16)
By default you can use the organelle keys to play/record.
When using the Organelle keys to play, the aux button will let you, otherwise you can just press the keys by themselves.
When using an external MIDI controller to play/record (settign is on the second page) the keys 



To record a loop to channel one, press the first key to select the channel, play the phrase, then press again to commit the phrase.
It will automatically begin looping, press the 13th key, one octave up, to play or stop the loop. 
To add more notes to this loop, turn overdub on and play the notes you want to ad.
To clear the loop, hold the first key for at least two beats and release it.

### Page 2 :
 - Quantize : Sets quantization, (0 is off, 1 = whole note, 2 = 1/2 note, 3 = 1/3 note, 4 = 1/4 note)
 - Midi Src: -1 = organelle keys, 0 = omi mode, 1-16 = midi channel 1-16

### Page 3:
 - BPM
 - Midi clock out on/off
 - Metronome volume (left channel)
 - Metronome volume (right channel)


