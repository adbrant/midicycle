#pragma once
#include <set>
#include <math.h>
#include <string>
#include <deque>
#include <cstring>
#include "mcycle_defs.hpp"
namespace MCycle {
class KnobTracker {
  public:
  const float nullvalue = -1;  
  KnobTracker(int num_knobs=4) :
  m_num_knobs(num_knobs),m_values(num_knobs, nullvalue)
  { 
  }
  const float epsilon = 4.0/1024.0;
  //returns -1 if no update
  float update(int knob, float input){
    if(input == m_values[knob]) return nullvalue;
    if(fabs(input - m_values[knob]) < epsilon ) return nullvalue;
    if(m_values[knob] == nullvalue){
      m_values[knob] = input;
      return nullvalue;
    }
    m_values[knob] = input;
    return input;   
  }
  private:
  int m_num_knobs;
  std::vector<float> m_values;
};
}
