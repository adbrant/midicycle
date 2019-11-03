#include "OscPool.hpp"
#include "m_pd.h"
#include <cereal/archives/binary.hpp>
#include <fstream>
// Pure data external interface definition
using namespace MCycle;
extern "C" {
static t_class *oscpool_class;


typedef struct _oscpool {
  t_object x_obj;
  OscPool *x_oscpool;
  t_outlet *notes_out;
  t_outlet *gate_out;
  t_atom output_list[2];
  t_int numoscs;
} t_oscpool;

void oscpool_bang(t_oscpool *x) {
  post("Test");
  SETFLOAT(x->output_list, 2);
  SETFLOAT(x->output_list + 1, 3);
  outlet_list(x->notes_out, &s_list, 2, x->output_list);
}

void *oscpool_new(t_floatarg oscs) {
  t_oscpool *x = (t_oscpool *)pd_new(oscpool_class);
  x->notes_out = outlet_new(&x->x_obj, &s_list);
  x->gate_out = outlet_new(&x->x_obj, &s_list);
  x->x_oscpool = new OscPool(oscs);
  x->numoscs = oscs;
  return (void *)x;
}

void oscpool_free(t_oscpool *x) { delete x->x_oscpool; }

void oscpool_note(t_oscpool *x, t_floatarg f1, t_floatarg f2) {
  x->x_oscpool->note_event(unsigned(f1), unsigned(f2));
  
  for(int i = 0; i < x->numoscs; i++){
    SETFLOAT(x->output_list, i);
    SETFLOAT(x->output_list+1,  x->x_oscpool->oscval(i));
    outlet_list(x->notes_out, &s_list, 2, x->output_list);
  }
   SETFLOAT(x->output_list, x->x_oscpool->gate());
   outlet_list(x->gate_out, &s_list, 1, x->output_list);  
}

void oscpool_reset(t_oscpool *x, t_symbol sym) {
  x->x_oscpool->reset();
}
void oscpool_setup(void) {
  oscpool_class = class_new(gensym("oscpool"), (t_newmethod)oscpool_new,
                              (t_method)oscpool_free, sizeof(t_oscpool),
                              CLASS_DEFAULT, A_DEFFLOAT,(t_atomtype)0);
  class_addbang(oscpool_class, oscpool_bang);
  class_addmethod(oscpool_class, (t_method)oscpool_note, gensym("note"),
                  A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(oscpool_class, (t_method)oscpool_reset, gensym("reset"), A_DEFSYMBOL, 0);
                 
}
}
