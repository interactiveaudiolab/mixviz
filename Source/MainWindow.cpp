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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainWindow.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

#include "custom_jack_device.h"

Array<PropertyComponent*> MainWindow::createSettings(bool first)
{
    Array<PropertyComponent*> comps;

    // initialize constants to default values
    if (first)
    {
        numSpatialBinsValue.setValue("128");
        numTracksValue.setValue("4");
        intensityScalingConstantValue.setValue("150");
        intensityCutoffConstantValue.setValue("10");
        timeDecayConstantValue.setValue("0.94");
        maskingThresholdValue.setValue("1");
    }

    // add text fields to main window
    //comps.add (new TextPropertyComponent (numSpatialBinsValue, "Number of Spatial Bins (Fixed)", 20, false));
    //comps.add (new TextPropertyComponent (numFreqBinsValue, "Number of Frequency Bins (Fixed)", 20, false));
    comps.add (new TextPropertyComponent (numTracksValue, "Number of Stereo Tracks (4)", 20, false));
    comps.add (new TextPropertyComponent (intensityScalingConstantValue, "Intensity Constant (150)", 20, false));
    comps.add (new TextPropertyComponent (intensityCutoffConstantValue, "Intensity Cutoff (10)", 20, false));
    comps.add (new TextPropertyComponent (timeDecayConstantValue, "Time Decay Constant (0.95)", 20, false));
    comps.add (new TextPropertyComponent (maskingThresholdValue, "Masking Threshold (1)", 20, false));
    return comps;
}

Array<PropertyComponent*> MainWindow::createTracks(int numTracks)
{
    Array<PropertyComponent*> comps;
    TextPropertyComponent* comp;
    for (int i = 0; i < numTracks; ++i)
    {
        comp = new TextPropertyComponent (Value ("Name"+String(i)), "Track"+String(i), 20, false);
        comp->setColour(PropertyComponent::backgroundColourId, Colour((float) i / (float) numTracks, 0.8f, 1.0f, 1.0f));
        comps.add(comp);
    }
    return comps;
}

//[/MiscUserDefs]

//==============================================================================
MainWindow::MainWindow ()
{
    addAndMakeVisible (startButton = new TextButton ("Start"));
    startButton->setButtonText (TRANS("Apply Settings"));
    startButton->addListener (this);

    addAndMakeVisible (textEditor = new TextEditor ("new text editor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);


    //[UserPreSize]
	audioIODeviceType = createAudioIODeviceType_JACK_Custom();
	if (audioIODeviceType != nullptr)
	{
		audioIODeviceType->scanForDevices();
		StringArray deviceNames (audioIODeviceType->getDeviceNames());
     file:///home/jon/JUCE/html/classjuce_1_1BigInteger.html   textEditor->insertTextAtCaret(deviceNames[0]);
		audioIODevice = audioIODeviceType->createDevice(deviceNames[0],deviceNames[0]);
	} else {
		textEditor->insertTextAtCaret("Error, could not open Jack audio device. Is your Jack Server running?\n");
	}

    // make the settings panel for the visualizer
    addAndMakeVisible (settings = new PropertyPanel());
    settings->addSection ("Settings", createSettings(true));
    settings->addSection ("Tracks", createTracks(4));

    // makes a new visualizer with default settings
	addAndMakeVisible (visualizer = new Visualizer());
    audioIODevice->open(BigInteger(255),BigInteger(255),48000,1024);
    if (audioIODevice->isOpen())
    {
        textEditor->insertTextAtCaret("Audio device is open\n");
        audioIODevice->start(visualizer);
    } else {
        textEditor->insertTextAtCaret("Error, could not start Jack audio device.\n");
    }

    //[/UserPreSize]

    setSize (700, 1000);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainWindow::~MainWindow()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    startButton = nullptr;
    textEditor = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainWindow::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainWindow::resized()
{
    startButton->setBounds (408, 0, 150, 24);
    textEditor->setBounds (0, 0, 400, 24);
    //[UserResized] Add your own custom resize handling here..
    visualizer->setBounds(0,400,600,600);
    settings->setBounds(0,25,600,400);
    //[/UserResized]
}

void MainWindow::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == startButton)
    {
        //[UserButtonCode_startButton] -- add your button handler code here..
        // update visualizer settings
        const int numTracks = (int) numTracksValue.getValue();
        const int numSpatialBins = (int) numSpatialBinsValue.getValue();
        const float intensityScalingConstant = (float) intensityScalingConstantValue.getValue();
        const float intensityCutoffConstant = (float) intensityCutoffConstantValue.getValue();
        const double timeDecayConstant = (double) timeDecayConstantValue.getValue();
        const double maskingThreshold = (double) maskingThresholdValue.getValue();
        visualizer->changeSettings(numTracks, numSpatialBins, intensityScalingConstant, intensityCutoffConstant, timeDecayConstant, maskingThreshold);

        // build the panel with the tracks and their colors
        settings->clear();
        settings->addSection("Settings", createSettings(false));
        settings->addSection("Tracks", createTracks(numTracks));
        //[/UserButtonCode_startButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainWindow" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="700" initialHeight="1000">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTBUTTON name="Start" id="db72bd0eaa128ea6" memberName="startButton" virtualName=""
              explicitFocusOrder="0" pos="408 0 150 24" buttonText="Apply Settings"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="new text editor" id="6c76fb12e38dd58f" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="0 0 400 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
