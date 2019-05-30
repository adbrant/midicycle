
#include "MultiCycle.hpp"
#include "m_pd.h"
#include <fstream>
#include <cereal/archives/binary.hpp>
// Pure data external interface definition
using namespace MCycle;
extern "C" {
static t_class *multicycle_class;

typedef struct _multicycle {
  t_object x_obj;
  MultiCycle *x_multicycle;
  t_outlet *notes_out;
  t_outlet *status_out;
  // channel note velocity
  t_atom output_list[3];
} t_multicycle;

void multicycle_bang(t_multicycle *x) {
  post("Test");
  SETFLOAT(x->output_list, 2);
  SETFLOAT(x->output_list + 1, 3);
  //outlet_list(x->notes_out, &s_list, 2, x->output_list);
}

void *multicycle_new(t_floatarg loopers) {
  t_multicycle *x = (t_multicycle *)pd_new(multicycle_class);
  x->notes_out = outlet_new(&x->x_obj, &s_list);
  x->status_out = outlet_new(&x->x_obj, &s_symbol);
  x->x_multicycle = new MultiCycle(PPQ * 4 * 16,loopers);

  return (void *)x;
}

void multicycle_free(t_multicycle *x) { delete x->x_multicycle; }


void play_notes(t_multicycle *x, const mc_timestep & tstep) {
  
   for (auto &notepair : tstep) {
    // Output events
    auto note = notepair.second;
    int chan =  notepair.first;
    post("Playing note %d %d %d",chan,note.note, note.velocity);
    SETFLOAT(x->output_list, chan);
    SETFLOAT(x->output_list+1, note.note);
    SETFLOAT(x->output_list + 2, note.velocity);
    outlet_list(x->notes_out, &s_list, 3, x->output_list);
  } 
} 

void multicycle_note(t_multicycle *x, t_floatarg f1, t_floatarg f2, t_floatarg f3) {
  auto &tstep = x->x_multicycle->note_event(int(f1), int(f2), int(f3));
  play_notes(x, tstep);
}
void multicycle_key(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  auto &tstep = x->x_multicycle->key_event(int(f1), int(f2));
  play_notes(x, tstep);
  std::string & status = x->x_multicycle->get_status();

  outlet_symbol(x->status_out, gensym(status.c_str()));
}
void multicycle_aux(t_multicycle *x, t_floatarg f1) {
  x->x_multicycle->aux(int(f1));
}
void multicycle_dest(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  auto &tstep = x->x_multicycle->set_dest(int(f1), int(f2));
  play_notes(x, tstep);
}

void multicycle_src(t_multicycle *x, t_floatarg f1) {
  auto &tstep = x->x_multicycle->set_src(int(f1));
  play_notes(x, tstep);
}

void multicycle_tick(t_multicycle *x, t_floatarg f1) {
  auto &tstep = x->x_multicycle->tick(int(f1));  
  play_notes(x, tstep);
}

void multicycle_loop(t_multicycle *x, t_floatarg beats) {
  x->x_multicycle->set_loop_length(int(beats));
}

void multicycle_quantize(t_multicycle *x, t_floatarg division) {
  x->x_multicycle->quantize(int(division));
}

void multicycle_save(t_multicycle *x, t_symbol *file) {
  
  std::ofstream os(file->s_name,std::ios::binary);
  if(os){
    cereal::BinaryOutputArchive  archive(os);
    archive(*(x->x_multicycle));
  }
}


void multicycle_load(t_multicycle *x,t_symbol *file) {
  std::ifstream is(file->s_name,std::ios::binary);
  if(is){
    cereal::BinaryInputArchive  archive(is);
    archive(*(x->x_multicycle));
  }
}


void multicycle_setup(void) {
  multicycle_class = class_new(gensym("multicycle"), (t_newmethod)multicycle_new,
                              (t_method)multicycle_free, sizeof(t_multicycle),
                              CLASS_DEFAULT,A_DEFFLOAT, (t_atomtype)0);
  class_addbang(multicycle_class, multicycle_bang);
  class_addmethod(multicycle_class, (t_method)multicycle_note, gensym("note"),
                  A_DEFFLOAT, A_DEFFLOAT,A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_key, gensym("key"),
                  A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_aux, gensym("aux"),
                  A_DEFFLOAT, 0);                  
  class_addmethod(multicycle_class, (t_method)multicycle_src, gensym("src"),
                  A_DEFFLOAT, 0);    
  class_addmethod(multicycle_class, (t_method)multicycle_dest, gensym("dest"),
                  A_DEFFLOAT,A_DEFFLOAT, 0);
                   
  class_addmethod(multicycle_class, (t_method)multicycle_tick, gensym("tick"),
                  A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_loop, gensym("loop"),
                  A_DEFFLOAT, 0);
                  

  class_addmethod(multicycle_class, (t_method)multicycle_quantize,
                  gensym("quantize"), A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_save,
                  gensym("save"), A_DEFSYMBOL, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_load,
                  gensym("load"), A_DEFSYMBOL, 0);                    
}
}
