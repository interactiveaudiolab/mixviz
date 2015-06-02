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
    : nTrackGroups(4),
      nSpatialBins(128),
      intensityScalingConstant(18000.0f),
      intensityCutoffConstant(6000.0f),
      timeDecayConstant(0.85),
      maskingTimeDecayConstant(0.60),
      maskingThreshold(1),
      detectionMode(false),
      leftBorder(100.0f),
      rightBorder(1.0f),
      topBorder(5.0f),
      bottomBorder(40.0f)
{
    setOpaque(true);
    startTimer(1000/30);
    setName("Music Visualizer Window");

    for (int i =0; i < nTrackGroups; i++)
    {
        trackGroups.add(Array<int>());
        groupHues.add((float) i / (float) nTrackGroups);
    }

    visualizationImage = Image(Image::RGB, 700, 600, true);
}

Visualizer::~Visualizer()
{
}

void Visualizer::changeNTrackGroups(int newNTrackGroups)
{
    nTrackGroups = newNTrackGroups;
    groupHues.clear();
    for (int i =0; i < nTrackGroups; i++)
    {
        trackGroups.add(Array<int>());
        groupHues.add((float) i / (float) nTrackGroups);
    }

    model->reset();
    audioInputBank->resize(2 * nTrackGroups);
    model->initialize(*audioInputBank);
    std::cout << "get input n tracks" << audioInputBank->getNTracks() << std::endl;

    output.resize(2 * nTrackGroups);
    for (int track = 0; track < 2 * nTrackGroups; track++)
    {
        output[track].resize(nFreqBins);
        for (int freq = 0; freq < nFreqBins; freq++)
            output[track][freq].assign(180, 0);
    }
}

void Visualizer::clearTrackGroups()
{
    nTrackGroups = 0;
    trackGroups.clear();
}

void Visualizer::updateTracksInGroup(int groupIndex, Array<int> tracksInGroup)
{
    trackGroups.set(groupIndex, tracksInGroup);
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    // get info about the audio device we are connected to
    activeInputChannels = device->getActiveInputChannels();
    bufferSize = device->getCurrentBufferSizeSamples();
    numActiveChannels = activeInputChannels.countNumberOfSetBits();
    fs = device->getCurrentSampleRate();
    if (fs != 44100)
    {
        std::cout << "WARNING: loudness model currently configured to use 44100hz sample rate" << std::endl;
    }

    audioInputBank = new loudness::TrackBank();
    std::cout << "bufferSize: " << bufferSize << std::endl;
    audioInputBank->initialize(2 * nTrackGroups, 1, bufferSize, fs);

    model = new loudness::DynamicPartialLoudnessGM("44100_IIR_23_freemid.npy");
    model->initialize(*audioInputBank);

    powerSpectrumOutput = model->getModuleOutput(2);
    roexBankOutput = model->getModuleOutput(4);
    partialLoudnessOutput = model->getModuleOutput(5);
    integratedLoudnessOutput = model->getModuleOutput(6);

    nFreqBins = roexBankOutput->getNChannels();
    cutoffFreqs = roexBankOutput->getCentreFreqs();
    
    output.resize(2 * nTrackGroups);
    for (int track = 0; track < 2 * nTrackGroups; track++)
    {
        output[track].resize(nFreqBins);
        for (int freq = 0; freq < nFreqBins; freq++)
            output[track][freq].assign(180, 0);
    }
}

void Visualizer::audioDeviceStopped()
{
}

