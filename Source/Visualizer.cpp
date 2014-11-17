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
    changeSettings(2, 128, 150.0f, 10.0f, 0.94, 2);

    //initialize gaussians
    for (int i = -5; i < 6; ++i)
        spatialGaussian[i+5] = exp(pow((float)i,2.0f)/-8.0f);
    for (int i = -2; i < 3; ++i)
        freqGaussian[i+2] = exp(pow((float)i,2.0f)/-8.0f);

    shouldPrint = 0;

    audioInputBank = new loudness::TrackBank();
    audioInputBank->initialize(4, 1, 1024, 48000);

    model = new loudness::DynamicPartialLoudnessGM("48000_IIR_23_freemid.npy");
    model->initialize(*audioInputBank);

    powerSpectrumOutput = model->getModuleOutput(2);
    roexBankOutput = model->getModuleOutput(4);
    partialLoudnessOutput = model->getModuleOutput(5);

    numFreqBins = roexBankOutput->getNChannels();
}

Visualizer::~Visualizer()
{
}

void Visualizer::changeSettings(const int numTracks_, const int numSpatialBins_, const float intensityScalingConstant_, const float intensityCutoffConstant_, const double timeDecayConstant_, const double maskingThreshold_)
{
    numTracks = numTracks_;
    numSpatialBins = numSpatialBins_;
    intensityScalingConstant = intensityScalingConstant_;
    intensityCutoffConstant = intensityCutoffConstant_;
    timeDecayConstant = timeDecayConstant_;
    maskingThreshold = maskingThreshold_;
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
    double lo = 0;
    double pl = 0;

    // for each track
    for (int target = 0; target < numTracks; ++target)
    {
        // copy input L and R channel data into our input sample buffer
        for (int i = 0; i < numSamples; ++i)
        {
            audioInputBank->setSample(2*target, 0, i, (double) inputChannelData[target*2][i]); // right
            audioInputBank->setSample(2*target+1, 0, i, (double) inputChannelData[target*2+1][i]); // left
        }
    }

    // run the model
    model->process(*audioInputBank);

    if (shouldPrint == 0)
    {
        std::cout << "Instantaneous Loudness\n";
        for (int chn = 0; chn < partialLoudnessOutput->getNChannels(); chn++)
        {
            lo += partialLoudnessOutput->getSample(0,chn,0);
            pl += partialLoudnessOutput->getSample(0,chn,1);
        }
        cout << "lo: " << lo << " pl: " << pl << endl;
    }
    else
    {
        shouldPrint = (shouldPrint + 1) % 20000;
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
    const float maxXIndex = 180.0f; // number of degrees
    const float maxYIndex = (float) numFreqBins;
    const float binHeight = winHeight / maxYIndex;
    const float binWidth = winWidth / maxXIndex;
    const float textWidth = 50.0f;
    const float textOffset = textWidth / 2.0f;
    const float tickHeight = 5.0f;
    
    // draw the patterns for the target tracks
    for (int target = 0; target < numTracks; ++target)
    {
        for (int freq = 0; freq < numFreqBins; ++freq)
        {
            const float intensity = (float) roexBankOutput->getSample(target, freq, 0);
            if (intensity > intensityCutoffConstant)
            {
                const float xf = (float) powerSpectrumOutput->getSpatialPosition(target, freq) + 90.0f; // add 90 so all values are positive
                const float yf = (float) freq;

                // if there is masking, colour is black
                if (log(partialLoudnessOutput->getSample(target, freq, 0)) - log(partialLoudnessOutput->getSample(target, freq, 1)) > maskingThreshold)
                {
                    g.setColour(Colours::black);
                }
                else
                {
                    g.setColour(intensityToColour(intensity,target));
                }
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

Colour Visualizer::intensityToColour(const float intensity, const int track)
{
    return Colour((float)track / (float)numTracks, intensity / intensityScalingConstant, 1.0f, 1.0f);
}
