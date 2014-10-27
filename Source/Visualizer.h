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
    void changeSettings(const int tracks, const int spatialBins, const int freqBins, const float intensityScaling, const float intensityCutoff, const double timeDecay, const int freqFlag, const int spatialFlag);
    void resized();
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped();
    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;

private:
    // fft input arrays: real arrays containing audio samples
    double targetL[1024];
    double targetR[1024];
    double maskerL[1024];
    double maskerR[1024];

    // fft output arrays containing complex numbers
    fftw_complex fftOutputL[513];
    fftw_complex fftOutputR[513];

    // fft output with phase info removed
    double fftMagnitudesL[513];
    double fftMagnitudesR[513];

    // "settings" constants
    int numSpatialBins;
    int numFreqBins;
    int numTracks;
    float intensityScalingConstant;
    float intensityCutoffConstant;
    double timeDecayConstant;
    int freqMaskingFlag;
    int spatialMaskingFlag;

    // fft plans
    fftw_plan fftL;
    fftw_plan fftR;
    fftw_plan fftStereo;
    
    // masking model input and output buffers
    double maskingInput[4][128][40];
    double prevMaskingInput[4][128][40];
    double maskingOutput[4][128][40];

    // dummy convolution model
    double freqGaussian[5];
    double spatialGaussian[11];

    // functions to clear masking model input and output buffers
    void clearMaskingInput();
    void clearMaskingOutput();

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
