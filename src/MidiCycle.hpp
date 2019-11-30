#pragma once
#include "SeqRecorder.hpp"
#include <set>
#include <string>
#include <deque>
#include <cstring>
#include <cereal/access.hpp>
namespace MCycle {
enum class mcState { EMPTY,PLAYING,STOP,ARMED,RECORDING };
//
typedef std::pair <global_step, note_id> note_off_event;

class MidiCycle {
friend class cereal::access;
public:
  MidiCycle(int max_length=12)
      : m_seq_recorder(max_length), m_state{mcState::EMPTY}, m_state_last_displayed{mcState::EMPTY}, m_step{0},
        m_step_global{0}, m_max_length{max_length}, m_held_notes(),
        m_playing_notes(), m_quantize(0), m_quantize_reset(false), m_overdub(false),m_notes_out(),m_idle_steps(0),m_idle(true),m_transpose(0),m_arm_mode(false){
          assert(max_length < (1 << 16));
        }
        

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
      m_quantize_reset = true;
      m_state = mcState::PLAYING;      
    }
  }  
  void overdub(bool overdub) {
    m_overdub=overdub;
  }
  bool overdubbing(){
    return m_overdub;
  }
  void set_arm_mode (bool val){
    m_arm_mode=val;
  }
  bool get_arm_mode (){
    return m_arm_mode;
  }
  int looplen(){
    return m_looplen;
  }
  template<class Archive>
  void serialize(Archive & archive)
  {
    archive( m_seq_recorder, m_state, m_step,m_step_global,
    m_max_length,m_loop_start,m_loop_end,m_quantize_position,m_quantize,m_quantize_reset
    ); // serialize things by passing them to the archive
  }  
  void set_transpose(int transpose){
    m_transpose = transpose;
  }
  int get_transpose(){
    return m_transpose;
  }  
  void display_updated() {
    m_state_last_displayed = m_state;
  }
  bool state_changed (){
    return  m_state_last_displayed != m_state;
  }
  // Legalize values post loading
  void legalize();
private:
  
  void commit_loop();
  SeqRecorder m_seq_recorder;
  mcState m_state;
  mcState m_state_last_displayed;
  // Step position in loop
  local_step m_step;
  // Steps seen regardless of loop
  global_step m_step_global;
  int m_max_length;
  int m_looplen;
  std::multimap<note_id, noteOnInfo> m_held_notes;
  std::multimap<global_step, note_id> m_playing_notes;
  int m_loop_start;
  int m_loop_end;
  int m_quantize_position;
  int m_quantize;
  bool m_quantize_reset;
  bool m_overdub;
  int m_idle_steps;
  bool m_idle;
  timestep m_notes_out;
  int m_transpose;
  bool m_arm_mode;
  int m_arm_remaining_steps;
};
void calculate_loop_bounds(int current_step, int idle_steps, int loop_length, int buffer_size, int& loop_start, int& loop_end  );
}