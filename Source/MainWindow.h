/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_1A7C0314FA0C68BA__
#define __JUCE_HEADER_1A7C0314FA0C68BA__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "Visualizer.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainWindow  : public Component,
                    public ButtonListener
{
public:
    //==============================================================================
    MainWindow ();
    ~MainWindow();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    Array<PropertyComponent*> createSettings(bool first);
    Array<PropertyComponent*> createTracks(int numTracks);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScopedPointer<AudioIODevice> audioIODevice;
    ScopedPointer<AudioIODeviceType> audioIODeviceType;
    ScopedPointer<Visualizer> visualizer;
    ScopedPointer<PropertyPanel> settings;
    ScopedPointer<PropertyPanel> tracks;
    Value numTracksValue;
    Value numSpatialBinsValue;
    Value intensityScalingConstantValue;
    Value intensityCutoffConstantValue;
    Value timeDecayConstantValue;
    Value maskingThresholdValue;
    Value detectionModeValue;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> startButton;
    ScopedPointer<TextEditor> textEditor;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_1A7C0314FA0C68BA__
