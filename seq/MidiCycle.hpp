#pragma once
#include "SeqRecorder.hpp"
namespace MCycle {
class MidiCycle {
  public:
    MidiCycle(int max_length)
    : m_held_notes(), m_playing{false}, m_seq_recorder(max_length), m_step{0}, m_max_length{max_length}
    {
    }
    // Incoming note on/off to the record
    void note_event(char note, char velocity) {
      assert(note >= 0 and velocity >= 0);
        
      if(velocity){
        // Note on
        noteOnInfo note_on_info = { velocity, m_step};
        m_held_notes.insert({note, note_on_info});
      } else {
        // Note off
        auto iter = m_held_notes.find( note);
        if(iter != m_held_notes.end()){
          const noteOnInfo & note_on_info = (*iter).second;
          // need to handle looping
          short duration = (short)(m_step - note_on_info.start);

          noteEvent note_event = {note, note_on_info.velocity, duration};
          m_seq_recorder.add_note( note_on_info.start, note_event);
          
          m_held_notes.erase( iter);
        }   
      }
    };
    void step(int step)
    {
      m_step = (m_step+1)%m_max_length;
      if(m_playing) {

      } else {
        m_seq_recorder.clear_step(m_step);
      }    
    }

  private:
    SeqRecorder m_seq_recorder;
    bool m_playing;
    int m_step;
    int m_max_length;
    std::multimap<char, noteOnInfo> m_held_notes; 
};
}   