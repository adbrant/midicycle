#pragma once
#include <vector>
#include <map>
#include <memory>
#include <stdlib.h>
#include <utility>



#ifdef TEST
#define DEBUG_POST 
#else
#include "m_pd.h"
#define DEBUG_POST post
#endif


namespace MCycle {
enum { PPQ = 24 }; // Pulses per quarter note

// note_id may need to include channel later
typedef char note_id;

// global monotonic time position in PPQ
typedef long long global_step;

// position inside a loop 
typedef short local_step;

// channel
typedef short chan_id;


struct noteEvent {
  char note;
  char velocity;
  local_step duration;
};

typedef std::vector<noteEvent> timestep;

}