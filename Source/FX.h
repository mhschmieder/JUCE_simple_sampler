/*
  ==============================================================================

    FX.h
    Created: 5 Mar 2017 3:59:51pm
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#ifndef FX_H_INCLUDED
#define FX_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class FX    : public Component
{
public:
    FX();
    ~FX();

    float samplerate;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FX)
};


#endif  // FX_H_INCLUDED
