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
#include <stdio.h>

using namespace std;

//==============================================================================
Visualizer::Visualizer()
    :   numSpatialBins(128),
        numFreqBins(513)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setOpaque(true);
    startTimer(1000/30);

    // initialize fft plans
    fftL = fftw_plan_dft_r2c_1d(1024, fftInputL, fftOutputL, FFTW_MEASURE);
    fftR = fftw_plan_dft_r2c_1d(1024, fftInputR, fftOutputR, FFTW_MEASURE);
    fftStereo = fftw_plan_dft_r2c_1d(1024, fftInputStereo, fftOutputStereo, FFTW_MEASURE);

    //initialize gaussain
    for (int i = -5; i < 6; ++i)
    {
        gaussian[i+5] = exp(pow((float)i,2.0f)/-2.0f);
    } 
}

Visualizer::~Visualizer()
{    
    fftw_destroy_plan(fftL);
    fftw_destroy_plan(fftR);
    fftw_destroy_plan(fftStereo);
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    // get info about the audio device we are connected to
    activeInputChannels = device->getActiveInputChannels();
    bufferSize = 1024; //device->getCurrentBufferSizeSamples();
    numActiveChannels = activeInputChannels.countNumberOfSetBits();
    fs = device->getCurrentSampleRate();

    clearMaskingInput();
    clearMaskingOutput();
}

void Visualizer::audioDeviceStopped()
{
}

// this function happens each time audio data is recieved
void Visualizer::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                        float** outputChannelData, int numOutputChannels,
                                        int numSamples)
{
    // clear the input buffer
    clearMaskingInput();

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
            maskingInput[spatialBin][freqBin] = magnitudeStereo;
        }
    }

    runMaskingModel();
    
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
    const float maxXIndex = (float) numSpatialBins;
    const float maxYIndex = (float) numFreqBins;

    for (int x = 0; x < numSpatialBins; ++x)
    {
        for (int y = 0; y < numFreqBins; ++y)
        {
            const float intensity = (float) maskingOutput[x][y];
            //cout << "x: " << x << "\t\ty: " << y << "\t\t i: " << intensity << endl; 
            if (intensity > 0) 
            {
                const float xf = (float) x;
                const float yf = (float) y;
                const Colour colour = Colour(0.5f, intensity*200.0f, 1.0f, 1.0f);
                g.setColour(colour);
                g.fillRect(Rectangle<float>(xf/maxXIndex*width, (maxYIndex-yf)/maxYIndex*height, 2.0f, 2.0f));
            }
        }
    }
}

void Visualizer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

void Visualizer::clearMaskingInput()
{
    for (int loc = 0; loc < numSpatialBins; ++loc)
    {
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            maskingInput[loc][freq] = 0;
        }
    }
}

void Visualizer::clearMaskingOutput()
{
    for (int loc = 0; loc < numSpatialBins; ++loc)
    {
        for (int freq = 0; freq < numFreqBins; ++freq)
        {   
            maskingOutput[loc][freq]=0;
        }
    }
}
    

void Visualizer::timerCallback()
{
    repaint();
}

// given the magnitude of left and right channels, this function returns the correct spatial bin
int Visualizer::calculateSpatialBin(const float magnitudeL, const float magnitudeR)
{
    // calculate ratios between the two channels
    if (magnitudeL == 0)
    {
        // signal is all the way on the right 
        return numSpatialBins-1;
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

// this function returns the correct ERB frequency bin given a fft bin number
int Visualizer::calculateFreqBin(const int freq)
{
    return freq;
}

// executes the masking model on inputs
void Visualizer::runMaskingModel()
{
    // run each masking model
    calculateFreqMasking();
    calculateSpatialMasking();

    // place the output into the output buffer
    for (int loc = 0; loc < numSpatialBins; ++loc)
    {
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            maskingOutput[loc][freq] = maskingInput[loc][freq];
        }
    }
}

void Visualizer::calculateFreqMasking()
{
    // calculate the masking for each column
    for (int col = 0; col < numSpatialBins; ++col)
    {
        // put the column in a buffer
        for (int row = 0; row < numFreqBins; ++row)
        {
            // if this position is greater than 0, perform convolution
            if (maskingInput[col][row] > 0)
            {
                for (int idx = 0; idx < 11; ++idx)
                {
                    const int j = row-5+idx;
                    if (j > 0 && j < numFreqBins)
                        colBuffer[j] += gaussian[idx]*maskingInput[col][j];
                }
            }
        }
        
        // place buffer back into the column and clear buffer
        for (int row = 0; row < numFreqBins; ++row)
        {
            maskingInput[col][row] = colBuffer[row];
            colBuffer[row]=0;
        }
    }
}

void Visualizer::calculateSpatialMasking()
{
    for (int row = 0; row < numFreqBins; ++row)
    {
        // put the row in a buffer
        for (int col = 0; col < numSpatialBins; ++col)
            rowBuffer[col] = maskingInput[col][row];
        
        // call convolution function on the buffer

        // place buffer back into the row
        for (int col = 0; col < numSpatialBins; ++col)
            maskingInput[col][row] = rowBuffer[col];
    }
}
