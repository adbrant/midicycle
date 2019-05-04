#include "m_pd.h"  
 
static t_class *midicycle_class;  
 
typedef struct _midicycle {  
  t_object  x_obj;  
} t_midicycle;  
 
void midicycle_bang(t_midicycle *x)  
{  
}  
 
void *midicycle_new(void)  
{  
  t_midicycle *x = (t_midicycle *)pd_new(midicycle_class);  
  
  return (void *)x;  
}  
 
void midicycle_setup(void) {  
  midicycle_class = class_new(gensym("midicycle"),  
        (t_newmethod)midicycle_new,  
        0, sizeof(t_midicycle),  
        CLASS_DEFAULT, 0);  
  class_addbang(midicycle_class, midicycle_bang);  
}
