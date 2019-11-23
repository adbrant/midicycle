#pragma once
#include "MidiCycle.hpp"
#include "KnobTracker.hpp"
#include "NoteTracker.hpp"
#include <set>
#include <math.h>
#include <string>
#include <deque>
#include <cstring>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>


namespace MCycle {

const char* get_dst_display(int dst);
const char* get_src_display(int src);


class MultiCycle {
friend class cereal::access;
public:
  MultiCycle(int max_length, int num_channels)
      :  m_quantize(0), m_auxval(0),m_max_length{max_length},m_num_channels{num_channels}, m_active_channel{0},m_loop_length{1},  m_midi_src(0),m_loop_quant(num_channels, -1),m_step_global(0), m_mainpage(false),m_active_module(false)
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
  
  // set transpose for channel
  void  set_transpose(int channel, int transpose){
     m_midicycles[channel].set_transpose(transpose);
  }
  
  // Loop # of beats, or stop looping is arg is 0
  void loop(int mc_id, int beats){
    m_midicycles[mc_id].loop(beats);
  }
  // Quantize output to division 1-4, 0 is unquantized
  void quantize(int division){
    m_quantize = division;
    for( int i = 0; i < m_num_channels; i++ ) {
      if(m_loop_quant[i] == -1 ) {
        m_midicycles[i].quantize(division);
      }
    }
  }
  
  // Set recording mode
  void set_arm_mode(bool mode){
    for( int i = 0; i < m_num_channels; i++ ) {
      m_midicycles[i].set_arm_mode(mode);
    }
  }  
  
  // Quantize only this loop, -1 is use global setting
  void quantize_loop(int loop, int division) { 
    m_loop_quant[loop] = division;
    if(division != -1){
      m_midicycles[loop].quantize(division);      
    } else{
      m_midicycles[loop].quantize(m_quantize);
    } 
  }     
  
  // Incoming notes will overdub into active sequence
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
  
  void footswitch(int fsval){
    if( fsval == 1 && m_midicycles[m_active_channel].get_state() == mcState::EMPTY ) {
      m_midicycles[m_active_channel].loop(m_loop_length);
      DEBUG_POST("loop channel %d len %d",m_active_channel,m_loop_length);
    }  
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
  
  void active_page(const char* name){
    m_mainpage = ( std::strcmp(name,"pg_main") == 0);
  }
  
  bool mainpage() { return m_mainpage;}
  void set_module_id(const char* id) {
    m_module_id = id;
  }
  void set_active_module(const char* id) {
    m_active_module = (m_module_id == id);
  }
  bool is_main_page(){
    return m_active_module && m_mainpage;
  }
  void set_knob(int knob_id, float val){
    if(knob_id == 3){
      if(m_knob_tracker.update(knob_id-1,val) >= 0 ) { 
        m_src_msg = { "setparam", m_module_id,  "mc_trans_"+std::to_string(m_active_channel), std::to_string(val) }; 
      }      
    } else if (knob_id == 4){
      if(m_knob_tracker.update(knob_id-1,val) >= 0  ) { 
        m_dst_msg = { "setparam", m_module_id,  "mc_chan_"+std::to_string(m_active_channel), std::to_string(val) }; 
      }      
    }
  }
  bool get_pd_message(std::vector<std::string> & msg ){
    if(m_dst_msg.size() > 0){
      msg = std::move(m_dst_msg);
      m_dst_msg.clear();
      return true;      
    } else if(m_src_msg.size() > 0) {
      msg = std::move(m_src_msg);
      m_src_msg.clear();
      return true;      
  }      
    else {
      return false;
    }
  }
  
private:
  int m_auxval;
  int m_max_length;
  int m_num_channels;
  int m_active_channel;
  int m_loop_length;
  int m_midi_src;
  int m_quantize;
  std::vector<MidiCycle> m_midicycles;
  std::vector<int> m_channel_dests;
  NoteTracker m_playing_notes;
  std::vector<std::string> m_status;
  std::vector<std::string> m_status_empty;
  std::vector<std::string> m_src_msg;
  std::vector<std::string> m_dst_msg;
  std::vector<int> m_loop_quant;
  std::deque<std::pair<int, noteEvent>> m_notes_out;
  global_step m_step_global;
  NoteTracker m_key_tracker;
  KnobTracker m_knob_tracker;
  bool m_mainpage;
  bool m_active_module;
  std::string m_module_id;
  void flush_playing(){
    std::vector<note_id> note_offs;
    m_playing_notes.clear(note_offs);
    for( const note_id &pnote : note_offs){
      m_midicycles[m_active_channel].note_event(pnote,0);
      noteEvent ne = {pnote, 0 , 0};
      m_notes_out.push_back({m_channel_dests[m_active_channel],ne});
    }   
    m_playing_notes.clear();  
  }
};
}