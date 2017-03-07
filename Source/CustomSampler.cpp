#include "CustomSampler.h"

CustomSamplerSound::CustomSamplerSound (const String& soundName,
                            const BigInteger& notes,
                            const int midiNoteForNormalPitch,
                            const double attackTimeSecs,
                            const double releaseTimeSecs,
                            const double maxSampleLengthSeconds)
: detune(0),
thumbnailCache (5),                            // [4]
thumbnail (512, formatManager, thumbnailCache), // [5]
name (soundName),
midiNotes (notes),
midiRootNote (midiNoteForNormalPitch),
attackTimeSecs (attackTimeSecs),
releaseTimeSecs(releaseTimeSecs),
maxSampleLengthSeconds (maxSampleLengthSeconds)
{
    filter_type=1;
    filter_cutoff=1000.0;
}


CustomSamplerSound::~CustomSamplerSound()
{
}

void CustomSamplerSound::loadSound(int index)
{
    ScopedPointer<AudioFormatReader> source;
    int dataSizeInBytes;
    AiffAudioFormat aiffFormat;
    const char* ressource=BinaryData::getNamedResource (BinaryData::namedResourceList[index], dataSizeInBytes);
    
    source=aiffFormat.createReaderFor (new MemoryInputStream(ressource,dataSizeInBytes,false),true);
    thumbnail.setReader(aiffFormat.createReaderFor(new MemoryInputStream(ressource,dataSizeInBytes,false),true),index);
    sourceSampleRate = source->sampleRate;
    
    if (sourceSampleRate <= 0 || source->lengthInSamples <= 0)
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = jmin ((int) source->lengthInSamples,(int) (maxSampleLengthSeconds * sourceSampleRate));
        data = new AudioSampleBuffer (jmin (2, (int) source->numChannels), length + 4);
        source->read (data, 0, length + 4, 0, true, true);
        attackSamples = roundToInt (attackTimeSecs * sourceSampleRate);
        releaseSamples = roundToInt (releaseTimeSecs * sourceSampleRate);
        
    }
    
}

void CustomSamplerSound::setFilter()
{

    
}

bool CustomSamplerSound::appliesToNote (int midiNoteNumber)
{
    return midiNotes [midiNoteNumber];
}

bool CustomSamplerSound::appliesToChannel (int /*midiChannel*/)
{
    return true;
}



//==============================================================================
CustomSamplerVoice::CustomSamplerVoice()
: pitchRatio (0.0),
sourceSamplePosition (0.0),
lgain (0.0f), rgain (0.0f),
attackReleaseLevel (0), attackDelta (0), releaseDelta (0),
isInAttack (false), isInRelease (false)
{

    
}

CustomSamplerVoice::~CustomSamplerVoice()
{
}

bool CustomSamplerVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<const CustomSamplerSound*> (sound) != nullptr;
}

void CustomSamplerVoice::startNote (const int midiNoteNumber,
                              const float velocity,
                              SynthesiserSound* s,
                              const int /*currentPitchWheelPosition*/)
{
    if (CustomSamplerSound* sound = dynamic_cast<CustomSamplerSound*> (s))
    {

        
        //setFilterCoef(sound->filter_type,sound->sourceSampleRate,sound->filter_cutoff);
   
        IIRCoefficients coef;
        switch (sound->filter_type)
        {
            case 1:
                coef=IIRCoefficients::makeLowPass(sound->sourceSampleRate,sound->filter_cutoff);
                break;
            case 2:
                coef=IIRCoefficients::makeHighPass(sound->sourceSampleRate,sound->filter_cutoff);
                break;
            case 3:
                coef=IIRCoefficients::makeBandPass(sound->sourceSampleRate,sound->filter_cutoff);
                break;
        }
        filterR.setCoefficients(coef);
        filterL.setCoefficients(coef);
        filterR.reset();
        filterL.reset();
        
        pitchRatio = pow (2.0, (midiNoteNumber - sound->midiRootNote + sound->detune) / 12.0) * sound->sourceSampleRate / getSampleRate();
        

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;
        
        isInAttack = (sound->attackSamples > 0);
        isInRelease = false;
   
       
        if (isInAttack)
        {
            attackReleaseLevel = 0.0f;
            attackDelta = (float) (pitchRatio / sound->attackSamples);
        }
        else
        {
            attackReleaseLevel = 1.0f;
            attackDelta = 0.0f;
        }
        
        if (sound->releaseSamples > 0)
            releaseDelta = (float) (-pitchRatio / sound->releaseSamples);
        else
            releaseDelta = -1.0f;
    }
    else
    {
        jassertfalse; // this object can only play CustomSamplerSounds!
    }
}

void CustomSamplerVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    
 
    if (allowTailOff)
    {
        //Logger::outputDebugString("Coucou1");
        isInAttack = false;
        isInRelease = true;
      
    }
    else
    {
        clearCurrentNote();
    }

}



void CustomSamplerVoice::pitchWheelMoved (const int /*newValue*/)
{
}

void CustomSamplerVoice::controllerMoved (const int /*controllerNumber*/,
                                    const int /*newValue*/)
{
}

//==============================================================================
void CustomSamplerVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    
    if (const CustomSamplerSound* const playingSound = static_cast<CustomSamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        const float* const inL = playingSound->data->getReadPointer (0);
        const float* const inR = playingSound->data->getNumChannels() > 1 ? playingSound->data->getReadPointer (1) : nullptr;
        

        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;
        
        while (--numSamples >= 0)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;
            
            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha): l;
            
            l *= lgain;
            r *= rgain;
            
            if (isInAttack)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;
                
                attackReleaseLevel += attackDelta;
                
                if (attackReleaseLevel >= 1.0f)
                {
                    attackReleaseLevel = 1.0f;
                    isInAttack = false;
                }
            }
            else if (isInRelease)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;
                
                attackReleaseLevel += releaseDelta;

                if (attackReleaseLevel <= 0.0f)
                {
                    stopNote (0.0f, false);
                    break;
                }
            }
            
            //Perform filtering
            
            if (outR != nullptr)
            {
                *outL++ += filterL.processSingleSampleRaw(l);

                *outR++ += filterR.processSingleSampleRaw(r);
            }
            else
            {
                *outL++ += filterL.processSingleSampleRaw((l + r) * 0.5f);
            }
            sourceSamplePosition += pitchRatio;
            
            if (sourceSamplePosition > playingSound->length)
            {
                stopNote (0.0f,false);
                break;
            }
        }
    }
    
   
}



