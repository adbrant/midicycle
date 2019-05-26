#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include "catch.hpp"

#include "SeqRecorder.hpp"
#include "MidiCycle.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

using namespace MCycle;

TEST_CASE("SeqRecorder usage and serialization", "[SeqRecorder]"){
  
    int steps = 24 * 4 * 64 * 64;
  SeqRecorder record = SeqRecorder(steps);
  noteEvent n;
  for (int step = 0; step < 20000; step++) {
    record.clear_step(step % steps);
    if (rand() % 256 < 8) {
      record.add_note(rand() % steps, n);
    }
    n.note++;
  }
  {
    record.print_usage();
    std::ofstream os("data.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(record);
  }
  {
    record.print_usage();
    std::ofstream os("data.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(record);
  }  
  {
    std::ifstream is("data.json");
    assert(is);
    cereal::JSONInputArchive  archive(is);
    archive(record);
    record.print_usage();
  }
  {
    record.print_usage();
    std::ofstream os("data2.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(record);
  }
  SeqRecorder record2(steps);  
  {

    std::ifstream is("data.bin",std::ios::binary);
    assert(is);
    cereal::BinaryInputArchive  archive(is);
    archive(record2);
    record2.print_usage();
  }  
  {
    std::ofstream os("data2.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(record2);
  }
  printf("Done\n");
}


TEST_CASE("MidiCycle usage and serialization", "[MidiCycle]"){
  
  int steps = 24 * 4 * 64 * 64;
  MidiCycle mc = MidiCycle(steps);
  noteEvent n;
  bool note_playing = false ;
  int note = 0;
  for (int step = 0; step < 20000; step++) {
    mc.tick(step%24);
    
    if(rand() % 256 < 8) {
      if (!note_playing ) {
        note = rand()% 127;
        mc.note_event(note, 127);
        note_playing = true;
      } else {
        note_playing = false;
        mc.note_event(note, 0);
      }

     }
  }
  {
    std::ofstream os("datamc.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(mc);
  }
  {
    std::ofstream os("datamc.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(mc);
  }  
  {
    std::ifstream is("datamc.json");
    assert(is);
    cereal::JSONInputArchive  archive(is);
    archive(mc);
  }
  {
    std::ofstream os("data2mc.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(mc);
  }
  MidiCycle mc2 = MidiCycle(steps);
  {

    std::ifstream is("datamc.bin",std::ios::binary);
    assert(is);
    cereal::BinaryInputArchive  archive(is);
    archive(mc2);
  }  
  {
    std::ofstream os("data2mc.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(mc2);
  }
  printf("Done\n");
}