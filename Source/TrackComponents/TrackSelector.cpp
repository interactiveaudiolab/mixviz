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

#include "TrackSelector.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TrackSelector::TrackSelector ()
{
    setName ("TrackSelector");

    //[UserPreSize]
    //[/UserPreSize]

    setSize (700, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

TrackSelector::~TrackSelector()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void TrackSelector::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::grey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void TrackSelector::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void TrackSelector::makeTrackBoxes(StringArray trackNames)
{
  // remove any previous TrackBoxes
  removeAllChildren();

  // calculate trackbox spacing
  const int spacing = 5;
  const int trackBoxHeight = 20;
  const int width = getWidth();
  const int height = getHeight();
  const int tracksPerRow = 2;
  const int tracksPerCol = 2;
  const int trackBoxWidth = (width - (tracksPerRow * spacing)) / tracksPerRow;

  int nTracks = trackNames.size();
  for (int i=0; i<nTracks; ++i)
  {
    // calculate TrackBox colour and add it to the trackBoxes array
    Colour boxColour = Colour((float) i / (float) nTracks, 0.8f, 1.0f, 1.0f);
    trackBoxes.add(new TrackBox(trackNames[i], boxColour));

    // make the new TrackBox component visible
    addAndMakeVisible(trackBoxes[i]);
    int row = i / tracksPerRow;
    int col = i % tracksPerCol;
    int topY = row * (trackBoxHeight + spacing);
    int topX = col * (trackBoxWidth + spacing);
    trackBoxes[i]->setTopLeftPosition(topX, topY);
  }
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="TrackSelector" componentName="TrackSelector"
                 parentClasses="public Component, public DragAndDropContainer"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="700"
                 initialHeight="400">
  <BACKGROUND backgroundColour="ff808080"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
