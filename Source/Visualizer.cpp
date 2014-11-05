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
    changeSettings(4, 128, 150.0f, 10.0f, 0.94);

    //initialize gaussians
    for (int i = -5; i < 6; ++i)
        spatialGaussian[i+5] = exp(pow((float)i,2.0f)/-8.0f);
    for (int i = -2; i < 3; ++i)
        freqGaussian[i+2] = exp(pow((float)i,2.0f)/-8.0f);

    targetL.resize(1024);
    targetR.resize(1024);
    maskerL.resize(1024);
    maskerR.resize(1024);

    shouldPrint = 0;

    // initialize objects
    for (int track = 0; track < numTracks; ++track)
    {
        audioInputBankVector.push_back(std::unique_ptr<loudness::TrackBank> (new loudness::TrackBank()));
        audioInputBankVector[track]->initialize(4, 1, 1024, 48000);

        modelVector.push_back(std::unique_ptr<loudness::DynamicPartialLoudnessGM> (new loudness::DynamicPartialLoudnessGM("48000_IIR_23_freemid.npy")));
        modelVector[track]->initialize(*audioInputBankVector[track]);

        roexBankOutputVector.push_back(modelVector[track]->getModuleOutput(5));
        partialLoudnessOutputVector.push_back(modelVector[track]->getModuleOutput(6));
    }
    numFreqBins = roexBankOutputVector[0]->getNChannels();
}

Visualizer::~Visualizer()
{
}

void Visualizer::changeSettings(const int numTracks_, const int numSpatialBins_, const float intensityScalingConstant_, const float intensityCutoffConstant_, const double timeDecayConstant_)
{
    numTracks = numTracks_;
    numSpatialBins = numSpatialBins_;
    intensityScalingConstant = intensityScalingConstant_;
    intensityCutoffConstant = intensityCutoffConstant_;
    timeDecayConstant = timeDecayConstant_;
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
    // analyze each target
    for (int target = 0; target < numTracks; ++target)
    {
        // copy input L and R channel data into our input sample buffer
        // also set the maskerL and maskerR buffers to 0
        for (int i = 0; i < numSamples; ++i)
        {
            audioInputBankVector[target]->setSample(0, 0, i, (double) inputChannelData[target*2][i]); // track 0
            audioInputBankVector[target]->setSample(1, 0, i, (double) inputChannelData[target*2 + 1][i]); // track 1
            audioInputBankVector[target]->setSample(2, 0, i, 0); // track 2
            audioInputBankVector[target]->setSample(3, 0, i, 0); // track 3
        }

        // sum signals for other tracks to get masker
        for (int masker = 0; masker < numTracks; ++masker)
        {
            if (masker != target)
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    audioInputBankVector[target]->sumSample(2, 0, i, (double) inputChannelData[masker*2][i]);
                    audioInputBankVector[target]->sumSample(3, 0, i, (double) inputChannelData[masker*2 + 1][i]);
                }
            }
        }

        // run the model
        modelVector[target]->process(*audioInputBankVector[target]);

        if (shouldPrint == 0)
        {
            cout << "loudness and partial loudness for target track:\t" << target << endl;
            for (int i = 0; i < partialLoudnessOutputVector[target]->getNChannels(); ++i)
            {
                cout << "lo: bin" << i << ":\t" << partialLoudnessOutputVector[target]->getSample(0, i, 0) << "\t";
                cout << "pl: bin" << i << ":\t" << partialLoudnessOutputVector[target]->getSample(0, i, 1) << endl;
            }
        }
        else
        {
            shouldPrint = (shouldPrint + 1) % 2000;
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
    
    // draw the target tracks
    for (int target = 0; target < numTracks; ++target)
    {
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            const float intensity = (float) roexBankOutputVector[target]->getSample(0, freq, 0);
            if (intensity > intensityCutoffConstant)
            {
                const float xf = (float) (maxXIndex / 2); // temporarily no spatial bins
                const float yf = (float) freq;
                g.setColour(intensityToColour(intensity,target));
                g.fillRect(Rectangle<float>((xf + 1.0f) / maxXIndex * winWidth - binWidth + leftBorder,
                                            ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                            binWidth,
                                            binHeight));
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

    /*
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
    */
}

void Visualizer::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
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
