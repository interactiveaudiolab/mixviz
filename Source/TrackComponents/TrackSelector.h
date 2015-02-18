/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.1

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_3293B5410B5A1C94__
#define __JUCE_HEADER_3293B5410B5A1C94__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "TrackBox.h"
class TrackGroupContainer;
class MainWindow;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class TrackSelector  : public Component,
                       public DragAndDropContainer
{
public:
    //==============================================================================
    TrackSelector (MainWindow* mainWindow_);
    ~TrackSelector();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void makeTrackBoxes(StringArray trackNames);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    MainWindow* mainWindow;
    int nTrackGroups;
    int nTrackBoxes;
    OwnedArray<TrackBox> trackBoxes;
    OwnedArray<TrackGroupContainer> trackGroupContainers;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackSelector)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_3293B5410B5A1C94__
