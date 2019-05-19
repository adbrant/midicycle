#pragma once
#include "SeqRecorder.hpp"

namespace MCycle {

class MidiCycle {
public:
  MidiCycle(int max_length, t_outlet *outlet)
      : m_seq_recorder(max_length), m_state{mcState::EMPTY}, m_step{0},
        m_step_global{0}, m_max_length{max_length}, m_held_notes(),
        m_playing_notes(), m_outlet{outlet}, m_quantize(0), m_quantize_change(false) {}
  // Incoming note on/off to the record
  void note_event(note_id note, char velocity);
  // PPQ ticks from MIDI clock
  void tick(int tick);
  // Loop # of beats, or stop looping is arg is 0
  void loop(int beats);
  // Quantize output to division 1-4, 0 is unquantized
  void quantize(int division);
  

private:
  // Emit note to PD outlet
  void output_note(note_id note, char velocity) {
    DEBUG_POST("note %d %d", note, velocity);
    SETFLOAT(m_output_list, note);
    SETFLOAT(m_output_list + 1, velocity);
    outlet_list(m_outlet, &s_list, 2, m_output_list);
  }
  
  enum class mcState { EMPTY,PLAYING,STOP,OVERDUB };
  
  SeqRecorder m_seq_recorder;
  mcState m_state;
  // Step position in loop
  local_step m_step;
  // Steps seen regardless of loop
  global_step m_step_global;
  int m_max_length;
  std::multimap<note_id, noteOnInfo> m_held_notes;
  std::multimap<global_step, note_id> m_playing_notes;
  // Pd outlet handle
  t_outlet *m_outlet;
  t_atom m_output_list[2];
  int m_num_beats;
  int m_loop_start;
  int m_loop_end;
  int m_loop_len;
  int m_quantize_position;
  int m_quantize;
  bool m_quantize_change;
};

void MidiCycle::note_event(note_id note, char velocity) {
  // Takes note on and off events
  // Once they are complete, write to the note recorder
  if (note < 0 or velocity < 0) {
    post("MidiCycle: Invalid note");
    return;
  }

  if (velocity) {
    // Don't record new notes if we are playing
    if (m_state == mcState::EMPTY || m_state == mcState::OVERDUB) {
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
        post("MidiCycle: Note duration truncated");
        duration = PPQ;
      }
      DEBUG_POST("Adding note %d duration %d local timestep %d",note, duration, note_on_info.start_local);
      noteEvent note_event = {note, note_on_info.velocity, duration};
      m_seq_recorder.add_note(note_on_info.start_local, note_event);

      m_held_notes.erase(iter);
    }
  }
};
/*
// local_step inside the current loop bounds
constexpr local_step legalize_step( const local_step instep, const local_step loop_start, const local_step loop_size,  const local_step array_size  )  {
  local_step legal_step = instep%array_size;
  bool wrapped_end = (loop_start+loop_size) > loop_end;
  //re-baseline at loop_start
  if(wrapped_end ){
    if ( instep < loop_start  and instep > loop_end) {
      // before loop start or after loop end

    } else { // in loop region
      // legalize into array
      legal_step = instep%array_size;      
    }
    
    local_step offset = instep - loop_start;
    
  } else {
    local_step offset = instep - loop_start;
  }
  
  return legal_step;
}
// Test various legalizations
static_assert(legalize_step(0,0,PPQ,PPQ) == 0 );
static_assert(legalize_step(PPQ,0,PPQ,PPQ) == 0 );
// Loop wraps around end
static_assert(legalize_step(PPQ,15*PPQ,2*PPQ,16*PPQ) == 15*PPQ );
static_assert(legalize_step(15*PPQ-1,15*PPQ,2*PPQ,16*PPQ) == PPQ-1 );
*/

void MidiCycle::tick(int tick) {

  // If we are playing, emit any note ons
  // If we are recording, clear this step
  // Emit any note offs we have scheduled
  
  // We need to realign if we are out of sync
  // Only do at beat boundaries
  if (tick != (m_step % 24)) {
    if (tick == 0) {
      m_step -= (m_step % 24);
    }
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
        
        if(!m_quantize_change) {
          steps_to_play = quantize_len;
        } else {
          // Initial step, just play up to next beat
          m_quantize_position = m_step;
          steps_to_play = 1+(quantize_len/2);
          m_quantize_change = false;
        }
      }   
    } 
    
    while(steps_to_play > 0) {
      
      DEBUG_POST("Trying %d %d",m_step, m_quantize_position);
      if( (not m_seq_recorder.step_empty(m_quantize_position)) ){
        DEBUG_POST("Playing %d %d",m_step, m_quantize_position);
        const timestep &tstep = m_seq_recorder.get_step(m_quantize_position);
        for (auto &note : tstep) {
          // Output note and schedule note off for a later global step
          output_note(note.note, note.velocity);
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
  } else {
    // Clear this step
    // New notes will be commited later when they complete
    m_seq_recorder.clear_step((m_step + 1) % m_max_length);
    
    m_step = (m_step + 1) % m_max_length;
  }
  while (!m_playing_notes.empty() &&
         (*m_playing_notes.begin()).first <= m_step_global) {    
    output_note((*m_playing_notes.begin()).second, 0);    
    DEBUG_POST("Ending note %d global ts %d local ts %d",(*m_playing_notes.begin()).second, m_step_global, m_step);
    m_playing_notes.erase(m_playing_notes.begin());   
    
  }
  
  
  // One PPQ midi clock tick
  m_step_global++;
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
    m_num_beats = beats;
    m_loop_len = (beats * PPQ);
    m_loop_end = m_step;
    m_loop_start = (m_step - (m_loop_len) + m_max_length) % m_max_length;
    DEBUG_POST("loop start/end %d %d", m_loop_start, m_loop_end);
    m_step = m_loop_start;
    m_quantize_position = m_step;  
  }
}

void MidiCycle::quantize(int division) {
  // if beats > 0 start looping, else start recording again
  if (division < 0 or division > 4) {
    post("MidiCyle: invalid division");
  } else {    
    m_quantize = division;
    // reset playback
    m_quantize_position = m_step;
    m_quantize_change = true;
  }
}
}