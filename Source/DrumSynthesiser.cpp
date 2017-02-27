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
        
        audioReader[0]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche1_aif,BinaryData::Tranche1_aifSize,false),true);
        audioReader[1]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche2_aif,BinaryData::Tranche2_aifSize,false),true);
        audioReader[2]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche3_aif,BinaryData::Tranche3_aifSize,false),true);
        audioReader[3]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche4_aif,BinaryData::Tranche4_aifSize,false),true);
        audioReader[4]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche5_aif,BinaryData::Tranche5_aifSize,false),true);
        audioReader[5]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche6_aif,BinaryData::Tranche6_aifSize,false),true);
        audioReader[6]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche7_aif,BinaryData::Tranche7_aifSize,false),true);
        audioReader[7]=aiffFormat.createReaderFor (new MemoryInputStream(BinaryData::Tranche8_aif,BinaryData::Tranche8_aifSize,false),true);
        
        for (int indice=0;indice<8;indice++)
        {

            Notes.setRange(indice+base_note,1, true);
            samplemap[indice]=new CustomSamplerSound(std::to_string(indice),*audioReader[indice],Notes,indice+base_note,0.001, 0.1,10.0);
            addSound(samplemap[indice]);
    
        }
        

    }
