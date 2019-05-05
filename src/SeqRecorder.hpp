#pragma once

#include<vector>
#include<memory>
#include <map>
#include<utility>
#include <stdlib.h>  
#include <assert.h> 
#include "mcycle_defs.hpp"
namespace MCycle {


/*
SeqRecorder used to record midi notes into a buffer 
*/
class SeqRecorder {
  public:
    SeqRecorder(int sequence_size, int empty_reserve=32, int note_reserve=8) :
     m_sequence_size{sequence_size}, m_note_reserve{note_reserve}, 
     m_sequence_data(sequence_size),m_empty_vectors{}
    {
      m_empty_vectors.reserve(empty_reserve);
    };

    void add_note(int step, noteEvent note);
    void clear_step(int step) {
      assert(step >= 0 && step < m_sequence_size);
      if(!m_sequence_data[step]) return;
      
      m_sequence_data[step]->clear();
      m_empty_vectors.push_back( std::move(m_sequence_data[step]));
     };
    
    bool step_empty(int step){
      return (m_sequence_data[step] == nullptr);  
    }
    const timestep& get_step (int step) const {
      assert(step >= 0 && step < m_sequence_size);
      return *m_sequence_data[step];
     };
     
    // Report memory consumed
    void print_usage() const;
  private:
    int m_sequence_size;
    int m_note_reserve;

    sequence m_sequence_data;
    sequence m_empty_vectors;
    
};
		
}