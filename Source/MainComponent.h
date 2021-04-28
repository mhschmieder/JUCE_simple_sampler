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

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================


class MainComponent : public Component,
                      private AudioIODeviceCallback,  // [1]
                      private MidiInputCallback,
                      public MidiKeyboardStateListener,
                      public URL::DownloadTask::Listener,
                      private ComboBox::Listener
{
public:
    //==============================================================================
    MainComponent():
       tabs(TabbedButtonBar::TabsAtTop),
        progressbar(progress),
        isAddingFromMidiInput (false),
        keyboardComponent(keyboardState),
        startTime (Time::getMillisecondCounterHiRes() * 0.001)
    {
        
        setSize (800, 600);

        audioDeviceManager.initialise (0, 2, nullptr, true, String(), nullptr);
        audioDeviceManager.addMidiInputCallback (String(), this); // [6]
        audioDeviceManager.addAudioCallback (this);
        const StringArray midiInputs (MidiInput::getDevices());
        audioDeviceManager.setMidiInputEnabled(midiInputs[0],true);
        addAndMakeVisible (midinote_label);
        addAndMakeVisible (tabs);
        
        addAndMakeVisible (progressbar);
        keyboardState.addListener (this);
        
        combobox_kit.addListener(this);
        combobox_kit.addItem (String("Kit1"),1);
        combobox_kit.addItem (String("Kit2"),2);
        
        addAndMakeVisible (combobox_kit);
        
        midinote_label.setText("None",dontSendNotification);
        
        Colour background_colour=Colour(141,141,141);
        tabs.addTab ("Sample", background_colour, new SamplerPage(synth,&keyboardComponent),true);
        tabs.addTab ("Devices", background_colour, new AudioDeviceSelectorComponent(audioDeviceManager, 0, 0, 0, 256, true,false, true, false),true);

    }

    ~MainComponent()
    {
        audioDeviceManager.removeAudioCallback (this);
        audioDeviceManager.removeMidiInputCallback (String(), this);
  

        //progressbar.removeComponentListener(this);
    }


    //==============================================================================
    void resized() override
    {
        int offset=8;
        tabs.setBounds (offset,offset,getWidth()-2*offset,getHeight()-2*offset);

        combobox_kit.setBounds(getWidth()-300,8,90,20);
        progressbar.setBounds(getWidth()-200,8,192,20);
        repaint();
    }
    
        
    //==============================================================================
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override
    {
        AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
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
    
    void download_kit(int num_kit){
        
        synth.num_kit=num_kit;
        synth.nb_samples=8;
        
        File directory = File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile (String::formatted ("kit%d/",num_kit));
               
        Logger::outputDebugString(directory.getFullPathName());
        
        
        //add sample to combobox
        
        SamplerPage* samplerpage = static_cast<SamplerPage*> (tabs.getTabContentComponent(0));
        samplerpage->samplecomboBox.comboBox.clear();
        samplerpage->bankcomboBox.comboBox.clear();
        
        if (!directory.isDirectory())
        {
            directory.createDirectory();
        }
        
        progress=0;
        for (int index=1;index<=8;index++)
        {
            File audioFile = File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile (String::formatted ("kit%d/mysample%d.aif",num_kit,index));
            
            Logger::outputDebugString(audioFile.getFullPathName());
            
            if (!audioFile.exists())
            {
                //If file does not exist -> download file
                URL url{String::formatted ("https://s3-eu-west-1.amazonaws.com/choqueuse/drumheaven/kit%d/Tranche%d.aif",num_kit, index)};
                std::unique_ptr<InputStream> test= url.createInputStream(1);
                if (test != nullptr)
                {
                    Logger::outputDebugString("Load Downloaded File");
                    tache[index-1]=url.downloadToFile(audioFile,"",this);
                }
                else
                {
                    Logger::outputDebugString("The file does not exist and no Internet connection is available");
                }
            }
            else
            {
                progress=1;
                //load kit directly
                Logger::outputDebugString("Sample are already available");
                synth.loadKit();
            }
            

         // add item
        samplerpage->samplecomboBox.comboBox.addItem(audioFile.getFileName(),index);
        samplerpage->bankcomboBox.comboBox.addItem(String::formatted("Pad %d",index),index);
            
        }
        
    }

    void comboBoxChanged(ComboBox* combobox) override
    {

        if (combobox==&combobox_kit)
        {
            progress=0.;
            download_kit(1);
        }
    }
        
    std::unique_ptr<URL::DownloadTask> tache[8];

private:
    //==============================================================================
    void handleIncomingMidiMessage (MidiInput* /*source*/,const MidiMessage& message) override
    {
        
        const ScopedValueSetter<bool> scopedInputFlag (isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent (message);
    }
    
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {

        MidiMessage message (MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
        message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        midiCollector.addMessageToQueue (message);

        
        //update time for waveform
        SamplerPage* samplerpage = static_cast<SamplerPage*> (tabs.getCurrentContentComponent () );
        
        
        if (samplerpage->selected_slot+36 == midiNoteNumber)
        {
            samplerpage->start_timer();
            synth.midiNoteNumber_playing=midiNoteNumber;
        }
    }
    
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
    {

        MidiMessage message (MidiMessage::noteOff (midiChannel, midiNoteNumber));
        message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001);
        midiCollector.addMessageToQueue (message);
        
        
        if ((tabs.getCurrentTabIndex()==0) && (synth.midiNoteNumber_playing==midiNoteNumber))
        {
            SamplerPage* samplerpage = static_cast<SamplerPage*> (tabs.getCurrentContentComponent () );
            samplerpage->stop_timer();
        }
    }
    
    void finished(URL::DownloadTask *task, bool success) override
    {
        progress+=1./8.;
        
        for (int indice=0;indice<8;indice++)
        {
            if (task==tache[indice].get())
            {
                Logger::outputDebugString(String::formatted ("Load sample%d",indice));
            }
        }
        
        if (progress>=1.)
        {
          synth.loadKit();
        }

    }
    
    
    //==============================================================================
    AudioDeviceManager audioDeviceManager;         // [3]
    DrumSynthesiser synth;
    Label midinote_label;
    TabbedComponent tabs;
    ComboBox combobox_kit;
    ProgressBar progressbar;
    bool isAddingFromMidiInput;
    double progress;
    MidiKeyboardState keyboardState;
    MidiMessageCollector midiCollector;
    CustomMidiKeyboardComponent keyboardComponent;

    double startTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};





#endif  // MAINCOMPONENT_H_INCLUDED
