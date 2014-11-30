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
      RtMidiIn midiin;
      return midiin.getPortCount();
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return 0;
  }

  std::vector<std::string> Input::device_list() throw (std::runtime_error) {
    std::vector<std::string> list;
    try {
      RtMidiIn midiin;
      for (int i = 0; i <  midiin.getPortCount(); i++) {
        std::string n = midiin.getPortName(i);
        list.push_back(n);
      }
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    return list;
  }

  Input::Input(std::string name) throw (std::runtime_error) {
    try {
      for (unsigned int i = 0; i < mInput.getPortCount(); i++) {
        std::string n = mInput.getPortName(i);
        if (n == name) {
          mInput.openPort(i);
          return;
        }
      }
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
    throw std::runtime_error("midi::Input() couldn't find port with name: " +  name);
  }

  Input::Input(unsigned int index) throw (std::runtime_error) {
    try {
      mInput.openPort(index);
    } catch (RtError &error) {
      throw std::runtime_error(error.what());
    }
  }

  void Input::process() {
    do {
      std::vector<unsigned char> message;
      mInput.getMessage(&message);
      if (message.size() == 0)
        return;

      if (message.size() == 3) {
        uint8_t status = (message[0] & STATUS_MASK);
        uint8_t chan = (message[0] & CHANNEL_MASK);
        auto it = m3Funcs.find(status);
        if (it != m3Funcs.end())
          it->second(chan, message[1], message[2]);
        if (mNoteFunc && (status == NOTE_ON || status == NOTE_OFF))
          mNoteFunc(status == NOTE_ON, chan, message[1], message[2]);
      }
    } while (true);
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

  void Input::with_note(funcNote_t func) {
    mNoteFunc = func;
  }
}

