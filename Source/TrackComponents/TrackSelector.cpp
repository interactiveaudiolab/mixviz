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
#include "TrackGroupContainer.h"
#include "../MainWindow.h"
//[/Headers]

#include "TrackSelector.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
TrackSelector::TrackSelector (MainWindow* mainWindow_)
    : mainWindow(mainWindow_)
{
    setName ("TrackSelector");

    //[UserPreSize]
    //[/UserPreSize]

    setSize (700, 400);


    //[Constructor] You can add your own custom stuff here..
    // initalize track groups
    nTrackGroups = 5;
    const int nVisualizedTrackGroups = nTrackGroups - 1;
    std::cout << "nVisualizedTrackGroups: " << nVisualizedTrackGroups << std::endl;
    const int spacingBetweenTrackGroups = 30;
    const int leftAndTopSpacing = 5;
    const int width = getWidth() - leftAndTopSpacing * 2;
    const int height = getHeight() - leftAndTopSpacing * 2;

    // if there are more than 5 visualized track groups then we will make three rows
    const int nTrackGroupsPerCol = ((nVisualizedTrackGroups > 5) ? 3 : 2);
    std::cout << "nTrackGroupsPerCol: " << nTrackGroupsPerCol << std::endl;

    // if there is an odd number of tracks then we will have to squeeze the extras in some rows
    const int nExtraGroups = nVisualizedTrackGroups % nTrackGroupsPerCol;
    std::cout << "nExtraGroups: " << nExtraGroups << std::endl;

    // the number of TrackGroups to put in a row without an extra group in it
    // the number of regular rows is nTrackGroupsPerCol - nExtraGroups
    // the number of rows with an extra group is nExtraGroups
    const int nTrackGroupsPerRegularRow = nVisualizedTrackGroups / nTrackGroupsPerCol;
    std::cout << "nTrackGroupsPerRegularRow: " << nTrackGroupsPerRegularRow << std::endl; 

    // calculate width and height of trackboxes, with no spacing on edges
    // there is an extra track group at the bottom, so add one to nTrackGroupsPerCol
    const int trackGroupHeight = (height / (nTrackGroupsPerCol + 1)) - (spacingBetweenTrackGroups * (nTrackGroupsPerCol));
    const int trackGroupRegularWidth = (width / nTrackGroupsPerRegularRow) - (spacingBetweenTrackGroups * (nTrackGroupsPerRegularRow - 1));
    const int trackGroupExtraWidth = (width / (nTrackGroupsPerRegularRow + 1)) - (spacingBetweenTrackGroups * (nTrackGroupsPerRegularRow));
    std::cout << "height: " << trackGroupHeight << ", regular width: " << trackGroupRegularWidth << ", extra width: " << trackGroupExtraWidth << std::endl;

    for (int i = 0; i < nVisualizedTrackGroups; ++i)
    {
        // calculate color and add the group to the array
        Colour groupColour = Colour((float) i / (float) nVisualizedTrackGroups, 0.8f, 1.0f, 1.0f);
        trackGroupContainers.add(new TrackGroupContainer(mainWindow, i, groupColour));

        // make the new TrackGroupContainer visible and set position
        addAndMakeVisible(trackGroupContainers[i]);
        const int row = i / nTrackGroupsPerRegularRow;
        const int col = i % nTrackGroupsPerCol;

        // if this row is one with an extra group, we need to use trackGroupExtraWidth to calculate x position
        const int topY = row * (trackGroupHeight + spacingBetweenTrackGroups) + leftAndTopSpacing;
        int topX;
        if (row >= nExtraGroups)
        {          
            topX = col * (trackGroupExtraWidth + spacingBetweenTrackGroups) + leftAndTopSpacing;
            trackGroupContainers[i]->setBounds(topX, topY, trackGroupExtraWidth, trackGroupHeight);
        }
        else
        {
            topX = col * (trackGroupRegularWidth + spacingBetweenTrackGroups) + leftAndTopSpacing;
            trackGroupContainers[i]->setBounds(topX, topY, trackGroupRegularWidth, trackGroupHeight);
        }
        
        
    }

    // make the last track group (tracks in this group are NOT visualized)
    trackGroupContainers.add(new TrackGroupContainer(mainWindow, nTrackGroups-1, Colours::grey.brighter()));
    addAndMakeVisible(trackGroupContainers[nTrackGroups-1]);
    const int topY = (nTrackGroupsPerCol + 1) * (trackGroupHeight + spacingBetweenTrackGroups) + leftAndTopSpacing;
    const int topX = leftAndTopSpacing;
    trackGroupContainers[nTrackGroups-1]->setBounds(topX, topY, width, trackGroupHeight);

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
    // remove any previous TrackBoxes'
    // NOTE: fix this to actually remove the correct children
    // removeAllChildren();

    // calculate trackbox spacing
    const int spacing = 5;
    const int trackBoxHeight = 30;
    const int width = getWidth();
    const int height = getHeight();
    const int trackBoxWidth = 50;

    int nTracks = trackNames.size();
    // note that the index in the trackBoxes array is the same as the io port for the track
    for (int i=0; i<nTracks; ++i)
    {
        // add new track to the trackBoxes array
        // by default it is in group nTrackGroups (no group) and has a dark grey colour
        trackBoxes.add(new TrackBox(trackNames[i], Colours::grey.darker(), i, nTrackGroups));

        // make the new TrackBox component visible
        addAndMakeVisible(trackBoxes[i]);

        // for the first nTrackGroups-1 tracks added, we want to put them into a track group
        if (i < nTrackGroups-1)
        {
            trackGroupContainers[i]->addTrackToGroup(i);
            trackBoxes[i]->changeTrackGroup(i, trackGroupContainers[i]->getGroupColour());
            trackBoxes[i]->setTopLeftPosition(trackGroupContainers[i]->getPosition());
        }
        // for the other track boxes, put them in the unvisualized track group
        else
        {
            int xOffset = (i - nTrackGroups) * (trackBoxWidth + spacing);
            trackBoxes[i]->setTopLeftPosition(trackGroupContainers[nTrackGroups-1]->getPosition().translated(xOffset, 0));
        }
    }
}

void TrackSelector::switchTrackToNewGroup(int trackIndex, int oldTrackGroupIndex, int newTrackGroupIndex, Point<int> newPositionInGroupContainer)
{
    // remove the track from the the old TrackGroupContainer
    trackGroupContainers[oldTrackGroupIndex]->removeTrackFromGroup(trackIndex);

    // change the colour and group index of the TrackBox
    Colour newGroupColour = trackGroupContainers[newTrackGroupIndex]->getGroupColour();
    trackBoxes[trackIndex]->changeTrackGroup(newTrackGroupIndex, newGroupColour);

    // add the track to the new TrackGroupContainer
    trackGroupContainers[newTrackGroupIndex]->addTrackToGroup(trackIndex);

    // reposition the track
    trackBoxes[trackIndex]->setTopLeftPosition(trackGroupContainers[newTrackGroupIndex]->getPosition() + newPositionInGroupContainer);
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
                 constructorParams="MainWindow* mainWindow_" variableInitialisers="mainWindow(mainWindow_)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="700" initialHeight="400">
  <BACKGROUND backgroundColour="ff808080"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
