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
#include <cmath>
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

//==============================================================================
Visualizer::Visualizer()
    :   numSpatialBins(128),
        numFreqBins(40)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setOpaque(true);
    startTimer(1000/30);

    // initialize fft plans
    fftL = fftw_plan_dft_r2c_1d(1024, fftInputL, fftOutputL, FFTW_MEASURE);
    fftR = fftw_plan_dft_r2c_1d(1024, fftInputR, fftOutputR, FFTW_MEASURE);
    fftStereo = fftw_plan_dft_r2c_1d(1024, fftInputStereo, fftOutputStereo, FFTW_MEASURE);

    //initialize gaussians
    for (int i = -5; i < 6; ++i)
        spatialGaussian[i+5] = exp(pow((float)i,2.0f)/-8.0f);
    for (int i = -2; i < 3; ++i)
        freqGaussian[i+2] = exp(pow((float)i,2.0f)/-8.0f);
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

    makeGammatoneFilters();
    clearMaskingInput();
    for (int row = 0; row < numFreqBins; ++row)
        colBuffer[row] = 0;
    for (int col = 0; col < numSpatialBins; ++col)
        rowBuffer[col] = 0;
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
            //fftInputStereo[i]=fftInputL[i]+fftInputR[i];
        }

        // perform all the FFTs and calculate magnitudes
        fftw_execute(fftL);
        fftw_execute(fftR);
        //fftw_execute(fftStereo);
        for (int freq=0; freq < 513; ++freq)
        {
            // calculate magnitudes
            fftMagnitudesL[freq] = sqrt(pow(fftOutputL[freq][0],2.0f) + pow(fftOutputL[freq][1],2.0f));
            fftMagnitudesR[freq] = sqrt(pow(fftOutputR[freq][0],2.0f) + pow(fftOutputR[freq][1],2.0f));
        }
        
        // perform matrix multiply with magnitudes to get gammatone bins
        const int m = numFreqBins;
        const int c = 513;
        const int n = 1;
        for (int i = 0; i < m; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                filterOutputL[i] = 0;
                filterOutputR[i] = 0;
                for (int k = 0; k < c; ++k)
                {
                    const double filterVal = gammatoneFilter[i][k];
                    filterOutputL[i] += filterVal * fftMagnitudesL[k];
                    filterOutputR[i] += filterVal * fftMagnitudesR[k];
                }
            }
        }

        // calculate spatial positions and update masking input
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            const double magnitudeL = filterOutputL[freq];
            const double magnitudeR = filterOutputR[freq];
            const double magnitudeStereo = magnitudeL + magnitudeR;

            const int spatialBin = calculateSpatialBin(magnitudeL,magnitudeR);
            maskingInput[spatialBin][freq] = magnitudeStereo;
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
    const float winHeight = (float) getHeight();
    const float winWidth = (float) getWidth();
    const float maxXIndex = (float) numSpatialBins;
    const float maxYIndex = (float) numFreqBins;
    const float binHeight = winHeight / maxYIndex;
    const float binWidth = winWidth / maxXIndex;
    const float yOffset = binHeight / 2.0f;
    const float xOffset = binWidth / 2.0f;

    for (int x = 0; x < numSpatialBins; ++x)
    {
        for (int y = 0; y < numFreqBins; ++y)
        {
            const float intensity = (float) maskingOutput[x][y];
            if (intensity > 0.0f) 
            {

                //cout << "x: " << x << "\t\ty: " << y << "\t\t i: " << intensity << endl; 
                const float xf = (float) x;
                const float yf = (float) y;
                const Colour colour = Colour(0.5f, intensity, 1.0f, 1.0f);
                g.setColour(colour);
                g.fillRect(Rectangle<float> (xf / maxXIndex * winWidth,
                                            (maxYIndex - yf) / maxYIndex * winHeight,
                                            binWidth,
                                            binHeight));
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

Colour Visualizer::intensityToColour(float intensity)
{
    return Colour(1.0f,1.0f,1.0f,1.0f);
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
        for (int row = 0; row < numFreqBins; ++row)
        {
            // if this position is greater than 0, perform convolution
            const double intensity = maskingInput[col][row];
            if (intensity > 0)
            {
                for (int idx = 0; idx < 5; ++idx)
                {
                    const int j = row - 2 + idx;
                    if (j > 0 && j < numFreqBins)
                        colBuffer[j] += freqGaussian[idx]*intensity;
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
    // calculate masking for each row
    for (int row = 0; row < numFreqBins; ++row)
    {
        for (int col = 0; col < numSpatialBins; ++col)
        {
            // if this position is greater than 0, perform convolution
            const double intensity = maskingInput[col][row];
            if (intensity > 0)
            {
                for (int idx = 0; idx < 11; ++idx)
                {
                    const int j = col - 5 + idx;
                    if (j > 0 && j < numSpatialBins)
                        rowBuffer[j] += spatialGaussian[idx]*intensity;
                }
            }
        }

        // place buffer back into the row
        for (int col = 0; col < numSpatialBins; ++col)
        {
            maskingInput[col][row] = rowBuffer[col];
            rowBuffer[col] = 0;
        }
    }
}

void Visualizer::makeGammatoneFilters()
{
    //ifstream filterText ("gammatone_filter.txt","r");
    int filter = 0;
    int i = 0;
    float num = 0;
    ifstream source;
    source.open("gammatone_filter.txt");
    for ( std::string line; std::getline(source,line);)
    {
        std::istringstream in(line);
        while (in >> num)
        {
            gammatoneFilter[filter][i] = (double) num;
            i = (i + 1) % 513; 
            if (i == 0)
                filter++;
        }
    }
}
