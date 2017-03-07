/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED


//==============================================================================


class MainComponent : public Component,
                      private AudioIODeviceCallback,  // [1]
                      private MidiInputCallback,
                      public MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent()
        :keyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard),
       tabs(TabbedButtonBar::TabsAtTop),
    isAddingFromMidiInput (false)
    {
        
        setSize (650, 560);

        audioDeviceManager.initialise (0, 2, nullptr, true, String(), nullptr);
        audioDeviceManager.addMidiInputCallback (String(), this); // [6]
        audioDeviceManager.addAudioCallback (this);
        const StringArray midiInputs (MidiInput::getDevices());
        audioDeviceManager.setMidiInputEnabled(midiInputs[0],true);
        addAndMakeVisible (keyboardComponent);
        keyboardState.addListener (this);
        addAndMakeVisible (midinote_label);
        addAndMakeVisible (tabs);
        midinote_label.setText("None",dontSendNotification);
        
        tabs.addTab ("Kit", Colours::lightgrey, new KitPage(synth),true);
        tabs.addTab ("Sample", Colours::lightgrey, new SamplerPage(synth),true);
        tabs.addTab ("FX", Colours::lightgrey, new EffectPage(synth),true);
        tabs.addTab ("Devices", Colours::lightgrey, new AudioDeviceSelectorComponent(audioDeviceManager, 0, 0, 0, 256, true,false, true, false),true);


    }

    ~MainComponent()
    {
        audioDeviceManager.removeAudioCallback (this);
        keyboardState.removeListener (this);
        audioDeviceManager.removeMidiInputCallback (String(), this);
    }


    //==============================================================================
    void resized() override
    {
        //Rectangle<int> r (getLocalBounds());
        //audioSetupComp.setBounds (r);
        int offset=8;
        
        tabs.setBounds (offset,offset,getWidth()-2*offset,getHeight()-64-2*offset);
        keyboardComponent.setBounds (offset, getHeight()-64, getWidth() -2*offset, 64);
        midinote_label.setBounds (offset, 100, getWidth() -2*offset, 20);
        repaint();
        
    }
    
        
    //==============================================================================
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override
    {
        // make buffer
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
        
        // clear it to silence
        buffer.clear();
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
        synth.renderNextBlock (buffer, incomingMidi, 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        const double sampleRate = device->getCurrentSampleRate();
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate(sampleRate);
    }

    void audioDeviceStopped() override
    {
    }
    


private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/,
                                    const MidiMessage& message) override
    {
        const ScopedValueSetter<bool> scopedInputFlag (isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent (message);
        midiCollector.addMessageToQueue (message);
        //midinote_label.setText(message.getDescription(),dontSendNotification);
    }
    
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        if (! isAddingFromMidiInput)
        {
            MidiMessage message (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
            midiCollector.addMessageToQueue (message);
        }
    }
    
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
    {
        if (! isAddingFromMidiInput)
        {
            MidiMessage message (MidiMessage::noteOff (midiChannel, midiNoteNumber));
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
            midiCollector.addMessageToQueue (message);
        }
    }
    
    
    //==============================================================================
    AudioDeviceManager audioDeviceManager;         // [3]
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;
    DrumSynthesiser synth;
    MidiMessageCollector midiCollector;
    Label midinote_label;
 
    TabbedComponent tabs;
    bool isAddingFromMidiInput;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};





#endif  // MAINCOMPONENT_H_INCLUDED
