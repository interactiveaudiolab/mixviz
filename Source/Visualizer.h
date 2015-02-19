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
#include "TrackComponents/TrackSelector.h"
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
    void setIntensityScalingConstant(const float intensityScalingConstant_);
    void setIntensityCutoffConstant(const float intensityCutoffConstant_);
    void setTimeDecayConstant(const double timeDecayConstant_);
    void setMaskingTimeDecayConstant(const double maskingTimeDecayConstant_);
    void setMaskingThreshold(const double maskingThreshold_);
    void updateTracksInGroup(int groupIndex, Array<int> tracksInGroup);
    void resized();
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped();
    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;
    void printMe();

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
    int nFreqBins;
    int nSpatialBins;
    int nTrackGroups;
    float intensityScalingConstant;
    float intensityCutoffConstant;
    double timeDecayConstant;
    double maskingTimeDecayConstant;
    double maskingThreshold;
    bool detectionMode;

    // an array when index is the group index and 
    // values in the array at that index represent io port numbers of tracks in the group
    Array<Array<int>> trackGroups;
    
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
