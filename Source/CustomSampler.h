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
class CustomSamplerSound    : public SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a sampled sound from an audio reader.
     
     This will attempt to load the audio from the source into memory and store
     it in this object.
     
     @param name         a name for the sample
     @param source       the audio to load. This object can be safely deleted by the
     caller after this constructor returns
     @param midiNotes    the set of midi keys that this sound should be played on. This
     is used by the SynthesiserSound::appliesToNote() method
     @param midiNoteForNormalPitch   the midi note at which the sample should be played
     with its natural rate. All other notes will be pitched
     up or down relative to this one
     @param attackTimeSecs   the attack (fade-in) time, in seconds
     @param releaseTimeSecs  the decay (fade-out) time, in seconds
     @param maxSampleLengthSeconds   a maximum length of audio to read from the audio
     source, in seconds
     */
    CustomSamplerSound (const String& name,
                  AudioFormatReader& source,
                  const BigInteger& midiNotes,
                  int midiNoteForNormalPitch,
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
    
    
    //==============================================================================
    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;
    int detune;
    
private:
    //==============================================================================
    friend class CustomSamplerVoice;
    
    String name;
    ScopedPointer<AudioSampleBuffer> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length, attackSamples, releaseSamples;
    int midiRootNote;
    
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
    
    
private:
    //==============================================================================
    double pitchRatio;
    double sourceSamplePosition;
    float lgain, rgain, attackReleaseLevel, attackDelta, releaseDelta;
    bool isInAttack, isInRelease;
    
    JUCE_LEAK_DETECTOR (CustomSamplerVoice)
};


#endif   // CUSTOMSAMPLER_H_INCLUDED
