#pragma once
#include <cereal/access.hpp>
#include "mcycle_defs.hpp"
#include <assert.h>


namespace MCycle {
typedef std::vector<std::unique_ptr<timestep>> sequence;
/*
SeqRecorder used to record midi notes into a buffer
*/
class SeqRecorder {
friend class cereal::access;
public:
  SeqRecorder(int sequence_size, int empty_reserve = 32, int note_reserve = 8)
      : m_sequence_size{sequence_size}, m_note_reserve{note_reserve},
        m_sequence_data(sequence_size), m_empty_vectors{} {
    m_empty_vectors.reserve(empty_reserve);
  };
  void add_note(int step, noteEvent note);

  void clear_step(int step) {
    assert(step >= 0 && step < m_sequence_size);
    if (!m_sequence_data[step])
      return;

    m_sequence_data[step]->clear();
    m_empty_vectors.push_back(std::move(m_sequence_data[step]));
  };

  bool step_empty(int step) { return (m_sequence_data[step] == nullptr); }

  const timestep &get_step(int step) const {
    assert(step >= 0 && step < m_sequence_size);
    assert(m_sequence_data[step] != NULL);
    return *m_sequence_data[step];
  };

  // Report memory consumed
  void print_usage() const;
  
  template<class Archive>
  void save(Archive & archive) const
  {
    archive( m_sequence_size);
    long long num_notes = 0;    
    for(int i = 0 ; i < m_sequence_size; i++ ){
      if(m_sequence_data[i]){
        timestep ts = *m_sequence_data[i];
        num_notes += ts.size();
      }
    }
    archive( num_notes);
    num_notes = 0;
    for(int i = 0 ; i < m_sequence_size; i++ ){
      if(m_sequence_data[i]){
        timestep ts = *m_sequence_data[i];
        for(auto & note : ts){
          archive(i,note );
          num_notes++;
          
        }
      }
    }    
  }

  template<class Archive>
  void load(Archive & archive)
  {
    archive( m_sequence_size );
    long long num_notes;
    archive( num_notes );
    m_sequence_data = sequence(m_sequence_size);
    for(int i = 0 ; i < num_notes; i++ ){
      noteEvent note;
      int step;
      archive(step,note);
      add_note(step, note);
    }     
  }
private:
  int m_sequence_size;
  int m_note_reserve;

  sequence m_sequence_data;
  sequence m_empty_vectors;
};
}