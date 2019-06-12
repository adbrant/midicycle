#pragma once
#include "SeqRecorder.hpp"
#include <cereal/access.hpp>
namespace MCycle {
enum class mcState { EMPTY,PLAYING,STOP };
//
typedef std::pair <global_step, note_id> note_off_event;

class MidiCycle {
friend class cereal::access;
public:
  MidiCycle(int max_length=12)
      : m_seq_recorder(max_length), m_state{mcState::EMPTY}, m_step{0},
        m_step_global{0}, m_max_length{max_length}, m_held_notes(),
        m_playing_notes(), m_quantize(0), m_quantize_changed(false), m_overdub(false),m_notes_out(){}
        

  struct noteOnInfo {
    char velocity;
    global_step start_global;
    local_step start_local;
  };  
  // Incoming note on/off to the record
  void note_event(note_id note, char velocity);
  // PPQ ticks from MIDI clock, return note events
  const timestep&  tick(int tick);
  // Loop # of beats, or stop looping is arg is 0
  void loop(int beats);
  // Quantize output to division 1-4, 0 is unquantized
  void quantize(int division);
  
  // Kill remaining notes
  const timestep&  flush();
  
  mcState get_state() { return m_state; }
  void playstop() {
    if (m_state == mcState::PLAYING) {
      m_state = mcState::STOP;
    } else if (m_state == mcState::STOP ) {
      m_state = mcState::PLAYING;      
    }
  }  
  void overdub(bool overdub) {
    m_overdub=overdub;
  }
  bool overdubbing(){
    return m_overdub;
  }
  template<class Archive>
  void serialize(Archive & archive)
  {
    archive( m_seq_recorder, m_state, m_step,m_step_global,
    m_max_length,m_loop_start,m_loop_end,m_quantize_position,m_quantize,m_quantize_changed
    ); // serialize things by passing them to the archive
  }  
private:
  

  SeqRecorder m_seq_recorder;
  mcState m_state;
  // Step position in loop
  local_step m_step;
  // Steps seen regardless of loop
  global_step m_step_global;
  int m_max_length;
  std::multimap<note_id, noteOnInfo> m_held_notes;
  std::multimap<global_step, note_id> m_playing_notes;
  int m_loop_start;
  int m_loop_end;
  int m_quantize_position;
  int m_quantize;
  bool m_quantize_changed;
  bool m_overdub;
  timestep m_notes_out;
};

}