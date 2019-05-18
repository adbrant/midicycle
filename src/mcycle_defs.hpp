#pragma once

#define DEBUG_POST post

namespace MCycle {
enum { PPQ = 24 }; // Pulses per quarter note

// note_id may need to include channel later
typedef char note_id;

struct noteOnInfo {
  char velocity;
  long long start;
};

struct noteEvent {
  char note;
  char velocity;
  short duration;
};

typedef std::vector<noteEvent> timestep;
typedef std::vector<std::unique_ptr<timestep>> sequence;
}