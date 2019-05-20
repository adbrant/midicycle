
#include "MultiCycle.hpp"
#include "m_pd.h"

// Pure data external interface definition
using namespace MCycle;
extern "C" {
static t_class *multicycle_class;

typedef struct _multicycle {
  t_object x_obj;
  MultiCycle *x_multicycle;
  t_outlet *notes_out;
  // channel note velocity
  t_atom output_list[3];
} t_multicycle;

void multicycle_bang(t_multicycle *x) {
  post("Test");
  SETFLOAT(x->output_list, 2);
  SETFLOAT(x->output_list + 1, 3);
  outlet_list(x->notes_out, &s_list, 2, x->output_list);
}

void *multicycle_new(t_floatarg loopers) {
  t_multicycle *x = (t_multicycle *)pd_new(multicycle_class);
  x->notes_out = outlet_new(&x->x_obj, &s_list);
  x->x_multicycle = new MultiCycle(PPQ * 4 * 16,loopers);

  return (void *)x;
}

void multicycle_free(t_multicycle *x) { delete x->x_multicycle; }

void multicycle_note(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_multicycle->note_event(unsigned(f1), unsigned(f2));
}

void multicycle_tick(t_multicycle *x, t_floatarg f1) {
  auto &tstep = x->x_multicycle->tick(unsigned(f1));
  
  for (auto &notepair : tstep) {
    // Output events
    auto note = notepair.second;
    int chan =  notepair.first;
    SETFLOAT(x->output_list, chan);
    SETFLOAT(x->output_list+1, note.note);
    SETFLOAT(x->output_list + 2, note.velocity);
    outlet_list(x->notes_out, &s_list, 2, x->output_list);
  }  
}

void multicycle_loop(t_multicycle *x, t_floatarg beats) {
  //x->x_multicycle->loop(unsigned(beats));
}

void multicycle_quantize(t_multicycle *x, t_floatarg division) {
  //x->x_multicycle->quantize(unsigned(division));
}

void multicycle_overdub(t_multicycle *x, t_floatarg odub) {
  //x->x_multicycle->overdub(odub > 0.5);
}

void multicycle_setup(void) {
  multicycle_class = class_new(gensym("multicycle"), (t_newmethod)multicycle_new,
                              (t_method)multicycle_free, sizeof(t_multicycle),
                              CLASS_DEFAULT,A_DEFFLOAT, (t_atomtype)0);
  class_addbang(multicycle_class, multicycle_bang);
  class_addmethod(multicycle_class, (t_method)multicycle_note, gensym("note"),
                  A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_tick, gensym("tick"),
                  A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_loop, gensym("loop"),
                  A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_quantize,
                  gensym("quantize"), A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_overdub,
                  gensym("overdub"), A_DEFFLOAT, 0);                  
}
}
