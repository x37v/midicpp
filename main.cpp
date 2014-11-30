#include "midicpp.h"
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
  cout << "inputs: " << midicpp::Input::device_count() << endl;
  for (std::string n: midicpp::Input::device_list())
    cout << "\t" << n << endl;

  midicpp::Input in(0);
  try {
    in.with_message3(midicpp::SYSEX_BEGIN, [](uint8_t x, uint8_t y, uint8_t z) { });
  } catch (std::runtime_error& e) {
    cout << "got expected error: " << e.what() << endl;
  }

  auto cc = [](uint8_t chan, uint8_t num, uint8_t val) { };
  in.with_message3(midicpp::CC, cc);
  in.with_message3(midicpp::CC, nullptr); //clear it
  return 0;
}

