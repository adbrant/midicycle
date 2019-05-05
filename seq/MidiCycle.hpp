#pragma once
#include "SeqRecorder.hpp"
#include "m_pd.h"
namespace MCycle {

struct  noteOnInfo {
	char velocity;
	long long start;
	};

class MidiCycle {
  public:
    MidiCycle(int max_length,t_outlet * outlet )
    : m_seq_recorder(max_length), m_playing{false},   m_step{0}, m_step_global{0},
    m_max_length{max_length}, m_held_notes(),m_playing_notes(),m_outlet{outlet}
    {
    }
    // Incoming note on/off to the record
    void note_event(char note, char velocity);
    // PPQ ticks from MIDI clock
    void tick(int tick);
    // Loo
    void loop(int beats);
  private:
    // Emit note to PD outlet
    void output_note(char note, char velocity) {
      DEBUG_POST("note %d %d",note, velocity );
      SETFLOAT(m_output_list, note);
	    SETFLOAT(m_output_list+1, velocity);
      outlet_list(m_outlet, &s_list, 2, m_output_list);
    }
    SeqRecorder m_seq_recorder;
    bool m_playing;
    // Step position in loop
    int m_step;
    // Steps seen regardless of loop
    long long m_step_global;
    int m_max_length;
    std::multimap<char, noteOnInfo> m_held_notes; 
    std::multimap<long long, char> m_playing_notes;
    // Pd outlet handle
    t_outlet * m_outlet;
    t_atom m_output_list[2];
    int m_num_beats;
    int m_loop_start;
    int m_loop_end;
};


void MidiCycle::note_event(char note, char velocity) {
  if(note < 0 or velocity < 0) {
    post("MidiCycle: Invalid note");
    return;
  }
    
  if(velocity){
    // Note on
    noteOnInfo note_on_info = { velocity, m_step_global};
    m_held_notes.insert({note, note_on_info});
  } else {
    // Note off
    auto iter = m_held_notes.find( note);
    if(iter != m_held_notes.end()){
      const noteOnInfo & note_on_info = (*iter).second;

      short duration = (short)(m_step_global - note_on_info.start);
      
      // print warning if note is too long for short
      if( duration < 0) { 
        post("MidiCycle: Note duration truncated");
        duration = PPQ;
      }
      noteEvent note_event = {note, note_on_info.velocity, duration};
      m_seq_recorder.add_note( note_on_info.start, note_event);
      
      m_held_notes.erase( iter);
    }   
  }
};

void MidiCycle::tick(int tick)
{
  m_step = (m_step+1)%m_max_length;
  m_step_global++;
  
  // We need to realign if we are out of sync 
  // Only do at beat boundaries
  if(tick != (m_step%24)) {
    if(tick == 0) {
      m_step -= (m_step%24);
    } 
  }
  if(m_playing) {
    if (m_step == m_loop_end){
      m_step = m_loop_start;
      DEBUG_POST("loop looping");
    }
    if(not m_seq_recorder.step_empty(m_step)) {
      const timestep &tstep = m_seq_recorder.get_step(m_step);
      for(auto & note : tstep) {
        output_note(note.note, note.velocity);
        m_playing_notes.insert({m_step_global+(long long)note.duration, note.note});
      }
    }
  } else {
    m_seq_recorder.clear_step(m_step);
  }
  while(!m_playing_notes.empty() && (*m_playing_notes.begin()).first <= m_step_global){
    output_note((*m_playing_notes.begin()).second, 0);
    m_playing_notes.erase(m_playing_notes.begin());       
  }
}

void MidiCycle::loop(int beats)
{
  if(beats < 1) {
    // Stop playing and start recording again 
    m_playing = false;
    DEBUG_POST("loop ending");
  } else {
    DEBUG_POST("loop starting");
    m_playing = true;
    m_num_beats = beats;
    m_loop_end = m_step;
    m_loop_start = (m_step - (beats*PPQ)+m_max_length)%m_max_length; 
    DEBUG_POST("loop start/end %d %d",m_loop_start,m_loop_end);
    m_step = m_loop_start;
  }    
}
}   