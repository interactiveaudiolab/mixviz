/*
  ==============================================================================

    Visualizer.cpp
    Created: 28 Apr 2014 10:14:16am
    Author:  jon

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Visualizer.h"

//==============================================================================
Visualizer::Visualizer()
    : nextSample(0), subSample(0), accumulator(0)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setOpaque (true);
    clear();
    startTimer(1000/75);
}

Visualizer::~Visualizer()
{
}

void Visualizer::audioDeviceAboutToStart (AudioIODevice* device)
{
    inputChannelNames = device->getInputChannelNames();
    outputChannelNames = device->getOutputChannelNames();
    clear();
}

void Visualizer::audioDeviceStopped()
{
    clear();
}

void Visualizer::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                        float** outputChannelData, int numOutputChannels,
                                        int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float inputSample = 0;

        for (int chan = 0; chan < numInputChannels; ++chan)
            if (inputChannelData[chan] != nullptr)
                inputSample += std::abs (inputChannelData[chan][i]);  // find the sum of all the channels

        pushSample (10.0f * inputSample); // boost the level to make it more easily visible.
    }
        // We need to clear the output buffers before returning, in case they're full of junk..
        for (int j = 0; j < numOutputChannels; ++j)
            if (outputChannelData[j] != nullptr)
                zeromem (outputChannelData[j], sizeof (float) * (size_t) numSamples);
}

void Visualizer::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (Colours::black);   // clear the background
    const float midY = getHeight() * 0.5f;
    int samplesAgo = (nextSample + numElementsInArray (samples) - 1);

    RectangleList<float> waveform;

    for (int x = jmin (getWidth(), (int) numElementsInArray (samples)); --x >= 0;)
    {
        const float sampleSize = midY * samples [samplesAgo-- % numElementsInArray (samples)];
        waveform.addWithoutMerging (Rectangle<float> ((float) x, midY - sampleSize, 1.0f, sampleSize * 2.0f));
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
    zeromem (samples, sizeof (samples));
    accumulator = 0;
    subSample = 0;
}

void Visualizer::timerCallback()
{
    repaint();
}

void Visualizer::pushSample(const float newSample)
{
    accumulator += newSample;

    if (subSample == 0)
    {
        const int inputSamplesPerPixel = 200;

        samples[nextSample] = accumulator / inputSamplesPerPixel;
        nextSample = (nextSample + 1) % numElementsInArray (samples);
        subSample = inputSamplesPerPixel;
        accumulator = 0;
    }
    else
    {
        --subSample;
    }
}
