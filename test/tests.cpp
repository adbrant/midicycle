#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include "catch.hpp"
#define CATCHDEBUG
#include "SeqRecorder.hpp"
#include "MidiCycle.hpp"
#include "MultiCycle.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>
#include <iostream>
#define RUNSTEPS 512
using namespace MCycle;

TEST_CASE("SeqRecorder usage and serialization", "[SeqRecorder]"){
  
  int steps = 24 * 4 * 64;
  SeqRecorder record = SeqRecorder(steps);
  noteEvent n;
  for (int step = 0; step < RUNSTEPS; step++) {
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
  
  int steps = 24 * 4 * 64;
  MidiCycle mc = MidiCycle(steps);
  noteEvent n;
  bool note_playing = false ;
  int note = 0;
  for (int step = 0; step < RUNSTEPS; step++) {
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

TEST_CASE("MidiCycle playback", "[MidiCycle]"){
  
  // Basic playback
  for(int loopstep = 14; loopstep < 25 ; loopstep++){ 
    printf("loopstep %d\n",loopstep);
    MidiCycle mc = MidiCycle(PPQ * 4 * 64);
    noteEvent n;
    bool note_playing = false ;
    int note_id = 64;
    int count = 0;
    int steps = PPQ*20;
    for (int step = 0; step < steps; step++) {
      
      switch(step){       
       case 1:
        mc.note_event(note_id, 127); 
        break;
       case 13:
        mc.note_event(note_id, 0);
        break;
      }
      if(step == loopstep){
        mc.loop(1);
      }
      const timestep& notes = mc.tick(step%PPQ);
      for( auto & note : notes) {
        count++;
        printf("step %d note %d %d\n",step, note.note, note.velocity);
        if(step > PPQ) {
         switch(step%PPQ){       
         case 1:
          CHECK(note.note == note_id);
          CHECK(note.velocity == 127);
          break;
         case 13:
          CHECK(note.note == note_id);
          CHECK(note.velocity == 0);
          break;
         default:
            CHECK(0);
        }
        }
       }
    }
    printf("count %d %d\n",count,2*(steps-PPQ)/PPQ);
    CHECK(count == 2*(steps-PPQ)/PPQ );
  }
}




TEST_CASE("MidiCycle quantized playback", "[MidiCycle]"){
  for(int quantize = 0; quantize <=4; quantize++)  {
    printf("quantize %d \n", quantize);
    // Basic playback
    MidiCycle mc = MidiCycle(PPQ * 4 * 64);
    noteEvent n;
    bool note_playing = false ;
    int note_id = 64;
    int count = 0;
    int steps =  PPQ*20;
    mc.quantize(quantize);
    for (int step = 0; step < steps; step++) {
      
      switch(step){       
       case 1:
        mc.note_event(note_id, 127); 
        break;
       case 13:
        mc.note_event(note_id, 0);
        break;
       case 24:
        mc.loop(1);
      }
      const timestep& notes = mc.tick(step%PPQ);
      for( auto & note : notes) {
        count++;
        printf("step %d note %d %d\n",step, note.note, note.velocity);
        CHECK((note.velocity == 0 || quantize == 0 || (step%(PPQ/quantize)) == 0));
  
      }
    }
    printf("count %d %d\n",count,2*(steps-PPQ)/PPQ);
    CHECK(count == 2*(steps-PPQ)/PPQ );
  }
    
}

TEST_CASE("MultiCycle usage and serialization", "[MultiCycle]"){
  
  int steps = 24 * 4 * 64;
  MultiCycle mc = MultiCycle(steps,12);
  noteEvent n;
  bool note_playing = false ;
  int note = 0;
  for (int step = 0; step < RUNSTEPS; step++) {
    mc.tick(step%24);
    
    if(rand() % 256 < 8) {
      if (!note_playing ) {
        note = rand()% 127;
        mc.note_event(1,note, 127);
        note_playing = true;
      } else {
        note_playing = false;
        mc.note_event(1,note, 0);
      }

     }
  }
  {
    std::ofstream os("datamuc.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(mc);
  }
  {
    std::ofstream os("datamuc.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(mc);
  }  
  {
    std::ifstream is("datamuc.json");
    assert(is);
    cereal::JSONInputArchive  archive(is);
    archive(mc);
  }
  {
    
    std::ofstream os("data2muc.json");
    assert(os);
    cereal::JSONOutputArchive  archive(os);
    archive(mc);
  }
  MultiCycle mc2 = MultiCycle(steps, 12);
  {

    std::ifstream is("datamuc.bin",std::ios::binary);
    assert(is);
    cereal::BinaryInputArchive  archive(is);
    archive(mc2);
  }  
  {
    std::ofstream os("data2muc.bin",std::ios::binary);
    assert(os);
    cereal::BinaryOutputArchive  archive(os);
    archive(mc2);
  }
  printf("Done\n");
}



TEST_CASE("MultiCycle knob tracking", "[MultiCycle]"){
  
  int steps = 24 * 4 * 64;
  MultiCycle mc = MultiCycle(steps,12);
  mc.set_module_id("s2");
  CHECK(!mc.is_main_page() );
  mc.active_page("pg_main");
  mc.set_active_module("s2");
  CHECK(mc.is_main_page() );
  std::vector<std::string> msg;
  int knob = 3;
  // no message first call
  mc.set_knob(knob, 0.5);
  CHECK(msg.size() == 0);
  CHECK(mc.get_pd_message(msg ) == false);
  CHECK(msg.size() == 0);

  // no message repeat value
  mc.set_knob(knob, 0.5);
  CHECK(msg.size() == 0);
  CHECK(mc.get_pd_message(msg ) == false);
  CHECK(msg.size() == 0);  
  
  // no message repeat value
  mc.set_knob(knob, 0.8);
  CHECK(mc.get_pd_message(msg ) == true);
  CHECK(msg.size() == 4); 
  for( auto& str : msg){
    std::cout << str << std::endl;     
  }
  CHECK(msg[0] == "set_param"); 
  CHECK(msg[1] == "s2"); 
  CHECK(msg[2] == "mc_inchan");
  CHECK( msg[3].find("0.8")== 0);
  msg.clear();
  
  knob = 4;
  // no message first call
  mc.set_knob(knob, 0.4);
  CHECK(msg.size() == 0);
  CHECK(mc.get_pd_message(msg ) == false);
  CHECK(msg.size() == 0);
  for( auto& str : msg){
    std::cout << str << std::endl;     
  }
  // no message repeat value
  mc.set_knob(knob, 0.4);
  CHECK(msg.size() == 0);
  CHECK(mc.get_pd_message(msg ) == false);
  CHECK(msg.size() == 0);  
  
  // no message repeat value
  mc.set_knob(knob, 0.6);
  CHECK(mc.get_pd_message(msg ) == true);
  CHECK(msg.size() == 4); 
  for( auto& str : msg){
    std::cout << str << std::endl;     
  }
  CHECK(msg[0] == "set_param"); 
  CHECK(msg[1] == "s2"); 
  CHECK(msg[2] == "mc_chan_0");
  CHECK( msg[3].find("0.6")== 0);
}

