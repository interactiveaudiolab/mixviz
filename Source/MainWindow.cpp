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
#include <iostream>
//[/Headers]

#include "MainWindow.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

#include "custom_jack_device.h"

// custom look and feel taken from JuceDemo
struct CustomLookAndFeel    : public LookAndFeel_V3
{
    void drawRoundThumb (Graphics& g, const float x, const float y,
                         const float diameter, const Colour& colour, float outlineThickness)
    {
        const Rectangle<float> a (x, y, diameter, diameter);
        const float halfThickness = outlineThickness * 0.5f;

        Path p;
        p.addEllipse (x + halfThickness, y + halfThickness, diameter - outlineThickness, diameter - outlineThickness);

        const DropShadow ds (Colours::black, 1, Point<int> (0, 0));
        ds.drawForPath (g, p);

        g.setColour (colour);
        g.fillPath (p);

        g.setColour (colour.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }

    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        Colour baseColour (backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                           .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

        if (isButtonDown || isMouseOverButton)
            baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

        const bool flatOnLeft   = button.isConnectedOnLeft();
        const bool flatOnRight  = button.isConnectedOnRight();
        const bool flatOnTop    = button.isConnectedOnTop();
        const bool flatOnBottom = button.isConnectedOnBottom();

        const float width  = button.getWidth() - 1.0f;
        const float height = button.getHeight() - 1.0f;

        if (width > 0 && height > 0)
        {
            const float cornerSize = jmin (15.0f, jmin (width, height) * 0.45f);
            const float lineThickness = cornerSize * 0.1f;
            const float halfThickness = lineThickness * 0.5f;

            Path outline;
            outline.addRoundedRectangle (0.5f + halfThickness, 0.5f + halfThickness, width - lineThickness, height - lineThickness,
                                         cornerSize, cornerSize,
                                         ! (flatOnLeft  || flatOnTop),
                                         ! (flatOnRight || flatOnTop),
                                         ! (flatOnLeft  || flatOnBottom),
                                         ! (flatOnRight || flatOnBottom));

            const Colour outlineColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                                                   : TextButton::textColourOffId));

            g.setColour (baseColour);
            g.fillPath (outline);

            if (! button.getToggleState())
            {
                g.setColour (outlineColour);
                g.strokePath (outline, PathStrokeType (lineThickness));
            }
        }
    }

    void drawTickBox (Graphics& g, Component& component,
                      float x, float y, float w, float h,
                      bool ticked,
                      bool isEnabled,
                      bool isMouseOverButton,
                      bool isButtonDown) override
    {
        const float boxSize = w * 0.7f;

        bool isDownOrDragging = component.isEnabled() && (component.isMouseOverOrDragging() || component.isMouseButtonDown());
        const Colour colour (component.findColour (TextButton::buttonColourId).withMultipliedSaturation ((component.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                             .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.7f));

        drawRoundThumb (g, x, y + (h - boxSize) * 0.5f, boxSize, colour,
                        isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

        if (ticked)
        {
            const Path tick (LookAndFeel_V2::getTickShape (6.0f));
            g.setColour (isEnabled ? findColour (TextButton::buttonOnColourId) : Colours::grey);

            const float scale = 9.0f;
            const AffineTransform trans (AffineTransform::scale (w / scale, h / scale)
                                             .translated (x - 2.5f, y + 1.0f));
            g.fillPath (tick, trans);
        }
    }

    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style, Slider& slider) override
    {
        const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

        bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
        Colour knobColour (slider.findColour (Slider::thumbColourId).withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                           .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = x + width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = y + height * 0.5f;
            }

            const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

            drawRoundThumb (g,
                            kx - sliderRadius,
                            ky - sliderRadius,
                            sliderRadius * 2.0f,
                            knobColour, outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle style, Slider& slider) override
    {
        g.fillAll (slider.findColour (Slider::backgroundColourId));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            const float fx = (float) x, fy = (float) y, fw = (float) width, fh = (float) height;

            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle (fx, sliderPos, fw, 1.0f + fh - sliderPos);
            else
                p.addRectangle (fx, fy, sliderPos - fx, fh);


            Colour baseColour (slider.findColour (Slider::rotarySliderFillColourId)
                               .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                               .withMultipliedAlpha (0.8f));

            g.setColour (baseColour);
            g.fillPath (p);

            const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height,
                                     float /*sliderPos*/,
                                     float /*minSliderPos*/,
                                     float /*maxSliderPos*/,
                                     const Slider::SliderStyle /*style*/, Slider& slider) override
    {
        const float sliderRadius = getSliderThumbRadius (slider) - 5.0f;
        Path on, off;

        if (slider.isHorizontal())
        {
            const float iy = x + width * 0.5f - sliderRadius * 0.5f;
            Rectangle<float> r (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius);
            const float onW = r.getWidth() * ((float) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromLeft (onW));
            off.addRectangle (r);
        }
        else
        {
            const float ix = x + width * 0.5f - sliderRadius * 0.5f;
            Rectangle<float> r (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius);
            const float onH = r.getHeight() * ((float) slider.valueToProportionOfLength (slider.getValue()));

            on.addRectangle (r.removeFromBottom (onH));
            off.addRectangle (r);
        }

        g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
        g.fillPath (on);

        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillPath (off);
    }

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
    {
        const float radius = jmin (width / 2, height / 2) - 2.0f;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, 0.0);
            g.fillPath (filledArc);
        }

        {
            const float lineThickness = jmin (15.0f, jmin (width, height) * 0.45f) * 0.1f;
            Path outlineArc;
            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, 0.0);
            g.strokePath (outlineArc, PathStrokeType (lineThickness));
        }
    }
};
// **********************************
// **********************************
// END CUSTOM LOOK AND FEEL
// **********************************
// **********************************
//

