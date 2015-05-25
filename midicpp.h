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
    VAL_MASK = 0x7F,
    STATUS_MASK = 0xF0,
  };

  bool is_realtime(status_type_t status);
  unsigned int packet_length(status_type_t status);

  class Input {
    public:
      //chan, num, val
      typedef std::function<void(uint8_t, uint8_t, uint8_t)> func3_t;
      typedef std::function<void(uint8_t, uint8_t)> func2_t;
      typedef std::function<void(status_type_t)> func1_t;
      //on, chan, num, velocity
      typedef std::function<void(bool on, uint8_t chan, uint8_t number, uint8_t velocity)> funcNote_t;

      static unsigned int device_count() throw (std::runtime_error);
      static std::vector<std::string> device_list() throw (std::runtime_error);

      explicit Input(std::string name) throw (std::runtime_error);
      explicit Input(unsigned int index) throw (std::runtime_error);

      //processes [non blocking] input messages and executes callbacks
      void process();

      //masks off status so you can read the first byte direct as channel if applicable
      //only one per status type, clear with nullptr
      void with_message3(status_type_t status, func3_t func) throw (std::runtime_error);
      void with_message2(status_type_t status, func2_t func) throw (std::runtime_error);
      void with_message1(status_type_t status, func1_t func) throw (std::runtime_error);
      void with_note(funcNote_t func);
      void with_realtime(func1_t func);
    private:
      void open(unsigned int index) throw (std::runtime_error);
      RtMidiIn mInput;
      std::map<uint8_t, func3_t> m3Funcs;
      std::map<uint8_t, func2_t> m2Funcs;
      std::map<uint8_t, func1_t> m1Funcs;
      func1_t mRealtimeFunc;
      funcNote_t mNoteFunc = nullptr;
  };

  class Output {
    public:
      static unsigned int device_count() throw (std::runtime_error);
      static std::vector<std::string> device_list() throw (std::runtime_error);

      explicit Output(std::string name) throw (std::runtime_error);
      explicit Output(unsigned int index) throw (std::runtime_error);

      void note(bool on, uint8_t chan, uint8_t num, uint8_t velocity);
      void cc(uint8_t chan, uint8_t num, uint8_t value);
      void nrpn(uint8_t chan, uint16_t num, uint16_t val);
    private:
      void open(unsigned int index) throw (std::runtime_error);
      RtMidiOut mOutput;
  };
}

#endif
