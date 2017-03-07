/*
  ==============================================================================

    FXChain.h
    Created: 2 Mar 2017 9:41:35pm
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#ifndef FXCHAIN_H_INCLUDED
#define FXCHAIN_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class FXChain    : public Component
{
public:
    FXChain();
    ~FXChain();
    
    init();
    processSamples (float *samplesL,float *samplesR,int numSamples);

 
private:
    IIRFilter filterL, filterR;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FXChain)
};




#endif  // FXCHAIN_H_INCLUDED
