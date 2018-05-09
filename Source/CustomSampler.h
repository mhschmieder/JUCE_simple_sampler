#include "../JuceLibraryCode/JuceHeader.h"

#ifndef CUSTOMSAMPLER_H_INCLUDED
#define CUSTOMSAMPLER_H_INCLUDED


//==============================================================================
/**
 A subclass of SynthesiserSound that represents a sampled audio clip.
 
 This is a pretty basic sampler, and just attempts to load the whole audio stream
 into memory.
 
 To use it, create a Synthesiser, add some CustomSamplerVoice objects to it, then
 give it some SampledSound objects to play.
 
 @see CustomSamplerVoice, Synthesiser, SynthesiserSound
 */
class CustomSamplerSound    : public SynthesiserSound, AsyncUpdater
{
public:

    CustomSamplerSound (const String& name,
                  const BigInteger& midiNotes,
                  int midiNoteNumber,
                  double attackTimeSecs,
                  double releaseTimeSecs,
                  double maxSampleLengthSeconds);
    
    /** Destructor. */
    ~CustomSamplerSound();
    
    //==============================================================================
    /** Returns the sample's name */
    const String& getName() const noexcept                  { return name; }
    
    /** Returns the audio sample data.
     This could return nullptr if there was a problem loading the data.
     */
    AudioSampleBuffer* getAudioData() const noexcept        { return data; }
    
    void loadSound();
    void loadThumbnail();
    
    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;
    void handleAsyncUpdate() override;
    
    void setFilter();
    int detune;
    int filter_type;
    float filter_cutoff;
    int filter_active;
    double sample_length;
    float sample_start,sample_end;
    int sample_index;
    File audioFile;
    
    AudioFormatManager formatManager; 
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    int midiRootNote;

private:
    //==============================================================================
    friend class CustomSamplerVoice;
    
    String name;
    ScopedPointer<AudioSampleBuffer> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length, attackSamples, releaseSamples;
    
    double attackTimeSecs, releaseTimeSecs, maxSampleLengthSeconds;
    
    JUCE_LEAK_DETECTOR (CustomSamplerSound)
};


//==============================================================================
/**
 A subclass of SynthesiserVoice that can play a CustomSamplerSound.
 
 To use it, create a Synthesiser, add some CustomSamplerVoice objects to it, then
 give it some SampledSound objects to play.
 
 @see CustomSamplerSound, Synthesiser, SynthesiserVoice
 */
class CustomSamplerVoice    : public SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a CustomSamplerVoice. */
    CustomSamplerVoice();
    
    /** Destructor. */
    ~CustomSamplerVoice();
    
    //==============================================================================
    bool canPlaySound (SynthesiserSound*) override;
    
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    void renderNextBlock (AudioSampleBuffer&, int startSample, int numSamples) override;
  
    double sourceSamplePosition,sourceSampleLength;
    
private:
    //==============================================================================
    double pitchRatio;
    float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
    bool isInAttack, isInRelease;

    IIRFilter filterL;
    IIRFilter filterR;


    JUCE_LEAK_DETECTOR (CustomSamplerVoice)
};


#endif   // CUSTOMSAMPLER_H_INCLUDED
