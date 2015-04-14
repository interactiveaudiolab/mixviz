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

#ifndef __JUCE_HEADER_3ACEDE7F045E12C4__
#define __JUCE_HEADER_3ACEDE7F045E12C4__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class TrackBox  : public Component
{
public:
    //==============================================================================
    TrackBox (String trackName_, Colour trackColour_, int trackIndex_, int trackGroupIndex_);
    ~TrackBox();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void changeTrackGroup(int newTrackGroupIndex, Colour newColour);
    int getTrackGroupIndex();
    int getTrackIndex();
    Point<int> getStartingDragPositionRelativeToSelf();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void mouseEnter (const MouseEvent& e);
    void mouseExit (const MouseEvent& e);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.'
    Point<int> startingDragPositionRelativeToSelf;
    Colour trackColour;
    String trackName;
    int trackIndex; // corresponds to the input io socket of this track as read in the jack.xml file
    int trackGroupIndex; // the index of the current TrackGroup that this trackBox belongs to
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackBox)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_3ACEDE7F045E12C4__
