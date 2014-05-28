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

Array<PropertyComponent*> MainWindow::createSettings()
{
    Array<PropertyComponent*> comps;

    // initialize constants to default values
    numSpatialBinsValue.setValue("128");
    numFreqBinsValue.setValue("40");
    numTracksValue.setValue("2");
    intensityScalingConstantValue.setValue("100");
    intensityCutoffConstantValue.setValue("5");
    timeDecayConstantValue.setValue("0.5");

    // add text fields to main window
    comps.add (new TextPropertyComponent (numSpatialBinsValue, "Number of Spatial Bins (128)", 20, false));
    comps.add (new TextPropertyComponent (numFreqBinsValue, "Number of Frequency Bins (40)", 20, false));
    comps.add (new TextPropertyComponent (numTracksValue, "Number of Stereo Tracks (2)", 20, false));
    comps.add (new TextPropertyComponent (intensityScalingConstantValue, "Intensity Constant (100)", 20, false));
    comps.add (new TextPropertyComponent (intensityCutoffConstantValue, "Intensity Cutoff (5)", 20, false));
    comps.add (new TextPropertyComponent (timeDecayConstantValue, "Time Decay Constant (0.5)", 20, false));
    //comps.add (new VisualizerButtonPropertyComponent ("Click to Apply Settings"));
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
        textEditor->insertTextAtCaret(deviceNames[0]);
		audioIODevice = audioIODeviceType->createDevice(deviceNames[0],deviceNames[0]);
	} else {
		textEditor->insertTextAtCaret("Error, could not open Jack audio device. Is your Jack Server running?\n");
	}

    // make the settings panel for the visualizer
    addAndMakeVisible (settings = new PropertyPanel());
    settings->addSection ("Settings", createSettings());

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

    setSize (600, 900);


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
    visualizer->setBounds(0,299,600,600);
    settings->setBounds(0,25,600,200);
    //[/UserResized]
}

void MainWindow::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == startButton)
    {
        //[UserButtonCode_startButton] -- add your button handler code here..
        const int numTracks = (int) numTracksValue.getValue();
        const int numSpatialBins = (int) numSpatialBinsValue.getValue();
        const int numFreqBins = (int) numFreqBinsValue.getValue();
        const float intensityScalingConstant = (float) intensityScalingConstantValue.getValue();
        const float intensityCutoffConstant = (float) intensityCutoffConstantValue.getValue();
        const double timeDecayConstant = (double) timeDecayConstantValue.getValue();
        visualizer->changeSettings(numTracks, numSpatialBins, numFreqBins, intensityScalingConstant, intensityCutoffConstant, timeDecayConstant);
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
                 fixedSize="0" initialWidth="600" initialHeight="900">
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