//[/MiscUserDefs]

//==============================================================================
MainWindow::MainWindow ()
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (700, 1000);


    //[Constructor] You can add your own custom stuff here..
    // set custom look and feel
    CustomLookAndFeel* claf = new CustomLookAndFeel();
    LookAndFeel::setDefaultLookAndFeel(claf);

    // create Jack audio IO device
    audioIODeviceType = createAudioIODeviceType_JACK_Custom();
    if (audioIODeviceType != nullptr)
    {
        audioIODeviceType->scanForDevices();
        StringArray deviceNames (audioIODeviceType->getDeviceNames());
     file:///home/jon/JUCE/html/classjuce_1_1BigInteger.html   textEditor->insertTextAtCaret(deviceNames[0]);
        audioIODevice = audioIODeviceType->createDevice(deviceNames[0],deviceNames[0]);
    } else {
        // throw an error for no jack audio device
    }

    // makes a new visualizer with default settings
    addAndMakeVisible (visualizer = new Visualizer());
    visualizer->setBounds(0,0,700,600);
    audioIODevice->open(BigInteger(255),BigInteger(255),44100,1024);
    if (audioIODevice->isOpen())
    {
        audioIODevice->start(visualizer);
    } else {
        // throw an error somehow right here
    }

    /* MOVE INTO SEPARATE COMPONENT
    // add the visualizer's settings panel with sliders
    addAndMakeVisible (intensityScalingConstantSlider = new Slider());
    addAndMakeVisible (intensityScalingConstantLabel = new Label(String("isc"), String("Intensity Scaling Constant")));
    intensityScalingConstantSlider->setSliderStyle (Slider::LinearBar);
    intensityScalingConstantSlider->setRange (1, 15000);
    intensityScalingConstantSlider->setValue (5000);
    intensityScalingConstantSlider->addListener (this);
    intensityScalingConstantSlider->setBounds(100,600,100,65);
    intensityScalingConstantLabel->setBounds(100,670,100,65);

    addAndMakeVisible (intensityCutoffConstantSlider = new Slider());
    addAndMakeVisible (intensityCutoffConstantLabel = new Label(String("icc"), String("Intensity Cutoff Constant")));
    intensityCutoffConstantSlider->setSliderStyle (Slider::LinearBar);
    intensityCutoffConstantSlider->setRange (1, 50);
    intensityCutoffConstantSlider->setValue (10);
    intensityCutoffConstantSlider->addListener (this);
    intensityCutoffConstantSlider->setBounds(250,600,100,65);
    intensityCutoffConstantLabel->setBounds(250,670,100,65);

    addAndMakeVisible (timeDecayConstantSlider = new Slider());
    addAndMakeVisible (timeDecayConstantLabel = new Label(String("tdc"), String("Time Decay Constant")));
    timeDecayConstantSlider->setSliderStyle (Slider::LinearBar);
    timeDecayConstantSlider->setRange (0, 0.99);
    timeDecayConstantSlider->setValue (0.70);
    timeDecayConstantSlider->addListener (this);
    timeDecayConstantSlider->setBounds(400,600,100,65);
    timeDecayConstantLabel->setBounds(400,670,100,65);

    addAndMakeVisible (maskingThresholdSlider = new Slider());
    addAndMakeVisible (maskingThresholdLabel = new Label(String("mt"), String("Masking Threshold")));
    maskingThresholdSlider->setSliderStyle (Slider::LinearBar);
    maskingThresholdSlider->setRange (0.1, 6);
    maskingThresholdSlider->setValue (2);
    maskingThresholdSlider->addListener (this);
    maskingThresholdSlider->setBounds(550,600,100,65);
    maskingThresholdLabel->setBounds(550,670,100,65);
    */

    // add the tracks panel
    addAndMakeVisible (loadTracksButton = new TextButton("load"));
    loadTracksButton->setButtonText (TRANS("Load track names"));
    loadTracksButton->addListener (this);
    loadTracksButton->setBounds(0, 600, 100, 20);

    addAndMakeVisible (trackSelector = new TrackSelector(this));
    trackSelector->setTopLeftPosition(0,620);
    //[/Constructor]
}

