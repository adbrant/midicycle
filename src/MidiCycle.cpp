#include "MidiCycle.hpp"
namespace MCycle {
void calculate_loop_bounds(int current_step, int idle_steps, int loop_length, int buffer_size, int& loop_start, int& loop_end  ){
  loop_end = current_step - idle_steps;
  if( loop_end < 0) {
    loop_end += buffer_size;
  }
  
  loop_start = (loop_end - (loop_length * PPQ));
      
  if( loop_start < 0) {
    loop_start += buffer_size;
  }
}


bool play_on_step_quantized( int tick, int quantize_len, int swing8, int swing16) {
  if (quantize_len == (PPQ/4)) {
    // 16th notes
    return (tick == 0) || (tick == ((PPQ/4)+swing16)) || (tick == (PPQ/2+swing8)) || (tick == ((3*PPQ/4)+swing16));
  } else if (quantize_len == PPQ/2) {
    // 8th notes
    return (tick == 0) || (tick == (PPQ/2+swing8));
  }  else  {
    // no swing added to triplets
    return (tick%quantize_len) == 0;
  }  
}

void MidiCycle::note_event(note_id note, char velocity) {
  // Takes note on and off events
  // Once they are complete, write to the note recorder
  if (note < 0 or velocity < 0) {
    DEBUG_POST("MidiCycle: Invalid note");
    return;
  }

  if (velocity) {
    if(m_state == mcState::ARMED) {
      // Start recording now
      DEBUG_POST("MidiCycle: recording");
      m_state = mcState::RECORDING; 
      // -3 buffer to prevent repeated notes
      m_arm_remaining_steps = m_looplen * PPQ-3;
    }
    // Don't record new notes if we are playing
    if (m_state == mcState::EMPTY  || m_state == mcState::RECORDING  || m_overdub) {
      // Note on
      noteOnInfo note_on_info{velocity, m_step_global,m_step };
      m_held_notes.insert({note, note_on_info});
    }
    m_idle = false;
  } else {
    // Note off
    auto iter = m_held_notes.find(note);
    if (iter != m_held_notes.end()) {
      const auto &note_on_info = (*iter).second;

      auto duration = static_cast<short>(m_step_global - note_on_info.start_global);

      // print warning if note is too long for short
      if (duration < 0) {
        DEBUG_POST("MidiCycle: Note duration truncated");
        duration = PPQ;
      }
      DEBUG_POST("Adding note %d duration %d local timestep %d",note, duration, note_on_info.start_local);
      noteEvent note_event = {note, note_on_info.velocity, duration};
      m_seq_recorder.add_note(note_on_info.start_local, note_event);

      m_held_notes.erase(iter);
    }
  }
};
const timestep& MidiCycle::flush() {
   // Clear any previous notes
  m_notes_out.clear();
  for  (auto & notepair : m_playing_notes) {  
    noteEvent note_off = {notepair.second , 0, 0};
    m_notes_out.push_back(note_off);       
    DEBUG_POST("Flushing note %d global ts %d local ts %d",(*m_playing_notes.begin()).second, m_step_global, m_step);
   
  }
  m_playing_notes.clear();    
  return m_notes_out;
}
const timestep& MidiCycle::tick(int tick) {

  // If we are playing, emit any note ons
  // If we are recording, clear this step
  // Emit any note offs we have scheduled
  
  // Clear any previous notes
  m_notes_out.clear();
  
  // We need to realign if we are out of sync
  // do nothing until we are aligned
  if (tick != (m_step % PPQ)) {
    DEBUG_POST("Out of sync %d %d %d",tick,m_step%PPQ,m_step);
    return m_notes_out;
  } 
  if (m_state == mcState::RECORDING) {   
      if(m_arm_remaining_steps-- == 0){
        commit_loop();   
      }
  }
  if (m_state == mcState::PLAYING) {   
    local_step steps_to_play = 1;
    // Depending on quantization play 0 or more steps
    if(m_quantize > 0){
      int quantize_len = int(PPQ/m_quantize);
      if(!play_on_step_quantized( tick, quantize_len, m_swing_8,  m_swing_16) ) {  
        // Don't play on this step
        steps_to_play = 0;
      } else {
        
        if((tick == 0) && m_quantize_reset) {
          // Initial step, just play up to next beat
          m_quantize_position = m_step;
          steps_to_play = 1+(quantize_len/2);
          m_quantize_reset = false;          
        } else {
          steps_to_play = quantize_len;
        }
      }   
    } else {
      m_quantize_position = m_step;
    }
    
    while(steps_to_play > 0) {      
      //DEBUG_POST("Trying %d %d",m_step, m_quantize_position);
      if( (not m_seq_recorder.step_empty(m_quantize_position)) ){
        //DEBUG_POST("Playing %d %d",m_step, m_quantize_position);
        const timestep &tstep = m_seq_recorder.get_step(m_quantize_position);
        for (auto note : tstep) {
          // Output note and schedule note off for a later global step
          // Apply transpose
          note.note += m_transpose;
          m_notes_out.push_back(note);
          //DEBUG_POST("Playing note %d duration %d local ts %d note_off at %d",note.note, note.duration, m_step, m_step_global + note.duration);
          m_playing_notes.insert( note_off_event(m_step_global + note.duration, note.note));
        }
      }
      m_quantize_position = (m_quantize_position + 1) % m_max_length;     
      if (m_quantize_position == m_loop_end) {
        m_quantize_position = m_loop_start;
      }
      steps_to_play--;
    }    
    m_step = (m_step + 1) % m_max_length;
    if (m_step == m_loop_end) {
      m_step = m_loop_start;
      //DEBUG_POST("loop looping");
    }
  } else if (m_state == mcState::STOP){
    // Keep stepping but dont play
    m_step = (m_step + 1) % m_max_length;
    if (m_step == m_loop_end) {
      m_step = m_loop_start;
      //DEBUG_POST("loop looping");
    }
  } else {
    // Empty or armed/recording;
    // Clear this step
    // New notes will be committed later when they complete
    m_seq_recorder.clear_step((m_step + 1) % m_max_length);   
    m_step = (m_step + 1) % m_max_length;
  } 
  while (!m_playing_notes.empty() &&
         (*m_playing_notes.begin()).first <= m_step_global) {  
    noteEvent note_off = { (*m_playing_notes.begin()).second, 0, 0};
    m_notes_out.push_back(note_off);       
    //DEBUG_POST("Ending note %d global ts %d local ts %d",(*m_playing_notes.begin()).second, m_step_global, m_step);
    m_playing_notes.erase(m_playing_notes.begin());     
  }
  // One PPQ midi clock tick
  m_step_global++;
  if(m_idle) {
    m_idle_steps++;
  } else {
    m_idle_steps = 0;
  }
  m_idle = true;
  return m_notes_out;
}

void MidiCycle::loop(int beats) {
  // if beats > 0 start looping, else start recording again
  m_looplen = beats;
  if (beats < 1) {
    // Stop playing and start recording again
    m_state = mcState::EMPTY;
    DEBUG_POST("loop ending");
  } else  if (!m_arm_mode) {
    commit_loop();
  } else {
    DEBUG_POST("Armed");
    m_state = mcState::ARMED;
  }
}

void MidiCycle::commit_loop() {
  DEBUG_POST("loop starting");
  m_state = mcState::PLAYING;
  
  int idle_steps = m_idle_steps > PPQ ? PPQ: m_idle_steps;
  
  
  calculate_loop_bounds(m_step,idle_steps,m_looplen,m_max_length, m_loop_start, m_loop_end);
  
  DEBUG_POST("loop start/end %d %d idle %d mstep %d %d", m_loop_start, m_loop_end,idle_steps,m_step,m_loop_start + idle_steps );
  m_step =  m_loop_start + idle_steps;
   
  m_quantize_position = m_step;  
  // reset playback
  m_quantize_reset = true;
  
}

void MidiCycle::quantize(int division) {
  // if beats > 0 start looping, else start recording again
  if (division < 0 or division > 4) {
    DEBUG_POST("MidiCyle: invalid division");
  } else {    
    m_quantize = division;
    // reset playback
    m_quantize_reset = true;
  }
}

void MidiCycle::legalize() {
  if(m_step < 0 || m_loop_start < 0 || m_loop_end < 0
  || m_step >= m_max_length || m_loop_start >= m_max_length || m_loop_end >= m_max_length ){
    m_step = 0;
    m_loop_start = 0;
    m_loop_end = 0;
    m_state = mcState::EMPTY;
    DEBUG_POST("Illegal values detected, resetting midicycle");
  }
}
}