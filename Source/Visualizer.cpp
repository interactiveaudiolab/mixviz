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
    setOpaque(true);
    startTimer(1000/30);

    // initialize fft plans
    fftL = fftw_plan_dft_r2c_1d(1024, fftInputL, fftOutputL, FFTW_MEASURE);
    fftR = fftw_plan_dft_r2c_1d(1024, fftInputR, fftOutputR, FFTW_MEASURE);
    fftStereo = fftw_plan_dft_r2c_1d(1024, fftInputStereo, fftOutputStereo, FFTW_MEASURE);
}

Visualizer::~Visualizer()
{    
    fftw_destroy_plan(fftL);
    fftw_destroy_plan(fftR);
    fftw_destroy_plan(fftStereo);
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    activeInputChannels = device->getActiveInputChannels();
    bufferSize = 1024; //device->getCurrentBufferSizeSamples();
    numActiveChannels = activeInputChannels.countNumberOfSetBits();
    clear();
    fs = device->getCurrentSampleRate();
}

void Visualizer::audioDeviceStopped()
{
}


// this function happens each time audio data is recieved
void Visualizer::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                        float** outputChannelData, int numOutputChannels,
                                        int numSamples)
{
    // clear the input and output buffers
    clear();

    // for each stereo track, analyze!
    for (int track = 0; track < numInputChannels/2; ++track)
    {
        // copy input L and R channel data into our input sample buffer
        for (int i = 0; i < numSamples; ++i)
        {
            fftInputL[i] = (double) inputChannelData[2*track][i];
            fftInputR[i] = (double) inputChannelData[2*track+1][i];
            fftInputStereo[i]=fftInputL[i]+fftInputR[i];
        }

        // perform all the FFTs and calculate magnitudes, spatial/frequency positions
        fftw_execute(fftL);
        fftw_execute(fftR);
        fftw_execute(fftStereo);
        for (int freq=0; freq < 513; ++freq)
        {
            // calculate magnitudes
            const double magnitudeL = sqrt(pow(fftOutputL[freq][0],2.0f) + pow(fftOutputL[freq][1],2.0f));
            const double magnitudeR = sqrt(pow(fftOutputR[freq][0],2.0f) + pow(fftOutputR[freq][1],2.0f));
            const double magnitudeStereo = sqrt(pow(fftOutputStereo[freq][0],2.0f) + pow(fftOutputStereo[freq][1],2.0f));

            // update buffers
            fftMagnitudesL[freq] = magnitudeL;
            fftMagnitudesR[freq] = magnitudeR;
            fftMagnitudesStereo[freq] = magnitudeStereo;

            // calculate spatial/frequency positions and update buffer
            const int freqBin = calculateFreqBin(freq);
            const int spatialBin = calculateSpatialBin(magnitudeL,magnitudeR);
            maskingInputs[spatialBin][freqBin] = magnitudeStereo;
        }
    }

    // We need to clear the output buffers before returning, in case they're full of junk..
    for (int j = 0; j < numOutputChannels; ++j)
        if (outputChannelData[j] != nullptr)
            zeromem (outputChannelData[j], sizeof (float) * (size_t) numSamples);
}

// this function is called at each timer callback,
void Visualizer::paint (Graphics& g)
{

    g.fillAll (Colours::black);   // clear the background
    const float height = (float) getHeight();
    const float width = (float) getWidth();
    const float maxXIndex = 128.0f;
    const float maxYIndex = 513.0f;

    RectangleList<float> waveform;

    for (int x = 0; x < (int)maxXIndex; ++x)
    {
        for (int y = 0; y < (int)maxYIndex; ++y)
        {
            if (maskingInputs[x][y] > 0) 
            {
                const float xf = (float) x;
                const float yf = (float) y;
                waveform.addWithoutMerging (Rectangle<float> (xf/maxXIndex*width, (maxYIndex-yf)/maxYIndex*height, 1.0f, 1.0f));
            }
        }
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
    for (int x = 0; x < 128; ++x)
    {
        for (int y = 0; y < 513; ++y)
        {
            maskingInputs[x][y]=0;
            maskingOutputs[x][y]=0;
        }
    }
}

void Visualizer::timerCallback()
{
    repaint();
}

int Visualizer::calculateSpatialBin(const float magnitudeL, const float magnitudeR)
{
    // calculate ratios between the two channels
    if (magnitudeL == 0)
    {
        // signal is all the way on the right 
        return 127;
    }
    else if (magnitudeR == 0)
    {
        // signal is all the way on the left
        return 0;
    }
    else if (magnitudeL > magnitudeR)
    {
        // signal is partially on the left
        return ((int) ((magnitudeR/magnitudeL)*64));
    }
    else
    {
        // signal is partially on the right
        return ((int) ((1-(magnitudeL/magnitudeR))*64)+63);
    }
}

int Visualizer::calculateFreqBin(const int freq)
{
    return freq;
}
