/*
  ==============================================================================

    FXChain.cpp
    Created: 2 Mar 2017 9:41:35pm
    Author:  Vincent Choqueuse

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "FXChain.h"

//==============================================================================
FXChain::FXChain()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.


    
}

FXChain::~FXChain()
{
}


FXChain::init()
{
    filterL.reset();
    filterR.reset();
}

FXChain:processSamples (float *samplesL,float *samplesR,int numSamples)
{
    
    filterL.processSamples (float *samplesL, int numSamples);
    filterR.processSamples (float *samplesR, int numSamples);
       
}
