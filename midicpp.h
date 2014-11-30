#ifndef XNORMIDI_H
#define XNORMIDI_H

#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <functional>
#include "RtMidi.h"

namespace midicpp {
  enum status_type_t {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    AFTERTOUCH = 0xA0,
    CC = 0xB0,
    PROGCHANGE = 0xC0,
    CHANPRESSURE =0xD0,
    PITCHBEND = 0xE0,

    SYSEX_BEGIN = 0xF0,
    TC_QUARTERFRAME = 0xF1,
    SONGPOSITION = 0xF2,
    SONGSELECT = 0xF3,
    TUNEREQUEST = 0xF6,
    SYSEX_END = 0xF7,

    //realtime
    CLOCK = 0xF8,
    TICK = 0xF9,
    START = 0xFA,
    CONTINUE = 0xFB,
    STOP = 0xFC,
    ACTIVESENSE = 0xFE,
    RESET = 0xFF,
  };

  enum masks_t {
    CHANNEL_MASK = 0x0F,
    STATUS_MASK = 0xF0,
  };

  bool is_realtime(status_type_t status);
  unsigned int packet_length(status_type_t status);

  class Input {
    public:
      //chan, num, val
      typedef std::function<void(uint8_t, uint8_t, uint8_t)> func3_t;
      //on, chan, num, val
      typedef std::function<void(bool, uint8_t, uint8_t, uint8_t)> funcNote_t;

      static unsigned int device_count() throw (std::runtime_error);
      static std::vector<std::string> device_list() throw (std::runtime_error);

      explicit Input(std::string name) throw (std::runtime_error);
      explicit Input(unsigned int index) throw (std::runtime_error);

      void process();

      //masks off status so you can read the first byte direct as channel if applicable
      //only one per status type, clear with nullptr
      void with_message3(status_type_t status, func3_t func) throw (std::runtime_error);
      void with_note(funcNote_t func);
    private:
      RtMidiIn mInput;
      std::map<uint8_t, func3_t> m3Funcs;
      funcNote_t mNoteFunc = nullptr;
  };
}

#endif
