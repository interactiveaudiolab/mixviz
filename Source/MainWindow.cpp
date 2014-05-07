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
//[/MiscUserDefs]

//==============================================================================
MainWindow::MainWindow ()
{
    addAndMakeVisible (textEditor = new TextEditor ("new text editor"));
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String::empty);


    //[UserPreSize]
	audioIODeviceType = AudioIODeviceType::createAudioIODeviceType_JACK();
	if (audioIODeviceType != nullptr)
	{
		audioIODeviceType->scanForDevices();
		StringArray deviceNames (audioIODeviceType->getDeviceNames());
		for (int i=0; i<deviceNames.size(); ++i) {
		    textEditor->insertTextAtCaret("device: ");
			textEditor->insertTextAtCaret(deviceNames[i]);
			textEditor->insertTextAtCaret("\n");
			audioIODevice = audioIODeviceType->createDevice(deviceNames[i],deviceNames[i]);
		}
	} else {
		textEditor->insertTextAtCaret("Error, could not open Jack audio device. Is your Jack Server running?\n");
	}
	
	addAndMakeVisible (visualizer = new Visualizer());
    audioIODevice->open(BigInteger(3),BigInteger(12),48000,1024);
    if (audioIODevice->isOpen())
    {
        textEditor->insertTextAtCaret("Audio device is open\n");
        audioIODevice->start(visualizer);
    } else {
        textEditor->insertTextAtCaret("Error, could not start Jack audio device.\n");
    }

    //[/UserPreSize]

    setSize (600, 600);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainWindow::~MainWindow()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

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
    textEditor->setBounds (0, 0, 600, 90);
    //[UserResized] Add your own custom resize handling here..
    visualizer->setBounds(0,100,600,600);
    //[/UserResized]
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
                 fixedSize="0" initialWidth="600" initialHeight="600">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTEDITOR name="new text editor" id="7c999f9f8f057714" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="0 0 600 90" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
