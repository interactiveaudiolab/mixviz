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
#include <loudness/Models/DynamicPartialLoudnessGM.h>

using namespace std;

//==============================================================================
Visualizer::Visualizer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setOpaque(true);
    startTimer(1000/30);

    // give settings default values
    changeSettings(4, 128, 40, 150.0f, 10.0f, 0.94, 0, 0);

    //initialize gaussians
    for (int i = -5; i < 6; ++i)
        spatialGaussian[i+5] = exp(pow((float)i,2.0f)/-8.0f);
    for (int i = -2; i < 3; ++i)
        freqGaussian[i+2] = exp(pow((float)i,2.0f)/-8.0f);

    // initialize 
    audioInputBank = new loudness::TrackBank(4, 1, 1024);

    // create the model and initialize
    model = new loudness::DynamicPartialLoudnessGM("48000_IIR_23_freemid.npy");
    model->initialize(audioInputBank);

    roexBankOutput = model->getModuleOutput(5);
    partialLoudnessOutput = model->getModuleOutput(6);
}

Visualizer::~Visualizer()
{
    delete(model)

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
    // for each target track, analyze
    for (int targetTrack = 0; targetTrack < numTracks; ++targetTrack)
    {
        // copy input L and R channel data into our input sample buffer
        // also set the maskerL and maskerR buffers to 0
        for (int i = 0; i < numSamples; ++i)
        {
            targetR[i] = (double) inputChannelData[2*targetTrack][i];
            targetL[i] = (double) inputChannelData[2*targetTrack+1][i];
            maskerR[i] = 0;
            maskerL[i] = 0;
        }

        // sum the signal for all tracks that aren't the target
        for (int maskerTrack = 0; maskerTrack < numTracks; ++maskerTrack)
        {
            if (maskerTrack != targetTrack)
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    maskerR[i] += (double) inputChannelData[2*targetTrack][i];
                    maskerL[i] += (double) inputChannelData[2*targetTrack+1][i];
                }
            }
        }

        // feed target and masker into loudness model

        // run the model

        // put model output into a buffer
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
    g.drawFittedText ("Frequency (Hz)", Rectangle<int>(0,(int)(winHeight / 2.0f + topBorder), (int)(leftBorder/2.0f), (int)(winHeight/3.0f)), Justification(1), 10);
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
