/*
  ==============================================================================

    GUI.cpp
    Created: 28 Feb 2017 9:32:16am
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "GUI.h"
#include "DrumSynthesiser.h"

//==============================================================================

CustomSlider::CustomSlider(const String& componentName,const String& suffix,int x,int y,bool textbox)
{
    slider.setBounds (x+80,y,250,24);
    slider.setTextValueSuffix (suffix);
    if (textbox==true)
    {
        slider.setTextBoxStyle (Slider::TextBoxRight, true,80,24);
        slider.setColour(Slider::textBoxOutlineColourId,Colour(0));
        slider.setColour(Slider::textBoxTextColourId,Colour(24,30,35));
    }
    else
    {
        slider.setTextBoxStyle (Slider::NoTextBox, true,80,24);
    }
    sliderlabel.setText(componentName, dontSendNotification);
    sliderlabel.attachToComponent (&slider, true);
    sliderlabel.setColour(Label::textColourId, Colour(24,30,35));
    slider.setColour(Slider::thumbColourId,Colour(255,201,7));
   
}


CustomToggleButton::CustomToggleButton(const String& componentName,const String& suffix,int x,int y)
{
    button.setBounds (x+80,y,250,24);
    buttonlabel.setText(componentName, dontSendNotification);
    buttonlabel.attachToComponent (&button, true);
    buttonlabel.setColour(Label::textColourId, Colour(24,30,35));
}

CustomComboBox::CustomComboBox(const String& componentName,const String& suffix,int x,int y)
{
    comboBox.setBounds (x+80,y,250,24);
    comboBoxlabel.setText(componentName, dontSendNotification);
    comboBoxlabel.attachToComponent (&comboBox, true);
    comboBoxlabel.setColour(Label::textColourId, Colour(24,30,35));
}


SamplerPage::SamplerPage(DrumSynthesiser& synth_,CustomMidiKeyboardComponent *keyboardComponent_)
:   synth(synth_),
    keyboardComponent(keyboardComponent_),
    dialp("Pitch","",420,210,true),
    samplecomboBox("Sample","",420,180),
    bankcomboBox("Bank","",420,150),
    filter_comboBox("Filter","",420,300),
    filter_button("Type","",420,270),
    filter_dialf("Frequency","Hz",420,330,true),
    filter_dialr("Res","dB",420,360,true)
{
    
    nb_samples=BinaryData::namedResourceListSize;

    
    //synth.addListener(this);
    
    //add bank combo box items
    for (int indice=0;indice<16;indice++)
    {
        bankcomboBox.comboBox.addItem ("Slot"+String(indice), indice+1);
    }
    
    //add filter combo box items
    filter_comboBox.comboBox.addItem ("Lowpass", 1);
    filter_comboBox.comboBox.addItem ("Highpass", 2);
    filter_comboBox.comboBox.addItem ("Bandpass", 3);
    
    //add sample combo box items
    for (int indice=0;indice<nb_samples;indice++)
    {
        samplecomboBox.comboBox.addItem (BinaryData::namedResourceList[indice], indice+1);
    }
    
    addAndMakeVisible (bankcomboBox.comboBox);
    addAndMakeVisible(bankcomboBox.comboBoxlabel);
    bankcomboBox.comboBox.addListener(this);
    
    addAndMakeVisible (samplecomboBox.comboBox);
    addAndMakeVisible(samplecomboBox.comboBoxlabel);
    samplecomboBox.comboBox.addListener(this);
    
    dialp.slider.setRange (-24, 24);
    addAndMakeVisible (dialp.slider);
    addAndMakeVisible(dialp.sliderlabel);
    dialp.slider.addListener(this);
    
    slider_ss.setLookAndFeel (&otherLookAndFeel);
    slider_ss.setBounds(0, 18, 782, 100);
    slider_ss.setSliderStyle(Slider::TwoValueHorizontal);
    slider_ss.setRange(0.,1.,0.001);
    slider_ss.setMaxValue(1.);
    slider_ss.setTextBoxStyle (Slider::NoTextBox, true,80,24);
    slider_ss.addListener(this);
    addAndMakeVisible (slider_ss);
    Logger::outputDebugString(String(getRight()));
    
    addAndMakeVisible (filter_button.button);
    addAndMakeVisible (filter_button.buttonlabel);
    filter_button.button.addListener(this);
    
    addAndMakeVisible (filter_comboBox.comboBox);
    addAndMakeVisible(filter_comboBox.comboBoxlabel);
    filter_comboBox.comboBox.addListener(this);
    filter_dialf.slider.setRange (50, 20000);
    
    addAndMakeVisible (filter_dialf.slider);
    addAndMakeVisible(filter_dialf.sliderlabel);
    filter_dialf.slider.addListener(this);
    
    filter_dialr.slider.setRange (0.1, 10);
    addAndMakeVisible(filter_dialr.slider);
    addAndMakeVisible(filter_dialr.sliderlabel);
    

    addAndMakeVisible (keyboardComponent);
    keyboardComponent->setAvailableRange(36,36+16);
    
    keyboardComponent->setBounds(8,150,300,300);
    keyboardComponent->setKeyWidth();
    
    sampler_sound=nullptr;
}
void SamplerPage::start_timer()
{
    if (isTimerRunning ()==false)
        {
        startTimer (40);
        }
}
void SamplerPage::stop_timer()
{
    stopTimer();
}

void SamplerPage::comboBoxChanged (ComboBox* combobox)
{

    if (combobox==&bankcomboBox.comboBox)
    {
        selected_slot=combobox->getSelectedId()-1;
        CustomSamplerSound* sound{ dynamic_cast<CustomSamplerSound*> (synth.getSound(selected_slot).get()) };
        sampler_sound = sound;
        Logger::outputDebugString(sampler_sound->audioFile.getFullPathName());
        samplecomboBox.comboBox.setSelectedItemIndex(sampler_sound->sample_index,dontSendNotification);
        dialp.slider.setValue(sampler_sound->detune,dontSendNotification);
        filter_button.button.setToggleState(sampler_sound->filter_active,dontSendNotification);
        filter_dialf.slider.setValue(sampler_sound->filter_cutoff,dontSendNotification);
        filter_comboBox.comboBox.setSelectedItemIndex(sampler_sound->filter_type-1,dontSendNotification);
        slider_ss.setMinAndMaxValues(sampler_sound->sample_start,sampler_sound->sample_end );
        repaint();
    }
    
    
    if (combobox == &samplecomboBox.comboBox)
        {
        int selected_sample=combobox->getSelectedId()-1;

        String filename=String::formatted ("kit%d/mysample%d.aif",synth.num_kit,selected_sample+1);
        File audioFile = File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile (filename);
            
        CustomSamplerSound* sound{ dynamic_cast<CustomSamplerSound*> (synth.getSound(selected_slot).get()) };
        sampler_sound = sound;
        sampler_sound->sample_index=selected_sample+1;
        sampler_sound->audioFile=audioFile;
        sampler_sound->loadSound();
            
        repaint();
        }
    
    if (combobox == &filter_comboBox.comboBox)
    {
        int type=combobox->getSelectedId();
        sampler_sound->filter_type=type;
    }
    

}

void SamplerPage::sliderValueChanged (Slider* slider)
{
    if (sampler_sound != NULL)
    {

    if (slider == &dialp.slider)
    {
        sampler_sound->detune=slider->getValue();
    }
    if  (slider == &filter_dialf.slider)
    {
        sampler_sound->filter_cutoff=slider->getValue();
    }
    if ( slider == &slider_ss)
    {
        sampler_sound->sample_start=slider->getMinValueObject().getValue();
        sampler_sound->sample_end=slider->getMaxValueObject().getValue();
        repaint();
    }
    }

    
}


void SamplerPage::buttonStateChanged(Button* button)
{

}

void SamplerPage::buttonClicked(Button* button)
{
    if (button == &filter_button.button)
    {
        int value=button->getToggleStateValue().getValue();
        sampler_sound->filter_active=value;
    }

}




void SamplerPage::paint (Graphics& g)
{
        const Rectangle<int> thumbnailBounds (8,18, getWidth() - 16, 100);
        Colour wav_back_colour=Colour(24,30,35);
        Colour wav_colour=Colour(255,201,7);
        Colour wav_tick=Colour(255,101,0);
        Colour wav_separator=Colour(255,50,0);
        Colour pad_off=Colour(110,110,110);
    
    
        if (sampler_sound != NULL)
        {
    
        if (sampler_sound->thumbnail.getNumChannels() != 0)
        {
            const double audioLength (sampler_sound->thumbnail.getTotalLength());
            g.setColour (wav_back_colour);
            g.fillRect (thumbnailBounds);
            g.setColour (wav_colour);
            sampler_sound->thumbnail.drawChannel (g,thumbnailBounds, 0.0,sampler_sound->thumbnail.getTotalLength(), 1,1.0f);
            
            const double audioPosition (synth.getCurrentPosition(sampler_sound->midiRootNote));
            const float drawPosition ((audioPosition / audioLength) * thumbnailBounds.getWidth() + thumbnailBounds.getX());

            if (audioPosition > 0)
            {
                g.setColour (wav_tick);
                g.drawLine (drawPosition, thumbnailBounds.getY(), drawPosition,thumbnailBounds.getBottom(), 2.0f);
            }
            g.setColour (wav_separator);
        }
        }
        else{
            g.setColour (wav_back_colour);
            g.fillRect (thumbnailBounds);
            g.setColour (wav_colour);
            g.drawFittedText ("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);
        }
    

}

void SamplerPage::timerCallback()
{
    repaint();
}






