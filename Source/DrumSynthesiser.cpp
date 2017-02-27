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

//==============================================================================
DrumSynthesiser::DrumSynthesiser():   sampleRate (0),
            lastNoteOnCounter (0),
            minimumSubBlockSize (32),
            subBlockSubdivisionIsStrict (false),
            shouldStealNotes (true)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    
    for (int i = 0; i < numElementsInArray (lastPitchWheelValues); ++i)
        {
        lastPitchWheelValues[i] = 0x2000;  
        }
        
      for (int i = 0; i < 15; ++i)
            {
            addVoice (new CustomSamplerVoice());
            }
      loadSound();
}

DrumSynthesiser::~DrumSynthesiser()
{
}



void DrumSynthesiser::loadSound()
    {
        int base_note=36;
        AiffAudioFormat aiffFormat;
        
        ScopedPointer<AudioFormatReader> audioReader[8];
        BigInteger Notes;
        
        clearSounds();
        

        int dataSizeInBytes;
        
        for (int indice=0;indice<8;indice++)
        {

            const char* ressource=BinaryData::getNamedResource (BinaryData::namedResourceList[indice], dataSizeInBytes);
            
            audioReader[indice]=aiffFormat.createReaderFor (new MemoryInputStream(ressource,dataSizeInBytes,false),true);
            Notes.setRange(indice+base_note,1, true);
            samplemap[indice]=new CustomSamplerSound(std::to_string(indice),*audioReader[indice],Notes,indice+base_note,0.001, 0.1,10.0);
            addSound(samplemap[indice]);
    
        }
        

    }
