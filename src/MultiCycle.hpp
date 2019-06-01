#pragma once
#include "MidiCycle.hpp"
#include <set>
#include <string>
#include <deque>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
namespace MCycle {

typedef  std::vector < std::pair<chan_id, noteEvent>> mc_timestep;


class MultiCycle {
friend class cereal::access;
public:
  MultiCycle(int max_length, int num_channels)
      :  m_auxval(0),m_max_length{max_length},m_num_channels{num_channels}, m_active_channel{0},m_loop_length{1},  m_midi_src(0), m_midicycles(),  m_playing_notes(), m_notes_out()
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
  std::deque<std::pair<int, noteEvent>> m_notes_out;
  void flush_playing(){
    for( const note_id &pnote : m_playing_notes){
      m_midicycles[m_active_channel].note_event(pnote,0);
      noteEvent ne = {pnote, 0 , 0};
      m_notes_out.push_back({m_channel_dests[m_active_channel],ne});
    }   
    m_playing_notes.clear();  
  }
};


std::vector<std::string>& MultiCycle::get_status() {
  
  m_status[1].clear();
  m_status[3].clear();
  m_status[5].clear();
  for(int i = 0 ; i < m_num_channels; i ++) {
    
    auto & mc = m_midicycles[i];
    mcState state  = mc.get_state();
    bool active = i == m_active_channel;

    m_status[1]+= active ? "v" : " ";
    bool blackkey = i == 1 or i == 3 or i == 6 or i == 8 or i == 10;
    int statusline = blackkey ? 3 : 5;
    int emptyline = blackkey ? 5 : 3;
    if(state == mcState::EMPTY){
      m_status[statusline] +=  "o";
    } else if (state == mcState::PLAYING){
      m_status[statusline]+=   ">";
      
    } else if (state == mcState::STOP){
      m_status[statusline]+=   "|";
      
    }
    m_status[emptyline] += " ";
    /*     
    o : empty
    > : Playing
    | : stopped
    A : active playing
    0 : active empty
    a : active Stopped
    */
    
  }
  return m_status;
}


// Send note to active channel, also to outlet
void MultiCycle::note_event(int channel, note_id note, char velocity) {
  // send to active mc
  m_midicycles[m_active_channel].note_event(note,velocity );
  
  // src == 0 is omni
  if(m_midi_src == 0  || channel == m_midi_src) {
    noteEvent ne = {note, velocity, 0};
    if(velocity){
      // record note so we can flush if needed
      m_playing_notes.insert(note);
    } else {
      m_playing_notes.erase(note);
    }
    m_notes_out.push_back({m_channel_dests[m_active_channel],ne});
  }
  return;
};

// Loop or play 
void MultiCycle::key_event(note_id note, char velocity) {

  if(m_midi_src == -1 && m_auxval  == 0){
    
    note_event(-1, note, velocity);
  } else if(velocity > 0){
    if(note >= 60 && note < 72){
      int mc_id = note-60;
      if(mc_id == m_active_channel) {
        // loop, or if looping change overdub
        if( m_midicycles[m_active_channel].get_state() == mcState::EMPTY ) {
          m_midicycles[m_active_channel].loop(m_loop_length);
          DEBUG_POST("loop channel %d len %d",mc_id,m_loop_length);
        }
        
      } else {      
        // Flush channel held notes and switch 
        flush_playing();
        DEBUG_POST("active channel %d ",mc_id);
        m_active_channel = mc_id;
      }
      
    } else if (note >= 72 && note <84)  {
      int mc_id = note-72;
      // Play/Stop
      DEBUG_POST("play/stop channel %d",mc_id);
      m_midicycles[mc_id].playstop();
    } 
  }

  return;
};


void MultiCycle::set_src(int channel) {


  flush_playing();
  DEBUG_POST("src %d",channel);
  m_midi_src = channel;
  return;
}

void MultiCycle::set_dest(int channel, int dest) {

  if(m_channel_dests[channel] != dest) {
    //Flush  previous dest
    DEBUG_POST("channel %d  dest %d flushing",channel,dest);
    auto & notes = m_midicycles[channel].flush();
    for( auto & note : notes){
      m_notes_out.push_back({m_channel_dests[channel],note});
    }
    m_channel_dests[channel] = dest;
  }
  if( m_active_channel == channel){
    flush_playing();
  }
  return;
}

void MultiCycle::tick(int tick) {
  
  // Get active notes from all mcs
  for(int mc_id = 0; mc_id <m_num_channels ; mc_id++){
    auto & notes = m_midicycles[mc_id].tick(tick);
    for( auto & note : notes){
      m_notes_out.push_back({m_channel_dests[mc_id],note});
    }
  }
  
  return;
}
}