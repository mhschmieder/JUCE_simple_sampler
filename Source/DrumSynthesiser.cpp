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
    int i;
    int base_note=36;
    
    BigInteger Notes;

    nb_samples=BinaryData::namedResourceListSize;
    
    for (i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        {
        lastPitchWheelValues[i] = 0x2000;  
        }
        
    for (i = 0; i < 15; ++i)
            {
            addVoice (new CustomSamplerVoice());
            }
    
    clearSounds();

    for (i = 0; i < nb_samples; i++)
        {
        Notes.setRange(i+base_note,1, true);
        samplemap[i]=new CustomSamplerSound(std::to_string(i),Notes,i+base_note,0.01,0.2,10.0);
        addSound(samplemap[i]);
        }
    
 
}

DrumSynthesiser::~DrumSynthesiser()
{
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

