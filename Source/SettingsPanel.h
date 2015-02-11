/*
  ==============================================================================

    Settings.h
    Created: 28 Apr 2014 10:14:16am
    Author:  jon

  ==============================================================================
*/

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Visualizer.h"

//==============================================================================
/*
*/

class SettingsPanel 	: public Component,
                  		  public SliderListener,
                  		  public ButtonListener
{
public:
	SettingsPanel();
	~SettingsPanel();

	void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void sliderValueChanged (Slider*) override;

private:
	ScopedPointer<Visualizer> visualizer;
	ScopedPointer<Slider> intensityScalingConstantSlider;
    ScopedPointer<Slider> intensityCutoffConstantSlider;
    ScopedPointer<Slider> timeDecayConstantSlider;
    ScopedPointer<Slider> maskingThresholdSlider;
    ScopedPointer<Slider> detectionModeSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};

#endif  // SETTINGS_H_INCLUDED