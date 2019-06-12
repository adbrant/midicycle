#pragma once
#include "MidiCycle.hpp"
#include <cereal/access.hpp>
namespace MCycle {


void MidiCycle::note_event(note_id note, char velocity) {
  // Takes note on and off events
  // Once they are complete, write to the note recorder
  if (note < 0 or velocity < 0) {
    DEBUG_POST("MidiCycle: Invalid note");
    return;
  }

  if (velocity) {
    // Don't record new notes if we are playing
    if (m_state == mcState::EMPTY || m_overdub) {
      // Note on
      noteOnInfo note_on_info{velocity, m_step_global,m_step };
      m_held_notes.insert({note, note_on_info});
    }
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
  if (tick != (m_step % 24)) {
    return m_notes_out;
  } 
  
  if (m_state == mcState::PLAYING) {   
    local_step steps_to_play = 1;
    // Depending on quantization play 0 or more steps
    if(m_quantize > 0){
      int quantize_len = int(PPQ/m_quantize);
      if(m_step%quantize_len != 0) {
        // Don't play on this step
        steps_to_play = 0;
      } else {
        
        if(!m_quantize_changed) {
          steps_to_play = quantize_len;
        } else {
          // Initial step, just play up to next beat
          m_quantize_position = m_step;
          steps_to_play = 1+(quantize_len/2);
          m_quantize_changed = false;
        }
      }   
    } 
    
    while(steps_to_play > 0) {      
      //DEBUG_POST("Trying %d %d",m_step, m_quantize_position);
      if( (not m_seq_recorder.step_empty(m_quantize_position)) ){
        //DEBUG_POST("Playing %d %d",m_step, m_quantize_position);
        const timestep &tstep = m_seq_recorder.get_step(m_quantize_position);
        for (auto &note : tstep) {
          // Output note and schedule note off for a later global step
          m_notes_out.push_back(note);
          DEBUG_POST("Playing note %d duration %d local ts %d note_off at %d",note.note, note.duration, m_step, m_step_global + note.duration);
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
      DEBUG_POST("loop looping");
    }
  } else if (m_state == mcState::STOP){
    // Keep stepping but dont play
    m_step = (m_step + 1) % m_max_length;
    if (m_step == m_loop_end) {
      m_step = m_loop_start;
      DEBUG_POST("loop looping");
    }
  } else if (m_state == mcState::EMPTY){
    // Clear this step
    // New notes will be commited later when they complete
    m_seq_recorder.clear_step((m_step + 1) % m_max_length);
    
    m_step = (m_step + 1) % m_max_length;

  }
  while (!m_playing_notes.empty() &&
         (*m_playing_notes.begin()).first <= m_step_global) {  
    noteEvent note_off = { (*m_playing_notes.begin()).second, 0, 0};
    m_notes_out.push_back(note_off);       
    DEBUG_POST("Ending note %d global ts %d local ts %d",(*m_playing_notes.begin()).second, m_step_global, m_step);
    m_playing_notes.erase(m_playing_notes.begin());     
  }
  // One PPQ midi clock tick
  m_step_global++;
  
  return m_notes_out;
}

void MidiCycle::loop(int beats) {
  // if beats > 0 start looping, else start recording again
  if (beats < 1) {
    // Stop playing and start recording again
    m_state = mcState::EMPTY;
    DEBUG_POST("loop ending");
  } else {
    DEBUG_POST("loop starting");
    m_state = mcState::PLAYING;
    m_loop_end = m_step;
    m_loop_start = (m_step - (beats * PPQ) + m_max_length) % m_max_length;
    DEBUG_POST("loop start/end %d %d", m_loop_start, m_loop_end);
    m_step = m_loop_start;
    m_quantize_position = m_step;  
  }
}

void MidiCycle::quantize(int division) {
  // if beats > 0 start looping, else start recording again
  if (division < 0 or division > 4) {
    DEBUG_POST("MidiCyle: invalid division");
  } else {    
    m_quantize = division;
    // reset playback
    m_quantize_changed = true;
  }
}
}