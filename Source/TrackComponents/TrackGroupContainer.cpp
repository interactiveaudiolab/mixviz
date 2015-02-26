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

#include "TrackGroupContainer.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TrackGroupContainer::TrackGroupContainer (MainWindow* mainWindow_, int groupIndex_, Colour groupColour_)
    : mainWindow(mainWindow_),
      groupIndex(groupIndex_),
      groupColour(groupColour_),
      isCurrentlyDragTarget(false)
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (200, 200);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

TrackGroupContainer::~TrackGroupContainer()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void TrackGroupContainer::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffafafaf));

    //[UserPaint] Add your own custom painting code here..
    if (isCurrentlyDragTarget)
    {
        g.fillAll (groupColour.interpolatedWith(Colours::grey, 0.55));
    }
    else
    {
        g.fillAll (groupColour.interpolatedWith(Colours::grey, 0.85));
    }

    g.setColour(groupColour);
    g.drawRect(0, 0, getWidth(), getHeight());
    //[/UserPaint]
}

void TrackGroupContainer::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void TrackGroupContainer::addLabel(String labelText)
{
    addAndMakeVisible(groupLabel = new Label("group label", labelText));
    groupLabel->setColour(Label::textColourId, groupColour.interpolatedWith(Colours::grey, 0.99));
    groupLabel->setBounds(0, 0, getWidth(), getHeight());
    groupLabel->setFont(groupLabel->getFont().withHeight(getHeight()/2));
    groupLabel->setJustificationType(4);
}

void TrackGroupContainer::removeTrackFromGroup(int track)
{
    std::cout << "Removing track from group " << groupIndex << ", old size: " << tracksInGroup.size();
    tracksInGroup.removeFirstMatchingValue(track);
    std::cout << ", new size: " << tracksInGroup.size() << std::endl;
    mainWindow->updateVisualizerTracksInGroup(groupIndex, tracksInGroup);
}

void TrackGroupContainer::addTrackToGroup(int track)
{
    std::cout << "Adding track to group " << groupIndex << ", old size: " << tracksInGroup.size();
    tracksInGroup.add(track);
    std::cout << ", new size: " << tracksInGroup.size() << std::endl;
    mainWindow->updateVisualizerTracksInGroup(groupIndex, tracksInGroup);
}

bool TrackGroupContainer::isInterestedInDragSource (const SourceDetails &dragSourceDetails)
{
    if (dragSourceDetails.sourceComponent->getName().compare("TrackBox") == 0)
    {
        return true;
    }
    return false;
}

void TrackGroupContainer::itemDragEnter(const SourceDetails &dragSourceDetails)
{
    isCurrentlyDragTarget = true;
    repaint();
}

void TrackGroupContainer::itemDragExit (const SourceDetails &dragSourceDetails)
{
    // repaint the background of the TrackGroupContainer to its original colour
    isCurrentlyDragTarget = false;
    repaint();
}

void TrackGroupContainer::itemDropped (const SourceDetails &dragSourceDetails)
{
    // repaint the background of the TrackGroupContainer to its original colour
    isCurrentlyDragTarget = false;
    repaint();

    // get the TrackBox's trackIndex and its old trackGroupIndex
    TrackBox* trackBox = (TrackBox*) dragSourceDetails.sourceComponent.get();
    int oldTrackGroupIndex = trackBox->getTrackGroupIndex();
    int trackIndex = trackBox->getTrackIndex();

    // get the parent component
    TrackSelector* trackSelector = (TrackSelector*) getParentComponent();
    trackSelector->switchTrackToNewGroup(trackIndex, oldTrackGroupIndex, groupIndex, dragSourceDetails.localPosition);

}

Colour TrackGroupContainer::getGroupColour()
{
    return groupColour;
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="TrackGroupContainer" componentName=""
                 parentClasses="public Component, public DragAndDropTarget" constructorParams="MainWindow* mainWindow_, int groupIndex_, Colour groupColour_"
                 variableInitialisers="mainWindow(mainWindow_),&#10;groupIndex(groupIndex_),&#10;groupColour(groupColour_),&#10;isCurrentlyDragTarget(false)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="200" initialHeight="200">
  <BACKGROUND backgroundColour="ffafafaf"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
