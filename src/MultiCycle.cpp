#include "MultiCycle.hpp"
#include <set>
#include <string>
#include <deque>
#include <cstring>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
namespace MCycle {

std::vector<std::string>& MultiCycle::get_status() {
  if(!m_mainpage) {
    return m_status_empty;
  }
  m_status[1].clear();
  m_status[3].clear();
  m_status[5].clear();
  
  const int blankspaces = 4;
  for(int i = 0 ; i < blankspaces; i ++) {
    m_status[1] += " ";
    m_status[3] += " ";
    m_status[5] += " ";
  }
  for(int i = 0 ; i < m_num_channels; i ++) {
    
    auto & mc = m_midicycles[i];
    mcState state  = mc.get_state();
    bool active = i == m_active_channel;

    
    bool blackkey = i == 1 or i == 3 or i == 6 or i == 8 or i == 10;
    int statusline = blackkey ? 3 : 5;
    int emptyline = blackkey ? 5 : 3;
    m_status[1]+= (active && blackkey) ? "v" : " ";
    if(state == mcState::EMPTY){
      m_status[statusline] +=  "o";
    } else if (state == mcState::PLAYING){
      m_status[statusline]+=   ">";
      
    } else if (state == mcState::STOP){
      m_status[statusline]+=   "|";
      
    }
    m_status[emptyline] += !blackkey && active ? "v" :  " ";
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
    DEBUG_POST("Note thru AC:%d Dest:%d Note:%d Velocity:%d\n",m_active_channel, m_channel_dests[m_active_channel], note, velocity );
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
    return;
  } 
  
  if(velocity > 0){
    m_key_tracker.note_on(m_step_global, note);
  } else {
    auto length = m_key_tracker.note_off(m_step_global, note);
    
    if(length > 48 ){
      // Long Press
      if(note >= 60 && note < 72) {
        int mc_id = note-60;
        m_midicycles[mc_id].loop(0);
      } else if (note >= 72 && note <84)  {
        // Goto new page
      }     
    } else if (length > 1){
      // Short press
      if(note >= 60 && note < 72) {
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
  }
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
  m_step_global++;
  return;
}
}