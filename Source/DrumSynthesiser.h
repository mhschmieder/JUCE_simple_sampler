/*
  ==============================================================================

    DrumSynthesiser.h
    Created: 27 Feb 2017 8:14:35am
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#ifndef DRUMSYNTHESISER_H_INCLUDED
#define DRUMSYNTHESISER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "CustomSampler.h"


//==============================================================================
/*
*/
class DrumSynthesiser    : public Synthesiser
{
public:
    DrumSynthesiser();
    ~DrumSynthesiser();
    
    float getCurrentPosition(int midiRootNote);
    int midiNoteNumber_playing;
    void loadKit();
    int current_sound;
    int num_kit;
    int nb_samples;

private:

    double sampleRate;
    uint32 lastNoteOnCounter;
    int minimumSubBlockSize;
    bool subBlockSubdivisionIsStrict;
    bool shouldStealNotes;
    BigInteger sustainPedalsDown;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumSynthesiser)
};




#endif  // DRUMSYNTHESISER_H_INCLUDED
