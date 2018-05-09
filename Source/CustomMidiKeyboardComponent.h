/*
  ==============================================================================

    CustomMidiKeyboardComponent.h
    Created: 25 Mar 2017 11:18:17am
    Author:  Vincent Choqueuse

  ==============================================================================
*/
#include "../JuceLibraryCode/JuceHeader.h"


#ifndef CUSTOMMIDIKEYBOARDCOMPONENT_H_INCLUDED
#define CUSTOMMIDIKEYBOARDCOMPONENT_H_INCLUDED


class CustomMidiKeyboardComponent  :
    public Component,
    public MidiKeyboardStateListener,
    public ChangeBroadcaster,
    private Timer
{
public:
    //==============================================================================
    /** The direction of the keyboard.
     @see setOrientation
     */
    

    CustomMidiKeyboardComponent(MidiKeyboardState& state);
    
    ~CustomMidiKeyboardComponent();
    
    //==============================================================================
    /** Changes the velocity used in midi note-on messages that are triggered by clicking
     on the component.
     Values are 0 to 1.0, where 1.0 is the heaviest.
     @see setMidiChannel
     */
    void setVelocity (float velocity, bool useMousePositionForVelocity);
    void setMidiChannel (int midiChannelNumber);
    int getMidiChannel() const noexcept                             { return midiChannel; }
    void setMidiChannelsToDisplay (int midiChannelMask);

    int getMidiChannelsToDisplay() const noexcept                   { return midiInChannelMask; }
    
    //==============================================================================
    /** Changes the width used to draw the white keys. */
    void setKeyWidth();
    
    /** Returns the width that was set by setKeyWidth(). */
    float getKeyWidth() const noexcept                              { return keyWidth; }

    void setAvailableRange (int lowestNote,
                            int highestNote);
    
    /** Returns the first note in the available range.
     @see setAvailableRange
     */
    int getRangeStart() const noexcept                              { return rangeStart; }
    
    /** Returns the last note in the available range.
     @see setAvailableRange
     */
    int getRangeEnd() const noexcept                                { return rangeEnd; }
    
    /** If the keyboard extends beyond the size of the component, this will scroll
     it to show the given key at the start.
     Whenever the keyboard's position is changed, this will use the ChangeBroadcaster
     base class to send a callback to any ChangeListeners that have been registered.
     */
    void setLowestVisibleKey (int noteNumber);
    
    /** Returns the number of the first key shown in the component.
     @see setLowestVisibleKey
     */
    int getLowestVisibleKey() const noexcept                        { return (int) firstKey; }
    
    
    
    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.
     These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
     methods.
     @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
     */
    enum ColourIds
    {
        whiteNoteColourId               = 0x1005000,
        blackNoteColourId               = 0x1005001,
        keySeparatorLineColourId        = 0x1005002,
        mouseOverKeyOverlayColourId     = 0x1005003,  /**< This colour will be overlaid on the normal note colour. */
        keyDownOverlayColourId          = 0x1005004,  /**< This colour will be overlaid on the normal note colour. */
        textLabelColourId               = 0x1005005,
        upDownButtonBackgroundColourId  = 0x1005006,
        upDownButtonArrowColourId       = 0x1005007,
        shadowColourId                  = 0x1005008
    };
    
    /** Returns the position within the component of the left-hand edge of a key.
     Depending on the keyboard's orientation, this may be a horizontal or vertical
     distance, in either direction.
     */
    int getKeyStartPosition (int midiNoteNumber) const;
    
    /** Returns the total width needed to fit all the keys in the available range. */
    int getTotalKeyboardWidth() const noexcept;
    
    /** Returns the key at a given coordinate. */
    int getNoteAtPosition (Point<int> position);
    
    //==============================================================================
    /** Deletes all key-mappings.
     @see setKeyPressForNote
     */
    void clearKeyMappings();
    
    void setKeyPressForNote (const KeyPress& key,
                             int midiNoteOffsetFromC);
    
    void removeKeyPressForNote (int midiNoteOffsetFromC);
    
    void setKeyPressBaseOctave (int newOctaveNumber);
    
    void setOctaveForMiddleC (int octaveNumForMiddleC);
    
    /** This returns the value set by setOctaveForMiddleC().
     @see setOctaveForMiddleC
     */
    int getOctaveForMiddleC() const noexcept            { return octaveNumForMiddleC; }
    
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseMove (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void mouseEnter (const MouseEvent&) override;
    /** @internal */
    void mouseExit (const MouseEvent&) override;
    /** @internal */
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    void timerCallback() override;
    /** @internal */
    bool keyStateChanged (bool isKeyDown) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    void focusLost (FocusChangeType) override;
    /** @internal */
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    /** @internal */
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    /** @internal */
    void colourChanged() override;
    
protected:


    /** Allows text to be drawn on the white notes.
     By default this is used to label the C in each octave, but could be used for other things.
     @see setOctaveForMiddleC
     */
    virtual String getWhiteNoteText (const int midiNoteNumber);
    
    /** Draws the up and down buttons that change the base note. */

    /** Callback when the mouse is clicked on a key.
     You could use this to do things like handle right-clicks on keys, etc.
     Return true if you want the click to trigger the note, or false if you
     want to handle it yourself and not have the note played.
     @see mouseDraggedToKey
     */
    virtual bool mouseDownOnKey (int midiNoteNumber, const MouseEvent& e);
    
    /** Callback when the mouse is dragged from one key onto another.
     @see mouseDownOnKey
     */
    virtual void mouseDraggedToKey (int midiNoteNumber, const MouseEvent& e);
    
    /** Callback when the mouse is released from a key.
     @see mouseDownOnKey
     */
    virtual void mouseUpOnKey (int midiNoteNumber, const MouseEvent& e);
    
    /** Calculates the position of a given midi-note.
     This can be overridden to create layouts with custom key-widths.
     @param midiNoteNumber   the note to find
     @param keyWidth         the desired width in pixels of one key - see setKeyWidth()
     @param x                the x position of the left-hand edge of the key (this method
     always works in terms of a horizontal keyboard)
     @param w                the width of the key
     */
    virtual void getKeyPosition (int midiNoteNumber, float keyWidth,
                                 int& x, int& w) const;
    
    /** Returns the rectangle for a given key if within the displayable range */
    Rectangle<int> getRectangleForKey (int midiNoteNumber) const;
    
    
private:
    //==============================================================================
    friend class MidiKeyboardUpDownButton;
    
    MidiKeyboardState& state;
    float blackNoteLengthRatio;
    int xOffset;
    float keyWidth;
    
    int midiChannel, midiInChannelMask;
    float velocity;
    
    Array<int> mouseOverNotes, mouseDownNotes;
    BigInteger keysPressed, keysCurrentlyDrawnDown;
    bool shouldCheckState;
    
    int rangeStart, rangeEnd;
    float firstKey;
    bool useMousePositionForVelocity, shouldCheckMousePos;
    
    Array<KeyPress> keyPresses;
    Array<int> keyPressNotes;
    int keyMappingOctave, octaveNumForMiddleC;
    
    static const uint8 whiteNotes[];
    static const uint8 blackNotes[];
    
    void getKeyPos (int midiNoteNumber, int& x, int& w) const;
    int xyToNote (Point<int>, float& mousePositionVelocity);
    int remappedXYToNote (Point<int>, float& mousePositionVelocity) const;
    void resetAnyKeysInUse();
    void updateNoteUnderMouse (Point<int>, bool isDown, int fingerNum);
    void updateNoteUnderMouse (const MouseEvent&, bool isDown);
    void repaintNote (int midiNoteNumber);
    void setLowestVisibleKeyFloat (float noteNumber);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomMidiKeyboardComponent)
};


#endif   // JUCE_CUSTOMMIDIKEYBOARDCOMPONENT_H_INCLUDED
