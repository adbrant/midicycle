#pragma once
#include "MidiCycle.hpp"
#include <set>
namespace MCycle {

typedef  std::vector < std::pair<chan_id, noteEvent>> mc_timestep;


class MultiCycle {
public:
  MultiCycle(int max_length, int num_channels)
      :  m_auxval(0),m_max_length{max_length}, m_num_channels{num_channels}, m_active_channel{0},m_loop_length{1}, m_midicycles(),  m_playing_notes(), m_notes_out()
      { 
        // Create n midi loopers        
        for(int i = 0 ; i < num_channels; i ++){
          m_midicycles.emplace_back(max_length);
          m_channels.push_back(0);
        }
      }
  // Incoming note on/off from keyboard (used for control)
  mc_timestep& key_event(note_id note, char velocity);
  
  // Incoming note on/off from MIDI
  mc_timestep& note_event(note_id note, char velocity);
  
  // PPQ ticks from MIDI clock, return note events with channels
  mc_timestep&  tick(int tick);
  // Loop # of beats, or stop looping is arg is 0
  void loop(int mc_id, int beats){
    m_midicycles[mc_id].loop(beats);
  }
  // Quantize output to division 1-4, 0 is unquantized
  void quantize(int mc_id, int division){
    m_midicycles[mc_id].quantize(division);
  }
  
  void overdub(int mc_id, bool overdub) {
    m_midicycles[mc_id].overdub(overdub);
  }
  
  void aux(int auxval){
    m_auxval = auxval;
  }
  void set_loop_length(int length){
    m_loop_length = length;
  }
  
  void set_active(int mc_id);
private:
  int m_auxval;
  int m_max_length;
  int m_num_channels;
  int m_active_channel;
  int m_loop_length;
  std::vector<MidiCycle> m_midicycles;
  std::vector<int> m_channels;
  std::multiset<note_id> m_playing_notes;
  mc_timestep m_notes_out;
};

// Send note to active channel, also to outlet
mc_timestep& MultiCycle::note_event(note_id note, char velocity) {
  // Clear any previous notes
  m_notes_out.clear();  
  // send to active mc
  m_midicycles[m_active_channel].note_event(note,velocity );
  
  noteEvent ne = {note, velocity, 0};
  if(velocity){
    // record note so we can flush if needed
    m_playing_notes.insert(note);
  } else {
    m_playing_notes.erase(note);
  }
  m_notes_out.push_back({m_active_channel,ne});
  
  return m_notes_out;
};

// Loop or play 
mc_timestep& MultiCycle::key_event(note_id note, char velocity) {
  
  // Clear any previous notes
  m_notes_out.clear();
  
  if(velocity > 0){
    if(note >= 60 && note < 72){
      int mc_id = note-60;
      if(mc_id == m_active_channel) {
        // loop, or if looping change overdub
        if( m_midicycles[m_active_channel].get_state() == mcState::EMPTY ) {
          m_midicycles[m_active_channel].loop(m_loop_length);
        } else {
          bool odub_state = !m_midicycles[m_active_channel].overdubbing();
          m_midicycles[m_active_channel].overdub(odub_state);
        }
        
      } else {      
        // Flush channel held notes and switch 
        for( const note_id &pnote : m_playing_notes){
          noteEvent ne = {pnote, 0, 0};
          m_notes_out.push_back({m_channels[m_active_channel],ne});
        }
        m_active_channel = mc_id;
      }
      
    } else if (note >= 72 && note <84)  {
      int mc_id = note-72;
      // Play/Stop
      m_midicycles[mc_id].playstop();
    }
    
  }
  
  return m_notes_out;
};

mc_timestep& MultiCycle::tick(int tick) {

  // Clear any previous notes
  m_notes_out.clear();
  
  // Get active notes from all mcs
  for(int mc_id = 0; mc_id <m_num_channels ; mc_id++){
    auto notes = m_midicycles[m_active_channel].tick(tick);
    for( auto note : notes){
      m_notes_out.push_back({m_channels[mc_id],note});
    }
  }
  
  return m_notes_out;
}
}