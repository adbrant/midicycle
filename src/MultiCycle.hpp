#pragma once
#include "MidiCycle.hpp"
#include <set>
#include <string>
#include <deque>
#include <cstring>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
namespace MCycle {

const char* get_dst_display(int dst);
const char* get_src_display(int src);

class NoteTracker {
  public:
  NoteTracker() : m_held_notes()
  {};
     
  void note_on(global_step step, note_id note){
    // Insert into held notes
    std::pair<char, global_step>  note_on{note, step };
    m_held_notes.insert(note_on);
  }
  // note off, return length or 0 if not found
  global_step note_off(global_step step, note_id note){

    // Note off
    global_step duration = 0;
    auto iter = m_held_notes.find(note);
    if (iter != m_held_notes.end()) {
      const auto &start_step = (*iter).second;
      duration = step - start_step;
      m_held_notes.erase(iter);
    }
    return duration;
  }
  void clear() {
    m_held_notes.clear();
  }
  private:
    std::multimap<note_id, global_step> m_held_notes;
  
};


class MultiCycle {
friend class cereal::access;
public:
  MultiCycle(int max_length, int num_channels)
      :  m_auxval(0),m_max_length{max_length},m_num_channels{num_channels}, m_active_channel{0},m_loop_length{1},  m_midi_src(0), m_midicycles(),  m_playing_notes(),m_status(), m_status_empty(), m_notes_out(),m_step_global(0), m_key_tracker(),m_mainpage(false)
      { 
        // Create n midi loopers        
        for(int i = 0 ; i < num_channels; i ++){
          m_midicycles.emplace_back(max_length);
          m_channel_dests.push_back(1);
        }
        m_status.push_back(std::string("screenLine3"));
        m_status.push_back(std::string());
        m_status.push_back(std::string("screenLine4"));
        m_status.push_back(std::string());
        m_status.push_back(std::string("screenLine5"));
        m_status.push_back(std::string());
        m_status_empty.push_back(std::string("screenLine5"));
        m_status_empty.push_back(std::string());
      }
  // Incoming note on/off from keyboard (used for control)
  void key_event(note_id note, char velocity);
  
  // Incoming note on/off from MIDI
  void note_event(int channel, note_id note, char velocity);
  
  // PPQ ticks from MIDI clock, return note events with channels
  void  tick(int tick);
  
  // set dest for channel, return any note-offs
  void  set_dest(int channel, int dest);
 
 // set dest for channel, return any note-offs
  void  set_src(int channel);
  
  // Loop # of beats, or stop looping is arg is 0
  void loop(int mc_id, int beats){
    m_midicycles[mc_id].loop(beats);
  }
  // Quantize output to division 1-4, 0 is unquantized
  void quantize(int division){
    for( auto& mc : m_midicycles) {
      mc.quantize(division);
    }
  }
  
  void overdub(int mc_id, bool overdub) {
    if(mc_id == -1) { 
      for ( auto & mc : m_midicycles){
        mc.overdub(overdub);
      }
    } else {
      m_midicycles[mc_id].overdub(overdub);
    }
  }
  
  void aux(int auxval){
    if (m_midi_src == -1 ) {
      if(auxval == 1 and m_auxval == 0){
        flush_playing();
      }
    }
    // Clear outstanding key presses
    if(m_auxval != auxval) m_key_tracker.clear();
    m_auxval = auxval;
  }
  void set_loop_length(int length){
    m_loop_length = length;
  }
  
  void set_active(int mc_id);
  
  std::vector<std::string>& get_status();
  
  template<class Archive>
  void serialize(Archive & archive)
  {

    archive( m_max_length,m_num_channels,m_loop_length, m_midicycles,m_channel_dests, m_active_channel,m_midi_src); // serialize things by passing them to the archive
  }   
  
  bool notesready() {
    return !m_notes_out.empty();
  }
  std::pair<int, noteEvent> popnote(){
    assert(notesready());
    std::pair<int, noteEvent> retval = m_notes_out.front();
    m_notes_out.pop_front();
    return retval;
  }
  
  void activepage(const char* name){
    m_mainpage = ( std::strcmp(name,"pg_main") == 0);
  }
  
private:
  int m_auxval;
  int m_max_length;
  int m_num_channels;
  int m_active_channel;
  int m_loop_length;
  int m_midi_src;
  std::vector<MidiCycle> m_midicycles;
  std::vector<int> m_channel_dests;
  std::multiset<note_id> m_playing_notes;
  std::vector<std::string> m_status;
  std::vector<std::string> m_status_empty;
  std::deque<std::pair<int, noteEvent>> m_notes_out;
  global_step m_step_global;
  NoteTracker m_key_tracker;
  bool m_mainpage;
  void flush_playing(){
    for( const note_id &pnote : m_playing_notes){
      m_midicycles[m_active_channel].note_event(pnote,0);
      noteEvent ne = {pnote, 0 , 0};
      m_notes_out.push_back({m_channel_dests[m_active_channel],ne});
    }   
    m_playing_notes.clear();  
  }
};
}