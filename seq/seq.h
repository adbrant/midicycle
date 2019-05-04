#pragma once

#include<vector>
#include<memory>
#include <map>
#include<utility>
#include <stdlib.h>  
#include <assert.h> 

struct  noteEvent {
  char note;
	char velocity;
	short duration;
	};

struct  noteOnInfo {
	char velocity;
	int start;
	};
    
typedef  std::vector<noteEvent> timestep;
typedef  std::vector<std::unique_ptr<timestep>> sequence;

class SeqRecorder {
public:
SeqRecorder(int sequence_size, int empty_reserve=32, int note_reserve=8) :
m_note_reserve{note_reserve}, m_sequence_data(sequence_size),m_empty_vectors{},m_sequence_size{sequence_size},m_held_notes()
{

	m_empty_vectors.reserve(empty_reserve);

};

void note_event(char note, char velocity) {
  assert(note >= 0 and velocity >= 0);
    
  if(velocity){
    // Note on
    noteOnInfo note_on_info = { velocity, m_step};
    m_held_notes.insert({note, note_on_info});
  } else {
    // Note off
    auto iter = m_held_notes.find( note);
    if(iter != m_held_notes.end()){
      const noteOnInfo & note_on_info = (*iter).second;
      // need to handle looping
      short duration = (short)(m_step - note_on_info.start);

      noteEvent note_event = {note, note_on_info.velocity, duration};
      add_note( note_on_info.start, note_event);
      
      m_held_notes.erase( iter);
    }
    
  }
    
};
  
void add_note(int step, noteEvent note);
void clear_step(int step) {

  	assert(step >= 0 && step < m_sequence_size);
		if(!m_sequence_data[step]) return;
    
		m_sequence_data[step]->clear();
		m_empty_vectors.push_back( std::move(m_sequence_data[step]));
   };
   
void print_usage();
private:
  int m_sequence_size;
  int m_note_reserve;
	int m_step;
	sequence m_sequence_data;
	sequence m_empty_vectors;
  std::multimap<char, noteOnInfo> m_held_notes; 
};
		