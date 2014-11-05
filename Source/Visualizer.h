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
    void changeSettings(const int numTracks_, const int numSpatialBins_, const float intensityScalingConstant_, const float intensityCutoffConstant_, const double timeDecayConstant_);
    void resized();
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped();
    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;

private:
    // data structures for masking model
    // turn these into arbitrary sized vectors
    std::vector<std::unique_ptr<loudness::TrackBank>> audioInputBankVector;
    std::vector<loudness::TrackBank const *> roexBankOutputVector;
    std::vector<loudness::TrackBank const *> partialLoudnessOutputVector;
    std::vector<std::unique_ptr<loudness::DynamicPartialLoudnessGM>> modelVector;
    int shouldPrint;

    // fft input arrays: real arrays containing audio samples
    RealVec fftInputL;
    RealVec fftInputR;

    // fft input arrays: real arrays containing audio samples
    RealVec targetL;
    RealVec targetR;
    RealVec maskerL;
    RealVec maskerR;

    // fft output with phase info removed
    RealVec fftMagnitudesL;
    RealVec fftMagnitudesR;

    // "settings" constants
    int numFreqBins;
    int numSpatialBins;
    int numTracks;
    float intensityScalingConstant;
    float intensityCutoffConstant;
    double timeDecayConstant;
    int freqMaskingFlag;
    int spatialMaskingFlag;

    // dummy convolution model
    double freqGaussian[5];
    double spatialGaussian[11];

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
    int calculateSpatialBin(const float magnitudeL, const float magnitudeR);
    Colour intensityToColour(const float intensity, const int track);
    void runMaskingModel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer)
};


#endif  // VISUALIZER_H_INCLUDED
