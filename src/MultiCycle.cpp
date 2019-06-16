#include "MultiCycle.hpp"
#include <set>
#include <string>
#include <deque>
#include <cstring>
#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
#include <assert.h>
namespace MCycle {


 // Return string literal of src name (three character)
const char* get_src_display(int src){

  switch (src)
  {
      case -1:
        return "ogk";
      case 0:
        return "mxx ";
      case 1:
        return "m01";
      case 2:
        return "m02";
      case 3:
        return "m03";
      case 4:
        return "m04";
      case 5:
        return "m05";        
      case 6:
        return "m06";      
      case 7:
        return "m07";   
      case 8:
        return "m08";   
      case 9:
        return "m09";    
      case 10:
        return "m10";    
      case 11:
        return "m11";
      case 12:
        return "m12";
      case 13:
        return "m13";
      case 14:
        return "m14";
      case 15:
        return "m15";        
      case 16:
        return "m16";          
  }
  assert(0);
  return "";
}


 // Return string literal of src name (three character)
const char* get_dst_display(int dst){

  switch (dst)
  {
      case 1:
        return "a1 ";
      case 2:
        return "a2 ";
      case 3:
        return "a3 ";
      case 4:
        return "b1 ";
      case 5:
        return "b2 ";        
      case 6:
        return "b3 ";      
      case 7:
        return "b4 ";   
      case 8:
        return "c1 ";   
      case 9:
        return "c2 ";    
      case 10:
        return "c3 ";    
      case 11:
        return "m01";
      case 12:
        return "m02";
      case 13:
        return "m03";
      case 14:
        return "m04";
      case 15:
        return "m05";        
      case 16:
        return "m06";      
      case 17:
        return "m07";   
      case 18:
        return "m08";   
      case 19:
        return "m09";    
      case 20:
        return "m10";    
      case 21:
        return "m11";
      case 22:
        return "m12";
      case 23:
        return "m13";
      case 24:
        return "m14";
      case 25:
        return "m15";        
      case 26:
        return "m16";          
  }
  assert(0);
  return "";
}

std::vector<std::string>& MultiCycle::get_status() {
  if(!m_mainpage) {
    return m_status_empty;
  }
  m_status[1].clear();
  m_status[3].clear();
  m_status[5].clear();
  
  const int blankspaces = 4;
  int totallen = blankspaces*2+m_num_channels+1;
  
  
  for(int i = 0 ; i < totallen; i ++) {
    m_status[1] += " ";
    m_status[3] += " ";
    m_status[5] += " ";
  }
  
  m_status[1].replace(0, 3 , "SRC");
  m_status[3].replace(0, 3 , get_src_display(m_midi_src) );
  
  m_status[1].replace(totallen-3, 3 , "DST");
  m_status[3].replace(totallen-3, 3 , get_dst_display(m_channel_dests[m_active_channel]));
  
  if(m_midicycles[m_active_channel].get_state() != mcState::EMPTY) {
    int length = m_midicycles[m_active_channel].looplen();
    auto numstring = std::to_string(length);
    int copylength = numstring.length() > 2 ? 2 : numstring.length();
    m_status[5].replace(totallen-4, copylength, "L:");
    m_status[5].replace(totallen-2, copylength, numstring);
  }
  for(int i = 0 ; i < m_num_channels; i++) {
    
    auto & mc = m_midicycles[i];
    mcState state  = mc.get_state();
    bool active = i == m_active_channel;

    
    bool blackkey = i == 1 or i == 3 or i == 6 or i == 8 or i == 10;
    int statusline = blackkey ? 3 : 5;
    if(active and  blackkey)  m_status[1][blankspaces+i] = 'v';
    if(active and  !blackkey)  m_status[3][blankspaces+i] = 'v';
    if(state == mcState::EMPTY){
      m_status[statusline][blankspaces+i] =  'o';
    } else if (state == mcState::PLAYING){
      m_status[statusline][blankspaces+i] =   '>';
      
    } else if (state == mcState::STOP){
      m_status[statusline][blankspaces+i] =   '|';
      
    }
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