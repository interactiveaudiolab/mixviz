/*
  ==============================================================================

    Settings.cpp
    Created: 28 Apr 2014 10:14:16am
    Author:  jon

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SettingsPanel.h"
#include "Visualizer.h"

SettingsPanel::SettingsPanel()
//		: depthLabel (String::empty, "Visualizer Settings"),
	//	  infoLabel (String::empty, "several visualizer settings")
{
	// create various sliders
    addAndMakeVisible (intensityScalingConstantSlider = new Slider());
    intensityScalingConstantSlider->setSliderStyle (Slider::LinearBar);
    intensityScalingConstantSlider->setValue (5000);
    intensityScalingConstantSlider->setRange (1, 15000);
    intensityScalingConstantSlider->addListener (this);

    addAndMakeVisible (intensityCutoffConstantSlider = new Slider());
    intensityCutoffConstantSlider->setSliderStyle (Slider::LinearBar);
    intensityCutoffConstantSlider->setValue (10);
    intensityCutoffConstantSlider->setRange (1, 50);
    intensityCutoffConstantSlider->addListener (this);

    addAndMakeVisible (timeDecayConstantSlider = new Slider());
    timeDecayConstantSlider->setSliderStyle (Slider::LinearBar);
    timeDecayConstantSlider->setValue (0.70);
    timeDecayConstantSlider->setRange (0, 0.99);
    timeDecayConstantSlider->addListener (this);

    addAndMakeVisible (maskingThresholdSlider = new Slider());
    maskingThresholdSlider->setSliderStyle (Slider::LinearBar);
    maskingThresholdSlider->setValue (2);
    maskingThresholdSlider->setRange (0.1, 6);
    maskingThresholdSlider->addListener (this);

    //visualizer = v;
}

SettingsPanel::~SettingsPanel() {}

void SettingsPanel::paint (Graphics& g)
{
	g.fillAll(Colours::grey);
}

void sliderValueChanged (Slider*)
{
	float intensityScalingConstant = (float) intensityScalingConstantSlider->getValue();
	float intensityCutoffConstantSlider = (float) intensityCutoffConstantSlider->getValue();
	double timeDecayConstant = (double) timeDecayConstantSlider->getValue();
	double maskingThreshold = (double) maskingThresholdSlider->getValue();
    visualizer->changeSettings(intensityScalingConstant,
    						   intensityCutoffConstant,
    						   timeDecayConstant,
    						   maskingThreshold);
}

void SettingsPanel::resized()
{
    intensityScalingConstantSlider->setBounds(0,0,100,65);
    intensityCutoffConstantSlider->setBounds(150,0,100,65);
    timeDecayConstantSlider->setBounds(300,0,100,65);
    maskingThresholdSlider->setBounds(450,0,100,65);
}