// this function happens each time audio data is recieved
void Visualizer::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                        float** outputChannelData, int numOutputChannels,
                                        int numSamples)
{
    // for each track group
    // NOTE: no gaps in the trackGroups array due to construction
    //std::chrono::time_point<std::chrono::system_clock> start, end;
    //start = std::chrono::system_clock::now();

    for (int groupIndex = 0; groupIndex < nTrackGroups; ++groupIndex)
    {
        // set inputs to model to zero
        for (int i = 0; i < numSamples; ++i)
        {
            audioInputBank->setSample(2*groupIndex, 0, i, 0.0000001); // right
            audioInputBank->setSample(2*groupIndex+1, 0, i, 0.0000001); // left
        }

        // loop over tracks in the group and add their data to the audioInputBank

        for (int targetIndex = 0; targetIndex < trackGroups[groupIndex].size(); ++targetIndex)
        {
            int targetIOIndex = trackGroups[groupIndex][targetIndex];
            // copy input L and R channel data into our input sample buffer
            for (int i = 0; i < numSamples; ++i)
            {
                audioInputBank->sumSample(2*groupIndex, 0, i, (double) inputChannelData[targetIOIndex*2+1][i]); // right
                audioInputBank->sumSample(2*groupIndex+1, 0, i, (double) inputChannelData[targetIOIndex*2][i]); // left
            }
        }
    }

    // run the model
    model->process(*audioInputBank);

    //double il = 0;
    //for (int freq = 0; freq < nFreqBins; ++freq)
    //    il += partialLoudnessOutput->getSample(1, freq, 1);

    // apply time decay to current track output
    for (int track = 0; track < nTrackGroups; ++track)
        for (int freq = 0; freq < nFreqBins; ++freq)
            for (int pos = 0; pos < 180; ++pos)
                output[track][freq][pos] *= timeDecayConstant;

    // apply masking time decay to current masking output
    for (int track = nTrackGroups; track < 2 * nTrackGroups; ++track)
        for (int freq = 0; freq < nFreqBins; ++freq)
            for (int pos = 0; pos < 180; ++pos)
                output[track][freq][pos] *= maskingTimeDecayConstant;
    
    // add current loudness values into output matrix
    for (int track = 0; track < nTrackGroups; ++track)
    {
        for (int freq = 0; freq < nFreqBins; ++freq)
        {
            const float intensity = (float) roexBankOutput->getSample(track, freq, 0);
            if (intensity > intensityCutoffConstant)
            {
                const int spatialBin = (int) powerSpectrumOutput->getSpatialPosition(track, freq);

                // detect masking
                // loudness - partial loudness
                if (log(integratedLoudnessOutput->getSample(track, freq, 0)) - log(integratedLoudnessOutput->getSample(track, freq, 3)) > maskingThreshold)
                {
                    output[track+nTrackGroups][freq][spatialBin] += intensity;
                }
                else
                {
                    output[track][freq][spatialBin] += intensity;
                }
            }
        }
    }
}

