#include "RtMidi.h"
#include "midicpp.h"
#include <memory>

namespace midicpp {
  bool is_realtime(status_type_t status) {
    return status >= CLOCK;
  }

  unsigned int packet_length(status_type_t status) {
    switch(status) {
      case CC:
      case NOTE_ON:
      case NOTE_OFF:
      case AFTERTOUCH:
      case PITCHBEND:
        return 3;
      case PROGCHANGE:
      case CHANPRESSURE:
        return 2;
      case CLOCK:
      case TICK:
      case START:
      case CONTINUE:
      case STOP:
      case ACTIVESENSE:
      case RESET:
      case TUNEREQUEST:
        return 1;
      case SONGPOSITION:
        return 3;
      case TC_QUARTERFRAME:
      case SONGSELECT:
        return 2;
      case SYSEX_END:
      case SYSEX_BEGIN:
      default:
        return 0;
    }
  }

  unsigned int Input::device_count() throw (std::runtime_error) {
    try {
      RtMidiIn midi;
      return midi.getPortCount();
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return 0;
  }

  std::vector<std::string> Input::device_list() throw (std::runtime_error) {
    std::vector<std::string> list;
    try {
      RtMidiIn midi;
      for (int i = 0; i <  midi.getPortCount(); i++) {
        std::string n = midi.getPortName(i);
        list.push_back(n);
      }
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return list;
  }

  Input::Input(std::string name) throw (std::runtime_error) {
    for (unsigned int i = 0; i < mInput.getPortCount(); i++) {
      std::string n = mInput.getPortName(i);
      if (n == name) {
        open(i);
        return;
      }
    }
    throw std::runtime_error("midi::Input() couldn't find port with name: " +  name);
  }

  Input::Input(unsigned int index) throw (std::runtime_error) {
    open(index);
  }

  void Input::open(unsigned int index) throw (std::runtime_error) {
    try {
      mInput.openPort(index);
      //XXX ignoring sysex for now
      mInput.ignoreTypes(true, false, false);
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
  }

  void Input::process() {
    while (true) {
      std::vector<unsigned char> message;
      mInput.getMessage(&message);
      if (message.size() == 0)
        return;

      const uint8_t status = (message[0] & STATUS_MASK);
      const uint8_t chan = (message[0] & CHANNEL_MASK);
      switch (message.size()) {
        case 3:
          {
            auto it = m3Funcs.find(status);
            if (it != m3Funcs.end())
              it->second(chan, message[1], message[2]);
            if (mNoteFunc && (status == NOTE_ON || status == NOTE_OFF))
              mNoteFunc(status == NOTE_ON, chan, message[1], message[2]);
          }
          break;
        case 2:
          {
            auto it = m2Funcs.find(status);
            if (it != m2Funcs.end())
              it->second(chan, message[1]);
          }
          break;
        case 1:
          if (status == 0xF0) {
            status_type_t s = static_cast<status_type_t>(message[0]);
            if (mRealtimeFunc && is_realtime(s))
              mRealtimeFunc(s);
            auto it = m1Funcs.find(s);
            if (it != m1Funcs.end())
              it->second(s);
          }
          break;
        default:
          break;
      }
    }
  }

  void Input::with_message3(status_type_t status, func3_t func) throw (std::runtime_error) {
    if (packet_length(status) != 3) {
      char b[8];
      std::sprintf(b, "%02X", status);
      throw std::runtime_error("message with status " + std::string(b) + " is not a 3 byte message");
    }
    if (func)
      m3Funcs[status] = func;
    else
      m3Funcs.erase(status);
  }

  void Input::with_message2(status_type_t status, func2_t func) throw (std::runtime_error) {
    if (packet_length(status) != 2) {
      char b[8];
      std::sprintf(b, "%02X", status);
      throw std::runtime_error("message with status " + std::string(b) + " is not a 2 byte message");
    }
    if (func)
      m2Funcs[status] = func;
    else
      m2Funcs.erase(status);
  }

  void Input::with_message1(status_type_t status, func1_t func) throw (std::runtime_error) {
    if (packet_length(status) != 1) {
      char b[8];
      std::sprintf(b, "%02X", status);
      throw std::runtime_error("message with status " + std::string(b) + " is not a 1 byte message");
    }
    if (func)
      m1Funcs[status] = func;
    else
      m1Funcs.erase(status);
  }

  void Input::with_note(funcNote_t func) {
    mNoteFunc = func;
  }

  void Input::with_realtime(func1_t func) {
    mRealtimeFunc = func;
  }


  unsigned int Output::device_count() throw (std::runtime_error) {
    try {
      RtMidiOut midi;
      return midi.getPortCount();
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return 0;
  }

  std::vector<std::string> Output::device_list() throw (std::runtime_error) {
    std::vector<std::string> list;
    try {
      RtMidiOut midi;
      for (int i = 0; i <  midi.getPortCount(); i++) {
        std::string n = midi.getPortName(i);
        list.push_back(n);
      }
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return list;
  }

  Output::Output(std::string name) throw (std::runtime_error) {
    for (unsigned int i = 0; i < mOutput.getPortCount(); i++) {
      std::string n = mOutput.getPortName(i);
      if (n == name) {
        open(i);
        return;
      }
    }
    throw std::runtime_error("midi::Output() couldn't find port with name: " +  name);
  }

  Output::Output(unsigned int index) throw (std::runtime_error) {
    open(index);
  }

  void Output::open(unsigned int index) throw (std::runtime_error) {
    try {
      mOutput.openPort(index);
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
  }

  void Output::note(bool on, uint8_t chan, uint8_t num, uint8_t velocity) {
    std::vector<unsigned char> message = {
      (unsigned char)((on ? NOTE_ON : NOTE_OFF) | (chan & CHANNEL_MASK)),
      (unsigned char)(num > 127 ? 127 : num),
      (unsigned char)(velocity > 127 ? 127 : velocity)
    };
    
    mOutput.sendMessage(&message);
  }

  void Output::cc(uint8_t chan, uint8_t num, uint8_t value) {
    std::vector<unsigned char> message = {
      (unsigned char)(CC | (chan & CHANNEL_MASK)),
      (unsigned char)(num > 127 ? 127 : num),
      (unsigned char)(value > 127 ? 127 : value)
    };
    mOutput.sendMessage(&message);
  }

  void Output::nrpn(uint8_t chan, uint16_t num, uint16_t val) {
    if (num > 16383)
      num = 16383;
    if (val > 16383)
      val = 16383;

    cc(chan, 99, (num >> 7) & VAL_MASK);
    cc(chan, 98, num & VAL_MASK);
    cc(chan, 6 , (val >> 7) & VAL_MASK);
    cc(chan, 38, val & VAL_MASK);
  }
}

