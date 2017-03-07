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


KitPage::KitPage(DrumSynthesiser& synth_)
:synth(synth_)
    {
        //const char *Filter_Type[3]={"Lowpass","Highpass","Bandpass"};
        
        nb_samples=BinaryData::namedResourceListSize;
        
        for (int numComb=0;numComb<nb_samples;numComb++)
        {
            comboBox[numComb].setBounds (8, 8+24*numComb, 100, 24);
            addAndMakeVisible (comboBox[numComb]);
            
            comboBox[numComb].setEditableText (true);
            comboBox[numComb].setJustificationType (Justification::centred);
        
            for (int indice=0;indice<nb_samples;indice++)
            {
                comboBox[numComb].addItem (BinaryData::namedResourceList[indice], indice+1);
            }
            
            comboBox[numComb].addListener (this);
            comboBox[numComb].setSelectedId (numComb+1,sendNotificationAsync );
            pitchSlider[numComb].setBounds (120, 8+24*numComb, 300, 24);
            addAndMakeVisible (pitchSlider[numComb]);
            pitchSlider[numComb].setRange(-24,24);
            pitchSlider[numComb].addListener(this);
        }
      
    }
    
void KitPage::sliderValueChanged (Slider* slider)
    {
        for (int indice=0;indice<nb_samples;indice++)
        {
            if (slider == &(pitchSlider[indice]))
            {
                int pitch=pitchSlider[indice].getValue();
                synth.samplemap[indice]->detune=pitch;
            }
        }
    }

void KitPage::comboBoxChanged (ComboBox* combobox)
{
    for (int index_samplemap=0;index_samplemap<nb_samples;index_samplemap++)
    {
        if (combobox == &(comboBox[index_samplemap]))
        {
            int index_binary=comboBox[index_samplemap].getSelectedId()-1;
            synth.allNotesOff(1,0);
            synth.samplemap[index_samplemap]->loadSound(index_binary);
            repaint();
        }
    }
}




SamplerPage::SamplerPage(DrumSynthesiser& synth_)
:synth(synth_)
{
   
    
    nb_samples=BinaryData::namedResourceListSize;
    startTimer (40);
    
}

void SamplerPage::sliderValueChanged (Slider* slider)
{
    
}

void SamplerPage::comboBoxChanged (ComboBox* combobox)
{
 
}

void SamplerPage::paint (Graphics& g)
{

        const Rectangle<int> thumbnailBounds (8,8, getWidth() - 16, 100);
        AudioThumbnail* thumbnail=&synth.samplemap[0]->thumbnail;
        
        if (thumbnail->getNumChannels() == 0)
        {
            g.setColour (Colours::black);
            g.fillRect (thumbnailBounds);
            g.setColour (Colours::lightyellow);
            g.drawFittedText ("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);
        }
        else
        {
            const double audioLength (thumbnail->getTotalLength());
            g.setColour (Colours::black);
            g.fillRect (thumbnailBounds);
            g.setColour (Colours::lightyellow);                                     // [8]
            thumbnail->drawChannel (g,thumbnailBounds, 0.0,thumbnail->getTotalLength(), 1,1.0f);
            
            
            const double audioPosition (synth.getCurrentPosition(synth.samplemap[0]->midiRootNote));
            const float drawPosition ((audioPosition / audioLength) * thumbnailBounds.getWidth() + thumbnailBounds.getX());
            g.drawLine (drawPosition, thumbnailBounds.getY(), drawPosition,thumbnailBounds.getBottom(), 2.0f);
        }
    
}

void SamplerPage::timerCallback()
{
    repaint();
}




EffectPage::EffectPage(DrumSynthesiser& synth_)
:synth(synth_)
{
    const char *Filter_Type[3]={"Lowpass","Highpass","Bandpass"};
    
    nb_samples=BinaryData::namedResourceListSize;

    for (int numComb=0;numComb<nb_samples;numComb++)
    {
        comboBox[numComb].setBounds (8, 8+24*numComb, 100, 24);
        addAndMakeVisible (comboBox[numComb]);
        
        comboBox[numComb].setEditableText (true);
        comboBox[numComb].setJustificationType (Justification::centred);
        
        for (int indice=0;indice<3;indice++)
        {
            comboBox[numComb].addItem (Filter_Type[indice], indice+1);
        }
        
        comboBox[numComb].addListener (this);
        comboBox[numComb].setSelectedId (1,sendNotificationAsync);
        
        cutoffSlider[numComb].setBounds (120, 8+24*numComb, 200, 24);
        addAndMakeVisible (cutoffSlider[numComb]);
        cutoffSlider[numComb].setRange(100,20000);
        cutoffSlider[numComb].addListener(this);
        
        QSlider[numComb].setBounds (340, 8+24*numComb, 200, 24);
        addAndMakeVisible (QSlider[numComb]);
        QSlider[numComb].setRange(60,20000);
        QSlider[numComb].addListener(this);
    }
    
}

void EffectPage::sliderValueChanged (Slider* slider)
{
    for (int indice=0;indice<nb_samples;indice++)
    {
        if (slider == &(cutoffSlider[indice]))
        {
            synth.samplemap[indice]->filter_cutoff=cutoffSlider[indice].getValue();
            synth.samplemap[indice]->setFilter();
        }
    }
}

void EffectPage::comboBoxChanged (ComboBox* combobox)
{
    for (int indice=0;indice<nb_samples;indice++)
    {
        if (combobox == &(comboBox[indice]))
        {
            synth.samplemap[indice]->filter_type=comboBox[indice].getSelectedId();
            synth.samplemap[indice]->setFilter();
        }
    }
}



