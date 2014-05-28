#include "jack/jack.h"
#include "jack/midiport.h"
#include "jack/transport.h"
#include "jack/session.h"

#include "../JuceLibraryCode/JuceHeader.h"
#include "custom_jack_device.h"

extern "C" int libjack_is_present;
extern "C" int libjack_session_is_supported;

static const char* jackAudioDeviceLocalName(bool autoconnect) {
    if (!autoconnect) return "Auto-connect OFF";
    else return "Auto-connect ON";
}

int display_jack_errors_in_message_box = 0;

//==============================================================================
class JackAudioIODevice   : public AudioIODevice,
                            public MessageListener
{
    bool autoconnect;
    JackClientConfiguration config;
public:
    JackAudioIODevice (JackClientConfiguration &config_, bool autoconnect_)
      : AudioIODevice (jackAudioDeviceLocalName(autoconnect_), "JACK"),
          autoconnect(autoconnect_),
          config(config_),
          isOpen_ (false),
          isStarted (false),
          callback (0),
        client (0),
        client_activated(false)
    {
        midi_in = 0;
        for (int i=0; i < config.inputChannels.size(); ++i) {
            if (config.inputChannels[i].length() == 0) config.inputChannels.set(i, "in_"+String(i+1));
        }
        for (int i = 0; i < config.outputChannels.size(); i++) {
            if (config.outputChannels[i].length() == 0) config.outputChannels.set(i, "out_"+String(i+1));
        }
        display_jack_errors_in_message_box = 0; // these messages sucks..
        jack_set_error_function (JackAudioIODevice::errorCallback);
        jack_status_t status;
        if (config.session_uuid.isNotEmpty() && libjack_session_is_supported) {
          //cerr << "JackAudioIODevice: opening with session_uuid: '" << config.session_uuid << "'\n";
          client = jack_client_open (config.clientName.toUTF8().getAddress(), JackSessionID, &status, config.session_uuid.toUTF8().getAddress());
        } else {
          //cerr << "JackAudioIODevice: opening WITHOUT session_uuid: '" << config.session_uuid << "'\n";          
          client = jack_client_open (config.clientName.toUTF8().getAddress(), JackNoStartServer, &status);
        }
        display_jack_errors_in_message_box = 0;
        if (client == 0)
        {
            if ((status & JackServerFailed) || (status & JackServerError))
                printf ("Unable to connect to JACK server\n");
            else if ((status & JackVersionError))
                printf ("Client's protocol version does not match\n");
            else if ((status & JackInvalidOption))
                printf ("The operation contained an invalid or unsupported option\n");
            else if ((status & JackNameNotUnique))
                printf ("The desired client name was not unique\n");
            else if ((status & JackNoSuchClient))
                printf ("Requested client does not exist\n");
            else if ((status & JackInitFailure))
                printf ("Unable to initialize client\n");
            else printf ("Unknown jack error [%d]\n", (int)status);
        }
        else
        {

            for (int i=0; i < config.inputChannels.size(); ++i) {
                jack_port_t* input =
                    jack_port_register (client, config.inputChannels[i].toUTF8().getAddress(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
                inputPorts.add (input);
            }

            for (int i = 0; i < config.outputChannels.size(); i++) {
                jack_port_t* output =
                    jack_port_register (client, config.outputChannels[i].toUTF8().getAddress(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
                outputPorts.add (output);
           }
           if (config.midi_events) 
              midi_in = jack_port_register(client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

           if (libjack_session_is_supported && config.session_callback) {
             jack_set_session_callback(client, sessionCallback, this);
           }
        }
        inChans  = (float**)malloc(config.inputChannels.size() *sizeof(float*));
        outChans = (float**)malloc(config.outputChannels.size()*sizeof(float*));
    }

    ~JackAudioIODevice()
    {
        if (client)
        {
            close ();

            jack_client_close (client);
            client = 0;
        }
        free(inChans);
        free(outChans);
    }

    bool autoConnect() const { return autoconnect; }

    StringArray getOutputChannelNames() { return config.outputChannels; }

    StringArray getInputChannelNames() { return config.inputChannels; }

    int getNumSampleRates()
    {
        return client ? 1 : 0;
    }

    double getSampleRate (int /*index*/)
    {
        return client ? jack_get_sample_rate (client) : 0;
    }

    int getNumBufferSizesAvailable()
    {
        return client ? 1 : 0;
    }

    int getBufferSizeSamples (int /*index*/)
    {
        return client ? jack_get_buffer_size (client) : 0;
    }

    int getDefaultBufferSize()
    {
        return client ? jack_get_buffer_size (client) : 0;
    }

    Array<double> getAvailableSampleRates()
    {
        Array<double> arr = Array<double>();
        return arr;
    }

    Array<int> getAvailableBufferSizes()
    {
        Array<int> arr = Array<int>();
        return arr;
    }

    String open (const BitArray& /*inputChannels*/,
                 const BitArray& /*outputChannels*/,
                 double /*sampleRate*/,
                 int /*bufferSizeSamples*/)
    {
        if (! client)
        {
            return "Jack server is not running";
        }

        close();

        // activate client !        
        jack_set_process_callback (client, JackAudioIODevice::processCallback, this);
	jack_set_freewheel_callback (client, JackAudioIODevice::freewheelCallback, this);
        jack_on_shutdown (client, JackAudioIODevice::shutdownCallback, this);

        jack_activate (client); client_activated = true;

        if (autoconnect) {
            const char **ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
            for (int i=0; i < 2 && i < outputPorts.size() && ports[i]; ++i) {
                jack_connect(client, jack_port_name((jack_port_t*)outputPorts[i]), ports[i]);
            }
        }

        isOpen_ = true;

        return String::empty;
    }

    void close()
    {
        stop();

        if (client && client_activated)
        {
          jack_deactivate (client); client_activated = false;

            jack_set_process_callback (client, JackAudioIODevice::processCallback, 0);
            jack_on_shutdown (client, JackAudioIODevice::shutdownCallback, 0);
        }
        if (config.freewheel_flag) *config.freewheel_flag = -1;
        isOpen_ = false;
    }

    bool isOpen()
    {
        return isOpen_;
    }

    int getCurrentBufferSizeSamples()
    {
        return getBufferSizeSamples (0);
    }

    double getCurrentSampleRate()
    {
        return getSampleRate (0);
    }

    int getCurrentBitDepth()
    {
        return 32;
    }

    BitArray getActiveOutputChannels() const
    {
        BitArray outputBits;
        outputBits.setRange(0, outputPorts.size(), true);
        return outputBits;
    }

    BitArray getActiveInputChannels() const
    {
        BitArray inputBits;
        inputBits.setRange(0, inputPorts.size(), true);
        return inputBits;
    }

    int getOutputLatencyInSamples()
    {
        int latency = 0;
        
        for (int i = 0; i < outputPorts.size(); i++)
            latency = jmax (latency, (int) jack_port_get_total_latency (client, (jack_port_t*) outputPorts [i]));
    
        return latency;
    }

    int getInputLatencyInSamples()
    {
        int latency = 0;
        
        for (int i = 0; i < inputPorts.size(); i++)
            latency = jmax (latency, (int) jack_port_get_total_latency (client, (jack_port_t*) inputPorts [i]));
    
        return latency;
    }

    void start (AudioIODeviceCallback* callback_)
    {
        if (! isOpen_)
            callback_ = 0;

        callback = callback_;

        if (callback != 0)
            callback->audioDeviceAboutToStart (this);

        isStarted = (callback != 0);
    }

    void freewheel (int starting ) {
        if (config.freewheel_flag) *config.freewheel_flag = starting; 
    }

    void process (int numSamples)
    {
      if (config.midi_events) {
        void* buf = jack_port_get_buffer(midi_in, numSamples);
        jack_nframes_t event_count = jack_midi_get_event_count(buf);
        jack_midi_event_t in_event;
        config.midi_events->clear();
        for (jack_nframes_t i=0; i < event_count; ++i) {
          jack_midi_event_get(&in_event, buf, i);
          //cerr << "add event : "<< (void*)*(const uint8_t*)in_event.buffer << ", sz=" << in_event.size << " sample: " << in_event.time << "\n";
          config.midi_events->addEvent((const uint8_t*)in_event.buffer, in_event.size, in_event.time);
        }
      }

        int i, numActiveInChans = 0, numActiveOutChans = 0;

        for (i = 0; i < inputPorts.size(); ++i)
        {
            jack_default_audio_sample_t *in =
                (jack_default_audio_sample_t *) jack_port_get_buffer (
                                                        (jack_port_t*) inputPorts.getUnchecked(i), numSamples);
            jassert (in != 0);
            inChans [numActiveInChans++] = (float*) in;
        }

        for (i = 0; i < outputPorts.size(); ++i)
        {
            jack_default_audio_sample_t *out =
                (jack_default_audio_sample_t *) jack_port_get_buffer (
                                                        (jack_port_t*) outputPorts.getUnchecked(i), numSamples);
            jassert (out != 0);
            outChans [numActiveOutChans++] = (float*) out;
        }

        if (callback != 0)
        {
            callback->audioDeviceIOCallback ((const float**) inChans,
                                             inputPorts.size(), 
                                             outChans,
                                             outputPorts.size(),
                                             numSamples);
        }
        else
        {
            for (i = 0; i < outputPorts.size(); ++i)
                zeromem (outChans[i], sizeof (float) * numSamples);
        }
    }

    void stop()
    {
        AudioIODeviceCallback* const oldCallback = callback;

        start (0);

        if (oldCallback != 0)
            oldCallback->audioDeviceStopped();
    }

    bool isPlaying()
    {
        return isStarted;
    }

    String getLastError()
    {
        return String::empty;
    }

    String inputId, outputId;

private:

    static void threadInitCallback (void* /*callbackArgument*/)
    {
    }
    
    static void shutdownCallback (void* callbackArgument)
    {
        JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;
    
        if (device) 
        {
            device->client = 0;
            device->close ();
        }
    }
    
    static int processCallback (jack_nframes_t nframes, void* callbackArgument)
    {
        JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;

        if (device)
            device->process (nframes);

        return 0;
    }

    struct SessionCallbackMessage : public Message {
      jack_session_event_t *event;
    };

    void handleMessage(const Message &msg) {
      const SessionCallbackMessage *sm;
      //printf("sessionCallback, received message\n");
      if ((sm = dynamic_cast<const SessionCallbackMessage*>(&msg))) {
        if (config.session_callback) {
          JackSessionCallbackArg arg;
          arg.session_directory = sm->event->session_dir;
          arg.session_uuid = sm->event->client_uuid;
          arg.quit = (sm->event->type == JackSessionSaveAndQuit);
          config.session_callback(arg);

          sm->event->command_line = strdup(arg.command_line.toUTF8().getAddress());
        }
        jack_session_reply(client, sm->event);
        jack_session_event_free(sm->event); 
      }
    }

    static void sessionCallback (jack_session_event_t *event, void *callbackArgument) 
    { 
      //printf("sessionCallback, posting message\n");
      JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;
      SessionCallbackMessage *m = new SessionCallbackMessage; m->event = event;
      device->postMessage(m);
    }

    static void freewheelCallback (int starting, void *callbackArgument)
    {
        JackAudioIODevice* device = (JackAudioIODevice*) callbackArgument;
	if (device) {
       	    device->freewheel (starting);
	}
    }

    static void errorCallback (const char *msg)
    {
        char errmsg[1024]; 
        const char *extra_msg = "";
        if (strcmp(msg, "Only external clients need attach port segments")==0) {
            extra_msg = "\nThis probably means that you are trying to connect a 32-bit jack client to a 64-bit server -- you need to make sure that you are using a recent version of jack (at least 0.116)";
        }
        snprintf(errmsg, 1024, "Jack error: %s%s", msg, extra_msg);
        fprintf (stderr, "%s\n", errmsg);
        if (display_jack_errors_in_message_box) {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "JACK error", errmsg);
            display_jack_errors_in_message_box = 0; // only once
        }
    }

    bool isOpen_, isStarted;

    AudioIODeviceCallback* callback;
    
    float** inChans;
    float** outChans;

    jack_client_t *client;
    bool client_activated;
    Array<void*> inputPorts;
    Array<void*> outputPorts;
    jack_port_t *midi_in;
};


//==============================================================================
class JackAudioIODeviceType  : public AudioIODeviceType
{
public:
    //==============================================================================
    JackAudioIODeviceType()
        : AudioIODeviceType("JACK")
    {
    }

    ~JackAudioIODeviceType()
    {
    }

    

    //==============================================================================
    void scanForDevices()
    {
    }

    StringArray getDeviceNames (const bool /*wantInputNames*/) const
    {
        StringArray s; 
        s.add(jackAudioDeviceLocalName(true)); // autoconnect is first
        s.add(jackAudioDeviceLocalName(false));
        return s;
    }

    int getDefaultDeviceIndex (const bool /*forInput*/) const
    {
        return 0; // autoconnect is default
    }

    bool hasSeparateInputsAndOutputs() const    { return false; }

    int getIndexOfDevice (AudioIODevice* device, const bool /*asInput*/) const
    {
        JackAudioIODevice* const d = dynamic_cast <JackAudioIODevice*> (device);
        if (d == 0) return -1;
        return d->autoConnect() ? 0 : 1;
    }

    AudioIODevice* createDevice (const String& outputDeviceName,
                                 const String& /*inputDeviceName*/)
    {
        bool autoconnect = (outputDeviceName == jackAudioDeviceLocalName(true));
        JackClientConfiguration config;
        getJackClientConfiguration(config);
        return new JackAudioIODevice(config, autoconnect);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JackAudioIODeviceType (const JackAudioIODeviceType&);
    const JackAudioIODeviceType& operator= (const JackAudioIODeviceType&);
};

AudioIODeviceType* createAudioIODeviceType_JACK_Custom()
{
    /* detect if libjack.so is available using relaytool on linux */
    if (!libjack_is_present) return 0;
    else return new JackAudioIODeviceType();
}

void getJackClientConfiguration(JackClientConfiguration &conf) {
    conf.clientName = "MixVisJack";
    String channels[16] = { "","","","",
                            "","","","",
                            "","","","",
                            "","","","" };
    conf.inputChannels = StringArray(channels,8);
    conf.outputChannels = StringArray(channels,8);
    conf.midi_events = nullptr;
    conf.session_uuid = "nothing";
}

