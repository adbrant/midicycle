
	
	
	
#include<vector>
#include<memory>
#include<utility>
#include <stdlib.h>  
#include <assert.h> 
struct  note_event {
  char note;
	char velocity;
	char channel;
	short duration;
	};

    
		
		
using timestep = std::vector<note_event>;
using sequence = std::vector<timestep>;

struct SeqRecorder {
public:
SeqRecorder(int sequence_size, int empty_reserve=32, int note_reserve=8) :
m_note_reserve{note_reserve}, m_sequence_data(sequence_size),m_sequence_size{sequence_size} {

};

void add_note(int step, note_event note);
void clear_step(int step) {
  	assert(step >= 0 && step < m_sequence_size);
		m_sequence_data[step].clear();
   };
   
void print_usage();
private:
  int m_sequence_size;
  int m_note_reserve;
	int m_step;
	sequence m_sequence_data;
};
		
void SeqRecorder::add_note(int step, note_event note)
{
	// Optimized for
	//   Low occupancy
	//   Frequent empty of steps and insert of notes
	//   Std::vectors will keep their buffers so
	assert(step >= 0 && step < m_sequence_size);
	m_sequence_data[step].push_back(note);
}

void SeqRecorder::print_usage()
{
	int total = 0 ;
  for( auto & p : m_sequence_data) {
      total += p.capacity();
  }
  printf("Total in Note = %d\n", total);
}


int main() {
  
   int steps=24*4*64*64;
   SeqRecorder record = SeqRecorder(steps);
    
   for(int step = 0; step < 200000000; step++){
       //printf("Step %d \n", step);
       note_event n;
       record.clear_step(step%steps);
       if(rand() % 256 < 8) { 
        record.add_note(rand() % steps, n);
       }
       
       
   }
   record.print_usage();
   printf("Done\n");
   return 0;
}