MainWindow::~MainWindow()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainWindow::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::grey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainWindow::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainWindow::printMe()
{
    String name = visualizer->getName();
    std::cout << "MainWindow::printMe: visualizer name: " << name << std::endl;
}

void MainWindow::updateVisualizerTracksInGroup (int groupIndex, Array<int> tracksInGroup)
{
    visualizer->updateTracksInGroup(groupIndex, tracksInGroup);
}

void MainWindow::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == loadTracksButton)
    {
        FileChooser myChooser ("Please select the .xml file generated by Jack...",
                               File::getSpecialLocation (File::userHomeDirectory),
                               "*.xml");
        std::cout << "Helloooooo" << std::endl;
        if (myChooser.browseForFileToOpen())
        {
            // load the chosen file and get output sockets
            File jackFile (myChooser.getResult());
            String text = jackFile.loadFileAsString();
            String textWithNames = text.fromFirstOccurrenceOf("<cables>", 0, 1).upToFirstOccurrenceOf("</cables>", 0, 1);

            // initialize empty hashmap
            // key is MixVis input socket
            // value is track name
            StringArray trackNames;

            // while there are still output sockets
            while (textWithNames.containsWholeWord("cable"))
            {
                // first thing should be a cable with name=
                String cableText = textWithNames.fromFirstOccurrenceOf("output=",0,1).upToFirstOccurrenceOf(" type=",0,1).unquoted();
                String name = cableText.upToFirstOccurrenceOf(" input=", 0, 1).unquoted();
                int inputSocket = cableText.fromFirstOccurrenceOf(" input=\"Input Socket ", 0, 1).upToFirstOccurrenceOf("\"", 0, 1).getIntValue();

                std::cout << inputSocket << ", " << name << std::endl;
                // insert track name, adjust for 0 indexing
                trackNames.insert(inputSocket-1, name);
                textWithNames = textWithNames.fromFirstOccurrenceOf("/>",0,1);
            }

            // display the tracks with these new names
            trackSelector->makeTrackBoxes(trackNames);
        }
    }
}

void MainWindow::sliderValueChanged (Slider*)
{
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainWindow" componentName=""
                 parentClasses="public Component, public ButtonListener, public SliderListener"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="700"
                 initialHeight="1000">
  <BACKGROUND backgroundColour="ff808080"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
