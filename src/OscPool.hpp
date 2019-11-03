#pragma once
#include <set>
#include <math.h>
#include <string>
#include <deque>
#include <assert.h>
#include "mcycle_defs.hpp"
#include "m_pd.h"
namespace MCycle {
  
// Allocate all held notes 
class OscPool {
  public:
  OscPool(int num_oscs )
  {
    m_used_notes = 0;
    m_num_oscs = num_oscs;
    
    for( int i = 0 ; i < m_num_oscs; i++){
      m_notes.push_back(-1);
      m_osc_count.push_back(0);
      m_osc_val.push_back(-1);
      m_osc_note_index.push_back(-1);
    }
  };
  void reset(){
    for( int i = 0 ; i < m_num_oscs; i++){
      m_notes[i] = -1;
      m_osc_count[i] = 0;
      m_osc_val[i] = -1;
      m_osc_note_index[i] = -1;
    }
    m_used_notes = 0;
  }
  void note_event(int note, int velocity){
    bool note_on = velocity > 0;

    if( note_on) {
       
      if(m_used_notes < m_num_oscs) {
        if(m_used_notes == 0){
          m_notes[0] = note;
          m_osc_count[0] = m_num_oscs;
          for( int i = 0 ; i < m_num_oscs; i++){
            m_osc_note_index[i] = 0;
            m_osc_val[i] = note;
          }
        } else {
          int min_oscs = m_num_oscs/(m_used_notes+1);
          int index = -1;
          // Find open slot
          for( int i = 0 ; i < m_num_oscs; i++){
            if(m_notes[i] == -1) {
              index= i;
              break;              
            }
          } 
          assert(index != -1);
          m_notes[index] = note;
          //initially reduce all to min_oscs+1
          for( int i = 0 ; i < m_num_oscs; i++){
            if( m_osc_count[index] < min_oscs && m_osc_count[m_osc_note_index[i]] > min_oscs+1){
              m_osc_count[m_osc_note_index[i]]--;
              m_osc_count[index]++;
              m_osc_note_index[i] = index;    
              m_osc_val[i] = note;
            }
          } 
          // reduce some to min_oscs
          for( int i = 0 ; i < m_num_oscs; i++){
            if( m_osc_count[index] < min_oscs && m_osc_count[m_osc_note_index[i]] > min_oscs){
              m_osc_count[m_osc_note_index[i]]--;
              m_osc_count[index]++;
              m_osc_note_index[i] = index;    
              m_osc_val[i] = note;
            }
          } 
        }
        m_used_notes++;
      }
    } else {
      // Note off
      int index = -1;
      // Find index for this note
      for( int i = 0 ; i < m_num_oscs; i++){
        if(m_notes[i] == note) {
          index= i;
          break;              
        }
      }      
      if(index == -1){
        // not found
        return;
      }
      if(m_used_notes == 1){
        // reset everything
        for( int i = 0 ; i < m_num_oscs; i++){
          m_osc_note_index[i] = -1;
          m_osc_val[i] = -1;
          m_osc_count[i] = 0;
          m_notes[i] = -1;
        }
        m_used_notes = 0;
      } else {
          m_notes[index] = -1;
          int min_oscs = m_num_oscs/(m_used_notes-1);
          int note_candidate = 0; 
          // Iterate through oscillators and notes to receive new oscillators
          for( int i = 0 ; i < m_num_oscs; i++){
            post("Osc %d", i);
            //increment until we find a note with less than minimum oscs
            while( note_candidate == index || m_notes[note_candidate] == -1 || m_osc_count[note_candidate] >= min_oscs) {
              post("Candidate %d %d %d",index,m_notes[note_candidate], m_osc_count[note_candidate]);
              note_candidate++;

            }
            if(note_candidate >= m_num_oscs ){
                break;
            }
            if( m_osc_count[index] > 0 &&  m_osc_note_index[i] == index){
              post("Match %d %d", i,note_candidate );
              m_osc_count[index]--;
              m_osc_count[note_candidate]++;
              m_osc_note_index[i] = note_candidate;    
              m_osc_val[i] = m_notes[note_candidate];
            }
          } 
          if(m_osc_count[index] > 0){
            //distribute remaining oscs
            note_candidate = 0; 
            // Iterate through oscillators and notes to receive new oscillators
            for( int i = 0 ; i < m_num_oscs; i++){
              //increment until we find a note with less than or equal to minimum oscs
              while( note_candidate == index || m_notes[note_candidate] == -1 || m_osc_count[note_candidate] > min_oscs) {
                note_candidate++;

              }
              if(note_candidate >= m_num_oscs ){
                  break;
              }  
              if( m_osc_count[index] > 0 &&  m_osc_note_index[i] == index){
                m_osc_count[index]--;
                m_osc_count[note_candidate]++;
                m_osc_note_index[i] = note_candidate;    
                m_osc_val[i] = m_notes[note_candidate];
              }
            } 
          }
          m_used_notes--;
      }
    }    
  }

  int gate(){
    return m_used_notes > 0;
    
    
  }
  int oscval(int i){
    return m_osc_val[i]; 
  }
  private:
    int m_used_notes;
    int m_num_oscs;
    std::vector<int> m_notes;
    std::vector<int> m_osc_count; 
    std::vector<int> m_osc_note_index;    
    std::vector<int> m_osc_val;
  
};
}