/*
  ==============================================================================

    GUI.h
    Created: 28 Feb 2017 9:32:16am
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrumSynthesiser.h"
#include "CustomMidiKeyboardComponent.h"

//==============================================================================

class OtherLookAndFeel : public LookAndFeel_V3
{
public:
    
    OtherLookAndFeel()
    {
        
    }
    
    void drawLinearSliderBackground (Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider &) override
    {
        
    }
    
    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style, Slider& slider) override
    {
        Colour knobColour;
        const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);
        
        bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
        
        if (isDownOrDragging)
        {
            knobColour=Colour(255,101,0);
        }
        else
        {
            knobColour=Colour(129,140,150);
        }
        
        
        const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

        if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
            {
      
                drawGlassPointer (g, minSliderPos,
                                  jmin (y + height - sliderRadius * 2.0f, y* 0.1f),
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 1);
                
                drawGlassPointer (g, maxSliderPos,
                                  jmin (y + height - sliderRadius * 2.0f, y * 0.1f),
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 3);
            }
        
    }
    
    void drawGlassPointer (Graphics& g,
                      const float x, const float y, const float diameter,
                      const Colour& colour, const float outlineThickness,
                      const int direction) noexcept
    {
        if (diameter <= outlineThickness)
            return;
        
        Colour border=Colour(14,18,20);

        Path p;
        p.startNewSubPath (x + diameter * 0.5f, y);
        p.lineTo (x + diameter, y + diameter * 0.8f);
        p.lineTo (x + diameter, y + diameter);
        p.lineTo (x, y + diameter);
        p.lineTo (x, y + diameter * 0.8f);
        p.closeSubPath();
        
        p.applyTransform (AffineTransform::rotation (direction * (float_Pi * 0.5f), x + diameter * 0.5f, y + diameter * 0.5f));

        if (direction==3)
        {
            p.applyTransform (AffineTransform::translation(-diameter, 0));
        }
        g.setColour (colour);
        g.fillPath (p);
        
        g.setColour (border);
        //g.strokePath (p, PathStrokeType (outlineThickness));
        g.setColour (colour);
        g.drawLine (x, y, x,y+100, 2.0f);
    
    }
};


class PadLookAndFeel : public LookAndFeel_V3
{
    public:
    PadLookAndFeel()
    {
        setColour (Slider::rotarySliderFillColourId, Colours::red);
    }
};


class CustomSlider : public Component
{
    public:
    CustomSlider(const String& componentName,const String& suffix,int x,int y,bool textbox);
    
    Slider slider;
    Label sliderlabel;
    
    
};

class CustomToggleButton: public Component
{
    public:
    CustomToggleButton(const String& componentName,const String& suffix,int x,int y);
    
    ToggleButton button;
    Label buttonlabel;
    
};

class CustomComboBox : public Component
{
    public:
    CustomComboBox(const String& componentName,const String& suffix,int x,int y);
    ComboBox comboBox;
    Label comboBoxlabel;
};



struct SamplerPage  :
public Component,
private Slider::Listener,
private Timer,
private ComboBox::Listener,
private Button::Listener
{
    SamplerPage(DrumSynthesiser& synth_,CustomMidiKeyboardComponent *keyboardComponent);
    
    void comboBoxChanged (ComboBox* combobox) override;
    void sliderValueChanged (Slider* slider) override;
    void buttonStateChanged(Button* button) override;
    void buttonClicked (Button * button) override;
    void paint (Graphics& g) override;
    void start_timer();
    void stop_timer();
    void timerCallback() override;
    
    int selected_slot;
    int nb_samples;
    
    
    OtherLookAndFeel otherLookAndFeel;
    Slider cutoffSlider;
    Slider QSlider;
    Label cutoffSliderLabel;
    DrumSynthesiser& synth;
    CustomMidiKeyboardComponent *keyboardComponent;
    CustomSlider dialp,filter_dialf,filter_dialr;
    Slider slider_ss;
    CustomComboBox samplecomboBox,filter_comboBox,bankcomboBox;
    CustomToggleButton filter_button;
    AudioThumbnail* thumbnail;
    CustomSamplerSound *sampler_sound;
    PadLookAndFeel padLookAndFeel;
    
    double startTime;
};



#endif  // GUI_H_INCLUDED
