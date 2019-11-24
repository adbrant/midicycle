
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
  t_atom output_list[4];
} t_multicycle;

void multicycle_bang(t_multicycle *x) {
 // post("Test");
  //SETFLOAT(x->output_list, 2);
  //SETFLOAT(x->output_list + 1, 3);
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


void play_notes(t_multicycle *x) {
  
   while (x->x_multicycle->notesready()) {
    // Output events
    auto notepair = x->x_multicycle->popnote();
    auto note = notepair.second;
    int chan =  notepair.first;
    SETFLOAT(x->output_list, chan);
    SETFLOAT(x->output_list+1, note.note);
    SETFLOAT(x->output_list + 2, note.velocity);
    outlet_list(x->notes_out, &s_list, 3, x->output_list);
  } 
} 
void print_status(t_multicycle *x) {
  
 
  auto & status = x->x_multicycle->get_status();

  // make sure gate is open
  t_pd *oscoutgate = gensym("oscOutGate")->s_thing;
  if(oscoutgate != NULL) {
    SETFLOAT(x->output_list,1);
    pd_forwardmess(oscoutgate, 1,x->output_list );
  }  
  
  for(int i = 0; i < (int)status.size() / 2 ; i++ ){
    
    t_pd *target = gensym(status[i*2].c_str())->s_thing;
    if(target != NULL){
      SETSYMBOL(x->output_list,gensym(status[i*2+1].c_str()));
      pd_forwardmess(target, 1,x->output_list );
    } else{
      post("No object %s",status[i*2].c_str());
    }
  }
} 

void send_pd_msg(t_multicycle *x) {
  std::vector<std::string> msg;
  
  if(x->x_multicycle->get_pd_message(msg )){
    if(msg.size() > 4){
      post("send_pd_msg too big");
      return;
    }

    t_pd *target = gensym("rackMsgs")->s_thing;
    if(target != NULL){
      
      for(int i = 0; i < msg.size() ; i++ ){
        
        try {
          float number =  std::stof(msg[i]);
          SETFLOAT(x->output_list+i,number);
        } catch (...) {
          SETSYMBOL(x->output_list+i,gensym(msg[i].c_str()));
        }
      }
      pd_forwardmess(target, msg.size(),x->output_list );
    } else{
      post("No rackMsgs object");
    }
  } 
} 


void multicycle_note(t_multicycle *x, t_floatarg f1, t_floatarg f2, t_floatarg f3) {
  x->x_multicycle->note_event(int(f1), int(f2), int(f3));
  play_notes(x);
}
void multicycle_key(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_multicycle->key_event(int(f1), int(f2));
  play_notes(x);
  print_status(x);
}
void multicycle_aux(t_multicycle *x, t_floatarg f1) {
  x->x_multicycle->aux(int(f1));
}

void multicycle_fs(t_multicycle *x, t_floatarg f1) {
  x->x_multicycle->footswitch(int(f1));
  if( x->x_multicycle->mainpage() ) {
    print_status(x);
  }
}
void multicycle_dest(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_multicycle->set_dest(int(f1), int(f2));
  play_notes(x);
  t_pd *target = gensym("screenLine5")->s_thing;
  if(target != NULL && !x->x_multicycle->mainpage()) {
    SETSYMBOL(x->output_list,gensym(get_dst_display(int(f2))));
    pd_forwardmess(target, 1,x->output_list );
  }
  if( x->x_multicycle->mainpage() ) {
    print_status(x);
  }
}

void multicycle_transpose(t_multicycle *x, t_floatarg f1, t_floatarg f2) {
  x->x_multicycle->set_transpose(int(f1), int(f2));
  if( x->x_multicycle->mainpage() ) {
    print_status(x);
  }
}

void multicycle_src(t_multicycle *x, t_floatarg f1) {
  x->x_multicycle->set_src(int(f1));
  play_notes(x);
  t_pd *target = gensym("screenLine5")->s_thing;
  if(target != NULL && !x->x_multicycle->mainpage() ) {
    SETSYMBOL(x->output_list,gensym(get_src_display(int(f1))));
    pd_forwardmess(target, 1,x->output_list );
  }
    
  if( x->x_multicycle->mainpage() ) {
    print_status(x);
  }
}

void multicycle_tick(t_multicycle *x, t_floatarg f1) {
  
  x->x_multicycle->tick(int(f1));  
  play_notes(x);
  
  if((int(f1) == 0) && x->x_multicycle->mainpage() && x->x_multicycle->state_changed() ) {
    print_status(x);
  }
}

void multicycle_loop(t_multicycle *x, t_floatarg beats) {
  x->x_multicycle->set_loop_length(int(beats));
}
void multicycle_overdub(t_multicycle *x, t_floatarg odub) {
  x->x_multicycle->overdub(-1, int(odub));
}

void multicycle_arm_mode(t_multicycle *x, t_floatarg arm_mode) {
  x->x_multicycle->set_arm_mode(arm_mode);
}

void multicycle_quantize(t_multicycle *x, t_floatarg quantize) {
  x->x_multicycle->quantize(int(quantize));
}

void multicycle_quantize_loop(t_multicycle *x,t_floatarg chan, t_floatarg quantize) {
  x->x_multicycle->quantize_loop(int(chan), int(quantize));
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

void multicycle_activepage(t_multicycle *x,t_symbol *page) {
  x->x_multicycle->active_page(page->s_name);
  print_status(x);
}

void multicycle_activemodule(t_multicycle *x,t_symbol *module) {
  x->x_multicycle->set_active_module(module->s_name);
}

void multicycle_moduleid(t_multicycle *x,t_symbol *id) {
  x->x_multicycle->set_module_id(id->s_name);
}

void multicycle_knob_raw(t_multicycle *x,t_floatarg knob, t_floatarg raw) {
  if(x->x_multicycle->is_main_page()){
    x->x_multicycle->set_knob(int(knob), raw);
    send_pd_msg(x);
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

  class_addmethod(multicycle_class, (t_method)multicycle_fs, gensym("fs"),
                  A_DEFFLOAT, 0);                   
  class_addmethod(multicycle_class, (t_method)multicycle_src, gensym("src"),
                  A_DEFFLOAT, 0);    
  class_addmethod(multicycle_class, (t_method)multicycle_dest, gensym("dest"),	
                   A_DEFFLOAT,A_DEFFLOAT, 0);                                      
  class_addmethod(multicycle_class, (t_method)multicycle_transpose, gensym("transpose"),
                  A_DEFFLOAT,A_DEFFLOAT, 0);                 
                                      
  class_addmethod(multicycle_class, (t_method)multicycle_tick, gensym("tick"),
                  A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_loop, gensym("loop"),
                  A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_overdub, gensym("overdub"),
                  A_DEFFLOAT, 0);                  
  class_addmethod(multicycle_class, (t_method)multicycle_arm_mode, gensym("arm_mode"),
                  A_DEFFLOAT, 0);          
  class_addmethod(multicycle_class, (t_method)multicycle_quantize,
                  gensym("quantize"), A_DEFFLOAT, 0);
    class_addmethod(multicycle_class, (t_method)multicycle_quantize_loop,
                  gensym("quantize_loop"), A_DEFFLOAT,A_DEFFLOAT, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_save,
                  gensym("save"), A_DEFSYMBOL, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_load,
                  gensym("load"), A_DEFSYMBOL, 0);  
  class_addmethod(multicycle_class, (t_method)multicycle_activepage,
                  gensym("activepage"), A_DEFSYMBOL, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_activemodule,
                  gensym("activemodule"), A_DEFSYMBOL, 0);
  class_addmethod(multicycle_class, (t_method)multicycle_moduleid,
                  gensym("moduleid"), A_DEFSYMBOL, 0);

  class_addmethod(multicycle_class, (t_method)multicycle_knob_raw, gensym("knobraw"),
                  A_DEFFLOAT,A_DEFFLOAT, 0);   

                     
}
}