// this function is called at each timer callback,
void Visualizer::paint (Graphics& g)
{
    const float height = (float) getHeight();
    const float width = (float) getWidth();
    const float winHeight = height - bottomBorder - topBorder;
    const float winWidth = width - leftBorder - rightBorder;
    const float maxXIndex = 180.0f; // number of degrees
    const float maxYIndex = (float) nFreqBins;
    const float binHeight = winHeight / maxYIndex;
    const float binWidth = winWidth / maxXIndex;
    const float textWidth = 50.0f;
    const float textOffset = textWidth / 2.0f;
    const float tickHeight = 5.0f;

    Graphics visualization (visualizationImage);
    visualization.fillAll (Colours::grey);

    visualization.setColour(Colours::black);
    visualization.fillRect(Rectangle<float>(leftBorder, topBorder, winWidth, winHeight));
    if (detectionMode)
    {
        // only draw maskers
        for (int track = 0; track < nTrackGroups; ++track)
        {
            const float groupHue = groupHues[track];
            const int idx = track + nTrackGroups;
            for (int freq = 0; freq < nFreqBins; ++freq)
            {
                for (int pos = 0; pos < 180; ++pos)
                {
                    const float intensity = output[idx][freq][pos];
                    if (intensity > intensityCutoffConstant)
                    {
                        const float xf = (float) pos; // add 90 so all values are positive
                        const float yf = (float) freq;
                        visualization.setColour(trackIntensityToColour(intensity, groupHue));
                        visualization.fillRect(Rectangle<float>((xf + 1.0f) / maxXIndex * winWidth - binWidth + leftBorder,
                                                ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                                binWidth,
                                                binHeight));
                    }
                }
            }
        }
    }
    else
    {
        // draw the current output matrix
        for (int track = 0; track < nTrackGroups; ++track)
        {
            const float groupHue = groupHues[track];
            for (int freq = 0; freq < nFreqBins; ++freq)
            {
                for (int pos = 0; pos < 180; ++pos)
                {
                    const float intensity = output[track][freq][pos];
                    if (intensity > intensityCutoffConstant)
                    {
                        const float xf = (float) pos; // add 90 so all values are positive
                        const float yf = (float) freq;
                        visualization.setColour(trackIntensityToColour(intensity, groupHue));
                        visualization.fillRect(Rectangle<float>((xf + 1.0f) / maxXIndex * winWidth - binWidth + leftBorder,
                                                ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                                binWidth,
                                                binHeight));
                    }
                }
            }
        }
        // draw maskers
        for (int track = nTrackGroups; track < 2 * nTrackGroups; ++track)
        {
            const float groupHue = groupHues[track - nTrackGroups];
            for (int freq = 0; freq < nFreqBins; ++freq)
            {
                for (int pos = 0; pos < 180; ++pos)
                {
                    const float intensity = output[track][freq][pos];
                    if (intensity > intensityCutoffConstant)
                    {
                        const float xf = (float) pos; // add 90 so all values are positive
                        const float yf = (float) freq;
                        visualization.setColour(maskerIntensityToColour(intensity, groupHue));
                        visualization.fillRect(Rectangle<float>((xf + 1.0f) / maxXIndex * winWidth - binWidth + leftBorder,
                                                ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                                binWidth,
                                                binHeight));
                    }
                }
            }
        }
    }


    // draw a line down the middle and around this box
    visualization.setColour(Colours::white);
    visualization.fillRect(Rectangle<float>(leftBorder, topBorder, winWidth, 1.0f)); // top line
    visualization.fillRect(Rectangle<float>(leftBorder, winHeight + topBorder, winWidth, 1.0f)); // bottom line
    visualization.fillRect(Rectangle<float>(leftBorder, topBorder, 1.0f, winHeight)); // left line
    visualization.fillRect(Rectangle<float>(width - rightBorder, topBorder, 1.0f, winHeight)); // right line
    visualization.fillRect(Rectangle<float>(winWidth / 2.0f + leftBorder, topBorder, 1.0f, winHeight)); // middle line

    visualization.setColour(Colours::black);
    // draw text and tick marks for labeling the graph
    visualization.drawText ("Spatial Position", 
                Rectangle<float>(winWidth / 2.0f + leftBorder - 40.0f,
                                                    winHeight + bottomBorder / 2.0f + topBorder,
                                                    100.0f,
                                                    10.0f),
                Justification(1),
                true);

    // L100
    visualization.fillRect (Rectangle<float>(leftBorder, winHeight + topBorder, 1.0f, tickHeight));
    visualization.drawText ("L100",
                Rectangle<float>(leftBorder - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // L50
    visualization.fillRect (Rectangle<float>(leftBorder + winWidth / 4.0f, winHeight + topBorder, 1.0f, tickHeight));
    visualization.drawText ("L50",
                Rectangle<float>(leftBorder + winWidth / 4.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);
    
    // C
    visualization.fillRect (Rectangle<float>(leftBorder + winWidth / 2.0f, winHeight + topBorder, 1.0f, tickHeight));
    visualization.drawText ("C",
                Rectangle<float>(leftBorder + winWidth / 2.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // R50
    visualization.fillRect (Rectangle<float>(leftBorder + 3.0f * winWidth / 4.0f, winHeight + topBorder, 1.0f, tickHeight));
    visualization.drawText ("R50",
                Rectangle<float>(leftBorder + 3.0f * winWidth / 4.0f - textOffset, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    // R100
    visualization.fillRect (Rectangle<float>(leftBorder + winWidth, winHeight + topBorder, 1.0f, tickHeight));
    visualization.drawText ("R100",
                Rectangle<float>(leftBorder + winWidth - textOffset - 15.0f, winHeight + 6.0f + topBorder, textWidth, 10.0f),
                Justification(4),
                true);

    
    // draw cutoff frequency lines
    // first draw the 0 
    const float tickX = leftBorder - tickHeight;
    const float txtX = tickX - textWidth;
    visualization.fillRect (Rectangle<float>(tickX,  winHeight + topBorder, tickHeight, 1.0f));
    visualization.drawText ("0", Rectangle<float>(txtX, winHeight + topBorder, tickHeight, 10.0f), Justification(4), true);
    for (int i = 0; i < nFreqBins; i += 10)
    {
        const float yf = (float) i;
        visualization.fillRect (Rectangle<float>(   tickX, 
                                        ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight + topBorder,
                                        tickHeight,
                                        1.0f));
        visualization.drawText (String((int) cutoffFreqs[i]),
                    Rectangle<float>(   txtX, 
                                        ((maxYIndex - yf) / maxYIndex) * winHeight - binHeight - 7.0f + topBorder,
                                        textWidth,
                                        10.0f),
                    Justification(12),
                    true);
    }
    visualization.addTransform(AffineTransform().rotated(-3.1415/2, 0, winHeight / 2.0f + topBorder));
    visualization.drawFittedText ("Frequency (Hz)", Rectangle<int>(-10,(int)(winHeight / 2.0f + topBorder) - 65, (int)(leftBorder), (int)(winHeight/3.0f)), Justification(1), 10);

    g.drawImageAt(visualizationImage, 0, 0);
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

void Visualizer::setIntensityScalingConstant(const float intensityScalingConstant_)
{
    intensityScalingConstant = intensityScalingConstant_;
}

void Visualizer::setIntensityCutoffConstant(const float intensityCutoffConstant_)
{
    intensityCutoffConstant = intensityCutoffConstant_;
}

void Visualizer::setTimeDecayConstant(const double timeDecayConstant_)
{
    timeDecayConstant = timeDecayConstant_;
}

void Visualizer::setMaskingThreshold(const double maskingThreshold_)
{
    maskingThreshold = maskingThreshold_;
}

void Visualizer::setMaskingTimeDecayConstant(const double maskingTimeDecayConstant_)
{
    maskingTimeDecayConstant = maskingTimeDecayConstant_;
}

Colour Visualizer::trackIntensityToColour(const float intensity, const float groupHue)
{
    return Colour(groupHue, 0.8f, intensity / intensityScalingConstant, 1.0f);
}

Colour Visualizer::maskerIntensityToColour(const float intensity, const float groupHue)
{
    return Colour(groupHue, 0.2f, 1.0f, 1.0f);
}