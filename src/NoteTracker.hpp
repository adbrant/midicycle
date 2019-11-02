#pragma once
#include <set>
#include <math.h>
#include <string>
#include <deque>
#include <cstring>
#include "mcycle_defs.hpp"
namespace MCycle {
class NoteTracker {
  public:
  NoteTracker()
  {};
  struct NoteTrackerInfo{
    NoteTrackerInfo(global_step g, int t) : gs(g), transpose(t){};
    global_step gs;
    int transpose;    
  };
  void note_on(global_step step, note_id note, int transpose){
    // Insert into held notes
    std::pair<char, NoteTrackerInfo>  note_on{note, NoteTrackerInfo(step,transpose)  };
    m_held_notes.insert(note_on);
  }
  // note off, return length or 0 if not found
  void note_off(global_step step, note_id note, global_step & duration, int & transpose ){

    // Note off
    auto iter = m_held_notes.find(note);
    if (iter != m_held_notes.end()) {
      const auto &start_step = (*iter).second.gs;
      duration = step - start_step;
      m_held_notes.erase(iter);
      transpose = (*iter).second.transpose;
    }
    
    return;
  }
  // clear all tracked notes
  void clear() {
    m_held_notes.clear();
  }
  //clear all tracked notes and return note offs
  void clear(std::vector<note_id> & note_offs) {
    for( auto & held_note :m_held_notes ){
      note_offs.push_back( char(held_note.first+held_note.second.transpose));
    }
    m_held_notes.clear();
  }  
  private:
    std::multimap<note_id, NoteTrackerInfo> m_held_notes;
  
};
}