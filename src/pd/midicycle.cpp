#include "MidiCycle.hpp"
#include "m_pd.h"
#include <cereal/archives/binary.hpp>
#include <fstream>
// Pure data external interface definition
using namespace MCycle;
extern "C" {
static t_class *midicycle_class;

typedef struct _midicycle {
  t_object x_obj;
  MidiCycle *x_midicycle;
  t_outlet *notes_out;
  t_atom output_list[2];
} t_midicycle;

void midicycle_bang(t_midicycle *x) {
  post("Test");
  SETFLOAT(x->output_list, 2);
  SETFLOAT(x->output_list + 1, 3);
  outlet_list(x->notes_out, &s_list, 2, x->output_list);
}

void *midicycle_new(void) {
  t_midicycle *x = (t_midicycle *)pd_new(midicycle_class);
  x->notes_out = outlet_new(&x->x_obj, &s_list);
  x->x_midicycle = new MidiCycle(PPQ * 4 * 16);

  return (void *)x;
}

void midicycle_free(t_midicycle *x) { delete x->x_midicycle; }

void midicycle_note(t_midicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_midicycle->note_event(unsigned(f1), unsigned(f2));
}

void midicycle_swing(t_midicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_midicycle->swing(unsigned(f1), unsigned(f2));
}

void midicycle_tick(t_midicycle *x, t_floatarg f1) {
  auto &tstep = x->x_midicycle->tick(unsigned(f1));
  
  for (auto &note : tstep) {
    // Output events
    SETFLOAT(x->output_list, note.note);
    SETFLOAT(x->output_list + 1, note.velocity);
    outlet_list(x->notes_out, &s_list, 2, x->output_list);
  }  
}

void midicycle_loop(t_midicycle *x, t_floatarg beats) {
  x->x_midicycle->loop(unsigned(beats));
}

void midicycle_quantize(t_midicycle *x, t_floatarg division) {
  x->x_midicycle->quantize(unsigned(division));
}

void midicycle_overdub(t_midicycle *x, t_floatarg odub) {
  x->x_midicycle->overdub(odub > 0.5);
}

void midicycle_arm_mode(t_midicycle *x, t_floatarg arm_mode) {
  x->x_midicycle->set_arm_mode(arm_mode > 0.5);
}


void midicycle_save(t_midicycle *x, t_symbol *file) {
  
  std::ofstream os(file->s_name,std::ios::binary);
  if(os){
    cereal::BinaryOutputArchive  archive(os);
    archive(*(x->x_midicycle));
  }
}


void midicycle_load(t_midicycle *x,t_symbol *file) {
  std::ifstream is(file->s_name,std::ios::binary);
  if(is){
    cereal::BinaryInputArchive  archive(is);
    archive(*(x->x_midicycle));
  }
}


void midicycle_setup(void) {
  midicycle_class = class_new(gensym("midicycle"), (t_newmethod)midicycle_new,
                              (t_method)midicycle_free, sizeof(t_midicycle),
                              CLASS_DEFAULT, (t_atomtype)0);
  class_addbang(midicycle_class, midicycle_bang);
  class_addmethod(midicycle_class, (t_method)midicycle_note, gensym("note"),
                  A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_tick, gensym("tick"),
                  A_DEFFLOAT, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_loop, gensym("loop"),
                  A_DEFFLOAT, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_quantize,
                  gensym("quantize"), A_DEFFLOAT, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_overdub,
                  gensym("overdub"), A_DEFFLOAT, 0);     
  class_addmethod(midicycle_class, (t_method)midicycle_arm_mode,
                  gensym("arm_mode"), A_DEFFLOAT, 0);  
  class_addmethod(midicycle_class, (t_method)midicycle_swing, gensym("swing"),
                  A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_save,
                  gensym("save"), A_DEFSYMBOL, 0);
  class_addmethod(midicycle_class, (t_method)midicycle_load,
                  gensym("load"), A_DEFSYMBOL, 0);                  
}
}
