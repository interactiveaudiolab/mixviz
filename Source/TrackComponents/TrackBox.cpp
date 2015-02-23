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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "TrackBox.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TrackBox::TrackBox (String trackName_, Colour trackColour_, int trackIndex_, int trackGroupIndex_)
    : trackName(trackName_),
      trackColour(trackColour_),
      trackIndex(trackIndex_),
      trackGroupIndex(trackGroupIndex_)
{
    setName ("TrackBox");

    //[UserPreSize]
    //[/UserPreSize]

    setSize (50, 20);


    //[Constructor] You can add your own custom stuff here..
    // make sure these are always on top of TrackGroupContainers
    setAlwaysOnTop(true);
    //[/Constructor]
}

TrackBox::~TrackBox()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void TrackBox::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    g.fillAll(trackColour);
    g.setColour(Colours::black);
    g.drawText(trackName, Rectangle<int>(0, 0, getWidth(), getHeight()), Justification(4), true);
    //[/UserPaint]
}

void TrackBox::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //constrainer.setMinimumOnscreenAmounts (getParentHeight(), getParentWidth(), getParentHeight(), getParentWidth());
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void TrackBox::mouseEnter (const MouseEvent& e)
{
    //[UserCode_mouseEnter] -- Add your code here...
    //[/UserCode_mouseEnter]
}

void TrackBox::mouseExit (const MouseEvent& e)
{
    //[UserCode_mouseExit] -- Add your code here...
    //[/UserCode_mouseExit]
}

void TrackBox::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    DragAndDropContainer* dragContainer = DragAndDropContainer::findParentDragContainerFor(this);
    startingDragPositionRelativeToSelf = e.getPosition();
    dragContainer->startDragging ("TrackBox", this);
    //[/UserCode_mouseDown]
}

void TrackBox::mouseDrag (const MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    //dragger.dragComponent (this, e, &constrainer);
    //[/UserCode_mouseDrag]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void TrackBox::changeTrackGroup(int newTrackGroupIndex, Colour newTrackColour)
{
    trackGroupIndex = newTrackGroupIndex;
    trackColour = newTrackColour;
}

int TrackBox::getTrackGroupIndex()
{
    return trackGroupIndex;
}

int TrackBox::getTrackIndex()
{
    return trackIndex;
}

Point<int> TrackBox::getStartingDragPositionRelativeToSelf()
{
    return startingDragPositionRelativeToSelf;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="TrackBox" componentName="TrackBox"
                 parentClasses="public Component" constructorParams="String trackName_, Colour trackColour_, int trackIndex_, int trackGroupIndex_"
                 variableInitialisers="trackName(trackName_),&#10;trackColour(trackColour_),&#10;trackIndex(trackIndex_),&#10;trackGroupIndex(trackGroupIndex_)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="50" initialHeight="20">
  <METHODS>
    <METHOD name="mouseEnter (const MouseEvent&amp; e)"/>
    <METHOD name="mouseExit (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
