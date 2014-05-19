#ifndef CUSTOM_JACK_DEVICE_H
#define CUSTOM_JACK_DEVICE_H

struct JackSessionCallbackArg {
  String session_directory;
  String session_uuid;
  String command_line;
  bool quit; 
};

struct JackClientConfiguration {
  String      clientName;
  // size of array = number of input channels. If the strings are empty, default names are chosen (in_1 , in_2 etc)
  StringArray inputChannels; 
  StringArray outputChannels;
  MidiBuffer *midi_events; // optional buffer where the jack midi events will get written to before each audio callback
  int *freewheel_flag; // optional flag toggled in freewheel mode
  String session_uuid;

  typedef void (*SessionCallback)(JackSessionCallbackArg &arg);
  SessionCallback session_callback;
};

// user supplied function for jack config
void getJackClientConfiguration(JackClientConfiguration &conf);

AudioIODeviceType* createAudioIODeviceType_JACK_Custom();

#endif // CUSTOM_JACK_DEVICE_H
