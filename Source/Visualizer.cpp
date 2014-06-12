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
#include <cstdio>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

//==============================================================================
Visualizer::Visualizer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setOpaque(true);
    startTimer(1000/30);

    // give settings default values
    changeSettings(2, 128, 40, 100.0f, 5.0f, 0.5, 0, 0);

    // initialize fft plans
    fftL = fftw_plan_dft_r2c_1d(1024, fftInputL, fftOutputL, FFTW_MEASURE);
    fftR = fftw_plan_dft_r2c_1d(1024, fftInputR, fftOutputR, FFTW_MEASURE);

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
}

void Visualizer::changeSettings(const int tracks, const int spatialBins, const int freqBins, const float intensityScaling, const float intensityCutoff, const double timeDecay, const int freqFlag, const int spatialFlag)
{
    numTracks = tracks;
    numSpatialBins = spatialBins;
    numFreqBins = freqBins;
    intensityScalingConstant = intensityScaling;
    intensityCutoffConstant = intensityCutoff;
    timeDecayConstant = timeDecay;
    freqMaskingFlag = freqFlag;
    spatialMaskingFlag = spatialFlag;
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    // get info about the audio device we are connected to
    activeInputChannels = device->getActiveInputChannels();
    bufferSize = 1024; //device->getCurrentBufferSizeSamples();
    numActiveChannels = activeInputChannels.countNumberOfSetBits();
    cout << "active channels: " << numActiveChannels << endl;
    fs = device->getCurrentSampleRate();

    makeGammatoneFilters();
    clearMaskingOutput();
    clearMaskingInput();

    for (int row = 0; row < numFreqBins; ++row)
        colBuffer[row] = 0;
    for (int col = 0; col < numSpatialBins; ++col)
        rowBuffer[col] = 0;
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
    for (int track = 0; track < numTracks; ++track)
    {
        // copy input L and R channel data into our input sample buffer
        for (int i = 0; i < numSamples; ++i)
        {
            fftInputR[i] = (double) inputChannelData[2*track][i];
            fftInputL[i] = (double) inputChannelData[2*track+1][i];
        }

        // perform all the FFTs and calculate magnitudes
        fftw_execute(fftL);
        fftw_execute(fftR);
        for (int freq=0; freq < 513; ++freq)
        {
            fftMagnitudesL[freq] = sqrt(pow(fftOutputL[freq][0],2.0) + pow(fftOutputL[freq][1],2.0));
            fftMagnitudesR[freq] = sqrt(pow(fftOutputR[freq][0],2.0) + pow(fftOutputR[freq][1],2.0));
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

        // time decay
        for (int loc = 0; loc < numSpatialBins; ++loc)
            for (int freq = 0; freq < numFreqBins; ++freq)
                maskingInput[track][loc][freq] = timeDecayConstant * prevMaskingInput[track][loc][freq];

        // calculate spatial positions and update masking input
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            const double magnitudeL = filterOutputL[freq];
            const double magnitudeR = filterOutputR[freq];
            const double magnitudeStereo = 10*log10(magnitudeL + magnitudeR);

            const int spatialBin = calculateSpatialBin(magnitudeL,magnitudeR);
            if (magnitudeStereo > 0)
                maskingInput[track][spatialBin][freq] += magnitudeStereo;
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
    runMaskingModel();
    g.fillAll (Colours::white);   // clear the background
    const float leftBorder = 100.0f;
    const float rightBorder = 30.0f;
    const float bottomBorder = 40.0f;
    const float topBorder = 5.0f;
    const float height = (float) getHeight();
    const float width = (float) getWidth();
    const float winHeight = height - bottomBorder - topBorder;
    const float winWidth = width - leftBorder - rightBorder;
    const float maxXIndex = (float) numSpatialBins;
    const float maxYIndex = (float) numFreqBins;
    const float binHeight = winHeight / maxYIndex;
    const float binWidth = winWidth / maxXIndex;
    const float textWidth = 50.0f;
    const float textOffset = textWidth / 2.0f;
    const float tickHeight = 5.0f;
    
    // draw the tracks
    for (int track = 0; track < numTracks; ++track)
    {
        for (int x = 0; x < numSpatialBins; ++x)
        {
            for (int y = 0; y < numFreqBins; ++y)
            {
                const float intensity = (float) maskingOutput[track][x][y];
                if (intensity > intensityCutoffConstant) 
                {
                    //cout << "x: " << x << "\t\ty: " << y << "\t\t i: " << intensity << endl; 
                    const float xf = (float) x;
                    const float yf = (float) y;
                    g.setColour(intensityToColour(intensity,track));
                    g.fillRect(Rectangle<float>((xf + 1.0f) / maxXIndex * winWidth - binWidth + leftBorder,
                                                ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                                binWidth,
                                                binHeight));
                }
            }
        }
    }

    // draw a line down the middle and around this box
    g.setColour(Colours::black);
    g.fillRect(Rectangle<float>(leftBorder, topBorder, winWidth, 1.0f)); // top line
    g.fillRect(Rectangle<float>(leftBorder, winHeight + topBorder, winWidth, 1.0f)); // bottom line
    g.fillRect(Rectangle<float>(leftBorder, topBorder, 1.0f, winHeight)); // left line
    g.fillRect(Rectangle<float>(width - rightBorder, topBorder, 1.0f, winHeight)); // right line
    g.fillRect(Rectangle<float>(winWidth / 2.0f + leftBorder, topBorder, 1.0f, winHeight)); // middle line

    // draw text and tick marks for labeling the graph
    g.drawText ("Spatial Position", 
                Rectangle<float>(winWidth / 2.0f + leftBorder - 40.0f,
                                                    winHeight + bottomBorder / 3.0f + topBorder,
                                                    100.0f,
                                                    10.0f),
                Justification(1),
                true);

    // L100
    g.fillRect (Rectangle<float>(leftBorder, winHeight + topBorder, 1.0f, tickHeight));
    g.drawText ("L100",
                Rectangle<float>(leftBorder - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // L50
    g.fillRect (Rectangle<float>(leftBorder + winWidth / 4.0f, winHeight + topBorder, 1.0f, tickHeight));
    g.drawText ("L50",
                Rectangle<float>(leftBorder + winWidth / 4.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);
    
    // C
    g.fillRect (Rectangle<float>(leftBorder + winWidth / 2.0f, winHeight + topBorder, 1.0f, tickHeight));
    g.drawText ("C",
                Rectangle<float>(leftBorder + winWidth / 2.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // R50
    g.fillRect (Rectangle<float>(leftBorder + 3.0f * winWidth / 4.0f, winHeight + topBorder, 1.0f, tickHeight));
    g.drawText ("R50",
                Rectangle<float>(leftBorder + 3.0f * winWidth / 4.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // R100
    g.fillRect (Rectangle<float>(leftBorder + winWidth, winHeight + topBorder, 1.0f, tickHeight));
    g.drawText ("R100",
                Rectangle<float>(leftBorder + winWidth - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // draw cutoff frequency lines
    // first draw the 0 
    const float tickX = leftBorder - tickHeight;
    const float txtX = tickX - textWidth;
    g.fillRect (Rectangle<float>(tickX,  winHeight + topBorder, tickHeight, 1.0f));
    g.drawText ("0", Rectangle<float>(txtX, winHeight + topBorder, tickHeight, 10.0f), Justification(4), true);
    for (int i = 0; i < numFreqBins; ++i)
    {
        const float yf = (float) i;
        g.fillRect (Rectangle<float>(   tickX, 
                                        ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                        tickHeight,
                                        1.0f));
        g.drawText (String((int) cutoffFreqs[i]),
                    Rectangle<float>(   txtX, 
                                        ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight - 7.0f + topBorder,
                                        textWidth,
                                        10.0f),
                    Justification(12),
                    true);
    }

}

void Visualizer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

// sets masking input to 0's, saves previous masking input
void Visualizer::clearMaskingInput()
{
    for (int track = 0; track < numTracks; ++track)
    {
        for (int loc = 0; loc < numSpatialBins; ++loc)
        {
            for (int freq = 0; freq < numFreqBins; ++freq)
            {
                prevMaskingInput[track][loc][freq] = maskingInput[track][loc][freq];
                maskingInput[track][loc][freq] = 0;
            }
        }
    }
}

void Visualizer::clearMaskingOutput()
{
    for (int track = 0; track < numTracks; ++track)
    {
        for (int loc = 0; loc < numSpatialBins; ++loc)
        {
            for (int freq = 0; freq < numFreqBins; ++freq)
            {   
                maskingOutput[track][loc][freq]=0;
            }
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
        return ((int) ((magnitudeR / magnitudeL) * (numSpatialBins / 2)));
    }
    else
    {
        // signal is partially on the right
        return ((int) ((1 - (magnitudeL / magnitudeR)) * (numSpatialBins / 2)) + (numSpatialBins / 2 - 1));
    }
}

Colour Visualizer::intensityToColour(const float intensity, const int track)
{
    return Colour((float)track / (float)numTracks, intensity / intensityScalingConstant, 1.0f, 1.0f);
}


// executes the masking model on inputs
void Visualizer::runMaskingModel()
{
    // run the masking model for each track then copy into output buffer
    for (int track = 0; track < numTracks; ++track)
    {

        if (freqMaskingFlag)
            calculateFreqMasking(track);
        if (spatialMaskingFlag)
            calculateSpatialMasking(track);

        for (int loc = 0; loc < numSpatialBins; ++loc)
        {
            for (int freq = 0; freq < numFreqBins; ++freq)
            {
                maskingOutput[track][loc][freq] = maskingInput[track][loc][freq];
            }
        }
    }
}

void Visualizer::calculateFreqMasking(const int track)
{
    // calculate the masking for each column
    for (int col = 0; col < numSpatialBins; ++col)
    {
        for (int row = 0; row < numFreqBins; ++row)
        {
            // if this position is greater than 0, perform convolution
            const double intensity = maskingInput[track][col][row];
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
            maskingInput[track][col][row] = colBuffer[row];
            colBuffer[row]=0;
        }
    }
}

void Visualizer::calculateSpatialMasking(const int track)
{
    // calculate masking for each row
    for (int row = 0; row < numFreqBins; ++row)
    {
        for (int col = 0; col < numSpatialBins; ++col)
        {
            // if this position is greater than 0, perform convolution
            const double intensity = maskingInput[track][col][row];
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
            maskingInput[track][col][row] = rowBuffer[col];
            rowBuffer[col] = 0;
        }
    }
}

void Visualizer::makeGammatoneFilters()
{
    int filter = 0;
    int i = 0;
    float num = 0;

    // read in gammatone filter data, precomputed with 40 bins and 1024 fft size
    ifstream filtertxt;
    filtertxt.open("gammatone_filter.txt");
    for ( std::string line; std::getline(filtertxt,line);)
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

    // read in cutoff frequency data of those gammatone filters
    ifstream cfreqstxt;
    cfreqstxt.open("cutoff_freqs.txt");
    for ( std::string line; std::getline(cfreqstxt,line);)
    {
        std::istringstream in(line);
        while (in >> num)
        {
            cutoffFreqs[i] = (double) num;
            i += 1;
        }
    }
}
