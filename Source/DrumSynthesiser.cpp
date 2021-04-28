/*
  ==============================================================================

    DrumSynthesiser.cpp
    Created: 27 Feb 2017 8:14:35am
    Author:  Vincent Choqueuse

  ==============================================================================
*/



#include "../JuceLibraryCode/JuceHeader.h"
#include "DrumSynthesiser.h"
#include "CustomSampler.h"

#define NB_SOUNDS_MAX 32

//==============================================================================
DrumSynthesiser::DrumSynthesiser():   Synthesiser(),sampleRate (0),
            lastNoteOnCounter (0),
            minimumSubBlockSize (32),
            subBlockSubdivisionIsStrict (false),
            shouldStealNotes (false)
            
{
    num_kit=0;
    int i;
    int base_note=36;

    nb_samples=BinaryData::namedResourceListSize;
    
    midiNoteNumber_playing=0;
    
    for (i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        {
        lastPitchWheelValues[i] = 0x2000;  
        }
        
    for (i = 0; i < 10; ++i)
        {
        addVoice (new CustomSamplerVoice());
        }
    
    clearSounds();
    
    for (i = 0; i < nb_samples; i++)
        {
        BigInteger Notes;
        Notes.setBit(i+base_note);
        addSound(new CustomSamplerSound(String(i),Notes,base_note+i,0.01,0.02,10.0));
        }
 
}

DrumSynthesiser::~DrumSynthesiser()
{
    
}


void DrumSynthesiser::loadKit()
{
    Logger::outputDebugString("DrumSynth_loadsound");
    for (int i = 0; i < nb_samples; i++)
    {
        SynthesiserSound::Ptr synthSound = getSound(i);
        CustomSamplerSound* sound{ dynamic_cast<CustomSamplerSound*> (synthSound.get()) };
        File audioFile = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile (String::formatted("kit%d/mysample%d.aif",num_kit,i+1));
        Logger::outputDebugString(audioFile.getFullPathName());
        sound->sample_index=i;
        sound->audioFile=audioFile;
        sound->loadSound();
    }

}





float DrumSynthesiser::getCurrentPosition(int midiRootNote)
{
    float audioPosition=0;
    
    for (int i = voices.size(); --i >= 0;)
    {
        if (voices.getUnchecked (i)->getCurrentlyPlayingNote() == midiRootNote)
        {
            CustomSamplerVoice* voice = dynamic_cast<CustomSamplerVoice*> (voices.getUnchecked (i));
            audioPosition=(voice->sourceSamplePosition)/voice->getSampleRate();
        }

    }
    return audioPosition;
}

