/*
  ==============================================================================

    Visualizer.h
    Created: 28 Apr 2014 10:14:16am
    Author:  jon

  ==============================================================================
*/

#ifndef VISUALIZER_H_INCLUDED
#define VISUALIZER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>
#include <loudness/Models/DynamicPartialLoudnessGM.h>

//==============================================================================
/*
*/

class Visualizer    : public Component,
                      public AudioIODeviceCallback,
                      private Timer
{
public:
    Visualizer();
    ~Visualizer();

    void paint (Graphics&);
    void changeSettings(const int numTracks_, const int numSpatialBins_, const float intensityScalingConstant_, const float intensityCutoffConstant_, const double timeDecayConstant_, const double maskingThreshold_, const bool detectionMode_);
    void resized();
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped();
    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;

private:
    // data structures for masking model
    // turn these into arbitrary sized vectors
    loudness::TrackBank *audioInputBank;
    const loudness::TrackBank *powerSpectrumOutput;
    const loudness::TrackBank *roexBankOutput;
    const loudness::TrackBank *partialLoudnessOutput;
    const loudness::TrackBank *integratedLoudnessOutput;
    loudness::DynamicPartialLoudnessGM *model;
    std::vector <std::vector <std::vector<double>> > output; // [track][freq][pos]
    std::vector<double> cutoffFreqs;

    // "settings" constants
    int numFreqBins;
    int numSpatialBins;
    int numTracks;
    float intensityScalingConstant;
    float intensityCutoffConstant;
    double timeDecayConstant;
    double maskingThreshold;
    bool detectionMode;

    // other buffers used in masking models
    double colBuffer[40];
    double rowBuffer[128];
    double freqConvBuffer[523]; // 523 + 11 - 1
    double spatialConvBuffer[138]; // 128 + 11 - 1
    
    // info about the audio device this component is recieving input from
    double fs;
    int bufferSize;
    int numActiveChannels;
    BigInteger activeInputChannels;

    void timerCallback() override;

    // useful helper functions
    Colour trackIntensityToColour(const float intensity, const int track);
    Colour maskerIntensityToColour(const float intensity, const int track);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer)
};


#endif  // VISUALIZER_H_INCLUDED
