/*
  ==============================================================================

    Visualizer.cpp
    Created: 28 Apr 2014 10:14:16am
    Author:  jon

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Visualizer.h"
#include <fftw3.h>
#include <math.h>

//==============================================================================
Visualizer::Visualizer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    //
    setOpaque(true);
    clear();
    startTimer(1000/75);
    fft = fftw_plan_dft_r2c_1d(1024, inputSamples, fftComplex, FFTW_MEASURE);
}

Visualizer::~Visualizer()
{    
    fftw_destroy_plan(fft);
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    activeInputChannels = device->getActiveInputChannels();
    bufferSize = 1024; //device->getCurrentBufferSizeSamples();
    numActiveChannels = activeInputChannels.countNumberOfSetBits();
    fs = device->getCurrentSampleRate();
}

void Visualizer::audioDeviceStopped()
{
}

void Visualizer::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                        float** outputChannelData, int numOutputChannels,
                                        int numSamples)
{
    // for each channel, analyze!
    for (int chan = 0; chan < numInputChannels; ++chan)
    {
        // copy input data into our fft sample buffer
        for (int i = 0; i < numSamples; ++i)
        {
            inputSamples[i] = (double) inputChannelData[chan][i];
        }

        // perform the FFT and fill the fftData Buffer with the magnitudes
        fftw_execute(fft);
        for (int i=0; i < 513; ++i)
        {
            const double magnitude = sqrt(pow(fftComplex[i][0],2.0f) + pow(fftComplex[i][1],2.0f));
            fftData[i] = magnitude;
        }
    }

    // We need to clear the output buffers before returning, in case they're full of junk..
    for (int j = 0; j < numOutputChannels; ++j)
        if (outputChannelData[j] != nullptr)
            zeromem (outputChannelData[j], sizeof (float) * (size_t) numSamples);
}

void Visualizer::paint (Graphics& g)
{

    g.fillAll (Colours::black);   // clear the background
    const float y = getHeight();

    RectangleList<float> waveform;

    for (int x = jmin (getWidth(), numElementsInArray(fftData)); --x >= 0;)
    {
        float freqIntensity = (float) fftData[x]*50.0f;
        waveform.addWithoutMerging (Rectangle<float> ((float) x, 0.0f, 1.0f, jmin(y, freqIntensity)));
    }

    g.setColour (Colours::lightgreen);
    g.fillRectList (waveform);
}

void Visualizer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

void Visualizer::clear()
{
}

void Visualizer::timerCallback()
{
    repaint();
}
