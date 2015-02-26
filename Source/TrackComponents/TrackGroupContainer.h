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

#ifndef __JUCE_HEADER_D10F0458577F46DE__
#define __JUCE_HEADER_D10F0458577F46DE__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "TrackBox.h"
#include "../MainWindow.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class TrackGroupContainer  : public Component,
                             public DragAndDropTarget
{
public:
    //==============================================================================
    TrackGroupContainer (MainWindow* mainWindow_, int groupIndex_, Colour groupColour_);
    ~TrackGroupContainer();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void removeTrackFromGroup(int track);
    void addTrackToGroup(int track);

    Colour getGroupColour();

    bool isInterestedInDragSource (const SourceDetails &dragSourceDetails);
    void itemDragEnter(const SourceDetails &dragSourceDetails);
    void itemDragExit (const SourceDetails &dragSourceDetails);
    void itemDropped (const SourceDetails &dragSourceDetails);
    void addLabel (String labelText);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScopedPointer<Label> groupLabel;
    MainWindow* mainWindow;
    int groupIndex;
    Array<int> tracksInGroup;
    Colour groupColour;
    bool isCurrentlyDragTarget;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackGroupContainer)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_D10F0458577F46DE__
