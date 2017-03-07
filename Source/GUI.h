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

//==============================================================================
/*
*/



struct KitPage  :  public Component,
                       public Slider::Listener,
                       private ComboBoxListener
{
    KitPage(DrumSynthesiser& synth_);
    void sliderValueChanged (Slider* slider) override;
    void comboBoxChanged (ComboBox* combobox) override;

    int nb_samples;
    ComboBox comboBox[8];
    Slider pitchSlider[8];
    DrumSynthesiser& synth;

};

struct SamplerPage  :  public Component,
public Slider::Listener,
private ComboBoxListener,
private Timer
{
    SamplerPage(DrumSynthesiser& synth_);
    void sliderValueChanged (Slider* slider) override;
    void comboBoxChanged (ComboBox* combobox) override;
    void paint (Graphics& g) override;
    void timerCallback() override;
    
    int nb_samples;
    ComboBox comboBox;
    Slider cutoffSlider;
    Slider QSlider;
    DrumSynthesiser& synth;
    
};


struct EffectPage  :  public Component,
public Slider::Listener,
private ComboBoxListener
{
    EffectPage(DrumSynthesiser& synth_);
    void sliderValueChanged (Slider* slider) override;
    void comboBoxChanged (ComboBox* combobox) override;
    
    int nb_samples;
    ComboBox comboBox[8];
    Slider cutoffSlider[8];
    Slider QSlider[8];
    DrumSynthesiser& synth;
    
};


#endif  // GUI_H_INCLUDED
