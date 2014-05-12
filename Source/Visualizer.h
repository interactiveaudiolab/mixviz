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
    void resized();
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped();
    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;

private:
    // fft input arrays: real arrays containing audio samples
    double fftInputL[1024];
    double fftInputR[1024];
    double fftInputStereo[1024];

    // fft output arrays containing complex numbers
    fftw_complex fftOutputL[513];
    fftw_complex fftOutputR[513];
    fftw_complex fftOutputStereo[513];

    // temporary constants
    const int numSpatialBins;
    const int numFreqBins;

    // fft plans
    fftw_plan fftL;
    fftw_plan fftR;
    fftw_plan fftStereo;
    
    // masking model input and output buffers
    double maskingInput[128][513];
    double prevMaskingInput[128][513];
    double maskingOutput[128][513];

    // dummy convolution model
    double freqGaussian[11];
    double spatialGaussian[11];

    // functions to clear masking model input and output buffers
    void clearMaskingInput();
    void clearMaskingOutput();

    // other buffers used in masking models
    double colBuffer[513];
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
    int calculateFreqBin(const int freq);
    void runMaskingModel();
    void calculateFreqMasking();
    void calculateSpatialMasking();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer)
};


#endif  // VISUALIZER_H_INCLUDED
