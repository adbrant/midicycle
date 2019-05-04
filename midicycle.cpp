#include "m_pd.h"
#include "MidiCycle.hpp"  
using namespace MCycle;
extern "C" {
static t_class *midicycle_class;  
 
typedef struct _midicycle {  
  t_object  x_obj;  
  MidiCycle* x_midicycle;
} t_midicycle;  
 
void midicycle_bang(t_midicycle *x)  
{  
    post("Test");  
}  
 
void *midicycle_new(void)  
{  
  t_midicycle *x = (t_midicycle *)pd_new(midicycle_class);  
  x->x_midicycle = new MidiCycle(24*16*16);
  return (void *)x;  
}

void midicycle_free(t_midicycle *x)  
{  
  delete x->x_midicycle;
}    

void midicycle_note(t_midicycle *x, t_floatarg f1, t_floatarg f2)  
{   
}    
 
void midicycle_setup(void) {  
  midicycle_class = class_new(gensym("midicycle"),  
        (t_newmethod)midicycle_new,  
        (t_method)midicycle_free, sizeof(t_midicycle),  
        CLASS_DEFAULT, (t_atomtype)0);  
  class_addbang(midicycle_class, midicycle_bang);
  class_addmethod(midicycle_class,
        (t_method)midicycle_note, gensym("note"),
        A_DEFFLOAT,A_DEFFLOAT, 0);  
}
}
