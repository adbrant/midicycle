#include "SeqRecorder.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <fstream>

using namespace MCycle;

void SeqRecorder::add_note(int step, noteEvent note) {
  // Optimized for
  //   Low occupancy
  //   Frequent empty of steps and insert of notes
  //   Std::vectors will keep their buffers so
  assert(step >= 0);
  assert(step < m_sequence_size);
  DEBUG_POST("Add note at %d", step);
  if (!m_sequence_data[step]) {
    if (m_empty_vectors.empty()) {
      m_sequence_data[step] = std::make_unique<std::vector<noteEvent>>();
      m_sequence_data[step]->reserve(m_note_reserve);
    } else {
      std::swap(m_sequence_data[step], m_empty_vectors.back());
      m_empty_vectors.pop_back();
    }
  }
  m_sequence_data[step]->push_back(note);
}

void SeqRecorder::print_usage() const {
  int total = 0;
  for (auto &p : m_sequence_data) {
    if (p) {
      total += p->capacity();
    }
  }
  printf("Total in Note = %d\n", total);
  total = 0;
  for (auto &p : m_empty_vectors) {
    if (p) {
      total += p->capacity();
    }
  }
  printf("Total in Empty = %d\n", total);
}

#ifdef TEST 
/*
int main() {

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
  return 0;
}
*/
#endif