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
    // fft input arraysreal arrays containing samples
    double fftInputL[1024];
    double fftInputR[1024];
    double fftInputStereo[1024];

    // fft output arrays containing complex numbers
    fftw_complex fftOutputL[513];
    fftw_complex fftOutputR[513];
    fftw_complex fftOutputStereo[513];

    // magnitudes version of the fft output arrays
    double fftMagnitudesL[513];
    double fftMagnitudesR[513];
    double fftMagnitudesStereo[513];

    // fft plans
    fftw_plan fftL;
    fftw_plan fftR;
    fftw_plan fftStereo;
    
    // masking model inputs and outputs
    double maskingInputs[128][513];
    double maskingOutputs[128][513];
    
    double fs;
    int bufferSize;
    int numActiveChannels;
    BigInteger activeInputChannels;

    void clear();
    void timerCallback() override;

    // useful helper functions
    int calculateSpatialBin(const float magnitudeL, const float magnitudeR);
    int calculateFreqBin(const int freq);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer)
};


#endif  // VISUALIZER_H_INCLUDED
