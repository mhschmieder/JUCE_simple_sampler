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

#include "CustomMidiKeyboardComponent.h"

class MidiKeyboardUpDownButton  : public Button
{
public:
    MidiKeyboardUpDownButton (CustomMidiKeyboardComponent& comp, const int d)
    : Button (String()), owner (comp), delta (d)
    {
    }
    
    void clicked() override
    {
        int note = owner.getLowestVisibleKey();
        
        if (delta < 0)
            note = (note - 1) / 12;
        else
            note = note / 12 + 1;
        
        owner.setLowestVisibleKey (note * 12);
    }
    
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        //owner.drawUpDownButton (g, getWidth(), getHeight(),isMouseOverButton, isButtonDown,delta > 0);
    }
    
private:
    CustomMidiKeyboardComponent& owner;
    const int delta;
    
    JUCE_DECLARE_NON_COPYABLE (MidiKeyboardUpDownButton)
};

//==============================================================================
CustomMidiKeyboardComponent::CustomMidiKeyboardComponent (MidiKeyboardState& s)
: state (s),
blackNoteLengthRatio (0.7f),
xOffset (0),
keyWidth (32.0f),
midiChannel (1),
midiInChannelMask (0xffff),
velocity (1.0f),
shouldCheckState (false),
rangeStart (0),
rangeEnd (127),
firstKey (12 * 4.0f),
useMousePositionForVelocity (true),
shouldCheckMousePos (false),
keyMappingOctave (6),
octaveNumForMiddleC (3)
{

    // initialise with a default set of qwerty key-mappings..
    const char* const keymap = "awsedftgyhujkolp;";
    
    for (int i = 0; keymap[i] != 0; ++i)
        setKeyPressForNote (KeyPress (keymap[i], 0, 0), i);
    
    mouseOverNotes.insertMultiple (0, -1, 32);
    mouseDownNotes.insertMultiple (0, -1, 32);
    
    colourChanged();
    setWantsKeyboardFocus (true);
    state.addListener (this);
    startTimerHz (20);
}

CustomMidiKeyboardComponent::~CustomMidiKeyboardComponent()
{
    state.removeListener (this);
}

//==============================================================================



void CustomMidiKeyboardComponent::setAvailableRange (const int lowestNote,
                                               const int highestNote)
{
    jassert (lowestNote >= 0 && lowestNote <= 127);
    jassert (highestNote >= 0 && highestNote <= 127);
    jassert (lowestNote <= highestNote);
    
    if (rangeStart != lowestNote || rangeEnd != highestNote)
    {
        rangeStart = jlimit (0, 127, lowestNote);
        rangeEnd = jlimit (0, 127, highestNote);
        firstKey = jlimit ((float) rangeStart, (float) rangeEnd, firstKey);
        resized();
    }
}

void CustomMidiKeyboardComponent::setLowestVisibleKey (int noteNumber)
{
    setLowestVisibleKeyFloat ((float) noteNumber);
}

void CustomMidiKeyboardComponent::setLowestVisibleKeyFloat (float noteNumber)
{
    noteNumber = jlimit ((float) rangeStart, (float) rangeEnd, noteNumber);
    
    if (noteNumber != firstKey)
    {
        const bool hasMoved = (((int) firstKey) != (int) noteNumber);
        firstKey = noteNumber;
        
        if (hasMoved)
            sendChangeMessage();
        
        resized();
    }
}


void CustomMidiKeyboardComponent::colourChanged()
{
    setOpaque (findColour (whiteNoteColourId).isOpaque());
    repaint();
}

//==============================================================================
void CustomMidiKeyboardComponent::setMidiChannel (const int midiChannelNumber)
{
    jassert (midiChannelNumber > 0 && midiChannelNumber <= 16);
    
    if (midiChannel != midiChannelNumber)
    {
        resetAnyKeysInUse();
        midiChannel = jlimit (1, 16, midiChannelNumber);
    }
}

void CustomMidiKeyboardComponent::setMidiChannelsToDisplay (const int midiChannelMask)
{
    midiInChannelMask = midiChannelMask;
    shouldCheckState = true;
}

void CustomMidiKeyboardComponent::setVelocity (const float v, const bool useMousePosition)
{
    velocity = jlimit (0.0f, 1.0f, v);
    useMousePositionForVelocity = useMousePosition;
}

void CustomMidiKeyboardComponent::setKeyWidth()
{
    keyWidth=(getHeight()/4)-1;
    
}


//==============================================================================
void CustomMidiKeyboardComponent::getKeyPosition (int midiNoteNumber, const float keyWidth_, int& x, int& y) const
{
    jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);
    int midiNoteNumberScaled=midiNoteNumber-36;
    x = roundToInt ((midiNoteNumberScaled%4)* (keyWidth_+1));
    y = roundToInt ((midiNoteNumberScaled/4)* (keyWidth_+1));
}

void CustomMidiKeyboardComponent::getKeyPos (int midiNoteNumber, int& x, int& y) const
{
    getKeyPosition (midiNoteNumber, keyWidth, x, y);
    int rx, ry;
    getKeyPosition (rangeStart, keyWidth, rx, ry);
    x -= xOffset + rx;
    
}

Rectangle<int> CustomMidiKeyboardComponent::getRectangleForKey (const int note) const
{
    jassert (note >= rangeStart && note <= rangeEnd);
    
    int x, y;
    getKeyPos (note, x, y);
    
    return Rectangle<int> (x, y, keyWidth, keyWidth);
    
}

int CustomMidiKeyboardComponent::getKeyStartPosition (const int midiNoteNumber) const
{
    int x, y;
    getKeyPos (midiNoteNumber, x, y);
    return x;
}

int CustomMidiKeyboardComponent::getTotalKeyboardWidth() const noexcept
{
    int x, w;
    getKeyPos (rangeEnd, x, w);
    return x + w;
}

int CustomMidiKeyboardComponent::getNoteAtPosition (Point<int> p)
{
    float v;
    return xyToNote (p, v);
}


int CustomMidiKeyboardComponent::xyToNote (Point<int> pos, float& mousePositionVelocity)
{
    if (! reallyContains (pos, false))
        return -1;
    
    Point<int> p (pos);

    return remappedXYToNote (p + Point<int> (xOffset, 0), mousePositionVelocity);
}

int CustomMidiKeyboardComponent::remappedXYToNote (Point<int> pos, float& mousePositionVelocity) const
{
    for (int note=rangeStart;note<rangeEnd;note++)
    {
        int kx, ky;
        getKeyPos (note, kx, ky);
        kx += xOffset;
        
        if (pos.x - kx < keyWidth && pos.y - ky < keyWidth)
        {
            mousePositionVelocity = 1.;
            return note;
        }
    }
    
    mousePositionVelocity = 0;
    return -1;
}

//==============================================================================
void CustomMidiKeyboardComponent::repaintNote (const int noteNum)
{
    if (noteNum >= rangeStart && noteNum <= rangeEnd)
        repaint (getRectangleForKey (noteNum));
}

void CustomMidiKeyboardComponent::paint (Graphics& g)
{
    
    const Colour lineColour (findColour (keySeparatorLineColourId));
    const Colour textColour (findColour (textLabelColourId));
    
    Colour background=Colour(141,141,141);
    Colour c=Colour(200,200,200);
    
    const Rectangle<int> Bounds (0,0, getWidth() , getHeight() );
    g.setColour (background);
    g.fillRect (Bounds);
    
    for (int noteNum=rangeStart;noteNum<rangeEnd;noteNum++)
    {
        Rectangle<int> pos = getRectangleForKey (noteNum);
        int x=pos.getX();
        int y=pos.getY();
        int w=pos.getWidth();
        int h=pos.getHeight();
        bool isDown=state.isNoteOnForChannels (midiInChannelMask, noteNum);
        bool isOver=mouseOverNotes.contains (noteNum);
        
        Colour c=Colour(191,188,187);
        
        if (isDown)  c = findColour (keyDownOverlayColourId);
        if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));
        
        g.setColour (c);
        g.fillRoundedRectangle(x, y, w,w, 0.1*w);
        
        const String text (getWhiteNoteText (noteNum));
        
        if (text.isNotEmpty())
        {
            const float fontHeight = jmin (12.0f, keyWidth * 0.9f);
            g.setColour (textColour);
            g.setFont (Font (fontHeight).withHorizontalScale (0.8f));
            g.drawText (text, x + 1, y,     w - 1, h - 2, Justification::centredBottom, false);
        }
        
        
        
    }

}

void CustomMidiKeyboardComponent::setOctaveForMiddleC (const int octaveNum)
{
    octaveNumForMiddleC = octaveNum;
    repaint();
}

String CustomMidiKeyboardComponent::getWhiteNoteText (const int midiNoteNumber)
{
    return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, octaveNumForMiddleC);
}


void CustomMidiKeyboardComponent::resized()
{
    int w = getWidth();
    int h = getHeight();
    
    if (w > 0 && h > 0)
    {
        int kx2, kw2;
        getKeyPos (rangeEnd, kx2, kw2);
        
        kx2 += kw2;
        
        if ((int) firstKey != rangeStart)
        {
            int kx1, kw1;
            getKeyPos (rangeStart, kx1, kw1);
            
            if (kx2 - kx1 <= w)
            {
                firstKey = (float) rangeStart;
                sendChangeMessage();
                repaint();
            }
        }
        
 
        xOffset = 0;
        firstKey = (float) rangeStart;
        getKeyPos (rangeEnd, kx2, kw2);
        repaint();
    }
}

//==============================================================================
void CustomMidiKeyboardComponent::handleNoteOn (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

void CustomMidiKeyboardComponent::handleNoteOff (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

//==============================================================================
void CustomMidiKeyboardComponent::resetAnyKeysInUse()
{
    if (! keysPressed.isZero())
    {
        for (int i = 128; --i >= 0;)
            if (keysPressed[i])
                state.noteOff (midiChannel, i, 0.0f);
        
        keysPressed.clear();
    }
    
    for (int i = mouseDownNotes.size(); --i >= 0;)
    {
        const int noteDown = mouseDownNotes.getUnchecked(i);
        
        if (noteDown >= 0)
        {
            state.noteOff (midiChannel, noteDown, 0.0f);
            mouseDownNotes.set (i, -1);
        }
        
        mouseOverNotes.set (i, -1);
    }
}

void CustomMidiKeyboardComponent::updateNoteUnderMouse (const MouseEvent& e, bool isDown)
{
    updateNoteUnderMouse (e.getEventRelativeTo (this).getPosition(), isDown, e.source.getIndex());
}

void CustomMidiKeyboardComponent::updateNoteUnderMouse (Point<int> pos, bool isDown, int fingerNum)
{
    float mousePositionVelocity = 0.0f;
    const int newNote = xyToNote (pos, mousePositionVelocity);
    const int oldNote = mouseOverNotes.getUnchecked (fingerNum);
    const int oldNoteDown = mouseDownNotes.getUnchecked (fingerNum);
    const float eventVelocity = useMousePositionForVelocity ? mousePositionVelocity * velocity : 1.0f;
    
    if (oldNote != newNote)
    {
        repaintNote (oldNote);
        repaintNote (newNote);
        mouseOverNotes.set (fingerNum, newNote);
    }
    
    if (isDown)
    {
        if (newNote != oldNoteDown)
        {
            if (oldNoteDown >= 0)
            {
                mouseDownNotes.set (fingerNum, -1);
                
                if (! mouseDownNotes.contains (oldNoteDown))
                    state.noteOff (midiChannel, oldNoteDown, eventVelocity);
            }
            
            if (newNote >= 0 && ! mouseDownNotes.contains (newNote))
            {
                state.noteOn (midiChannel, newNote, eventVelocity);
                mouseDownNotes.set (fingerNum, newNote);
            }
        }
    }
    else if (oldNoteDown >= 0)
    {
        mouseDownNotes.set (fingerNum, -1);
        
        if (! mouseDownNotes.contains (oldNoteDown))
            state.noteOff (midiChannel, oldNoteDown, eventVelocity);
    }
}

void CustomMidiKeyboardComponent::mouseMove (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
    shouldCheckMousePos = false;
}

void CustomMidiKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.getPosition(), mousePositionVelocity);
    
    if (newNote >= 0)
        mouseDraggedToKey (newNote, e);
    
    updateNoteUnderMouse (e, true);
}

bool CustomMidiKeyboardComponent::mouseDownOnKey    (int, const MouseEvent&)  { return true; }
void CustomMidiKeyboardComponent::mouseDraggedToKey (int, const MouseEvent&)  {}
void CustomMidiKeyboardComponent::mouseUpOnKey      (int, const MouseEvent&)  {}

void CustomMidiKeyboardComponent::mouseDown (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.getPosition(), mousePositionVelocity);
    
    if (newNote >= 0 && mouseDownOnKey (newNote, e))
    {
        updateNoteUnderMouse (e, true);
        shouldCheckMousePos = true;
    }
}

void CustomMidiKeyboardComponent::mouseUp (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
    shouldCheckMousePos = false;
    
    float mousePositionVelocity;
    const int note = xyToNote (e.getPosition(), mousePositionVelocity);
    if (note >= 0)
        mouseUpOnKey (note, e);
}

void CustomMidiKeyboardComponent::mouseEnter (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void CustomMidiKeyboardComponent::mouseExit (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void CustomMidiKeyboardComponent::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    const float amount =  wheel.deltaX ;
    
    setLowestVisibleKeyFloat (firstKey - amount * keyWidth);
}

void CustomMidiKeyboardComponent::timerCallback()
{
    if (shouldCheckState)
    {
        shouldCheckState = false;
        
        for (int i = rangeStart; i <= rangeEnd; ++i)
        {
            if (keysCurrentlyDrawnDown[i] != state.isNoteOnForChannels (midiInChannelMask, i))
            {
                keysCurrentlyDrawnDown.setBit (i, state.isNoteOnForChannels (midiInChannelMask, i));
                repaintNote (i);
            }
        }
    }
    
    if (shouldCheckMousePos)
    {
        const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();
        
        for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
            if (mi->getComponentUnderMouse() == this || isParentOf (mi->getComponentUnderMouse()))
                updateNoteUnderMouse (getLocalPoint (nullptr, mi->getScreenPosition()).roundToInt(), mi->isDragging(), mi->getIndex());
    }
}

//==============================================================================
void CustomMidiKeyboardComponent::clearKeyMappings()
{
    resetAnyKeysInUse();
    keyPressNotes.clear();
    keyPresses.clear();
}

void CustomMidiKeyboardComponent::setKeyPressForNote (const KeyPress& key, int midiNoteOffsetFromC)
{
    removeKeyPressForNote (midiNoteOffsetFromC);
    
    keyPressNotes.add (midiNoteOffsetFromC);
    keyPresses.add (key);
}

void CustomMidiKeyboardComponent::removeKeyPressForNote (const int midiNoteOffsetFromC)
{
    for (int i = keyPressNotes.size(); --i >= 0;)
    {
        if (keyPressNotes.getUnchecked (i) == midiNoteOffsetFromC)
        {
            keyPressNotes.remove (i);
            keyPresses.remove (i);
        }
    }
}

void CustomMidiKeyboardComponent::setKeyPressBaseOctave (const int newOctaveNumber)
{
    jassert (newOctaveNumber >= 0 && newOctaveNumber <= 10);
    
    keyMappingOctave = newOctaveNumber;
}

bool CustomMidiKeyboardComponent::keyStateChanged (const bool /*isKeyDown*/)
{
    bool keyPressUsed = false;
    
    for (int i = keyPresses.size(); --i >= 0;)
    {
        const int note = 12 * keyMappingOctave + keyPressNotes.getUnchecked (i);
        
        if (keyPresses.getReference(i).isCurrentlyDown())
        {
            if (! keysPressed [note])
            {
                keysPressed.setBit (note);
                state.noteOn (midiChannel, note, velocity);
                keyPressUsed = true;
            }
        }
        else
        {
            if (keysPressed [note])
            {
                keysPressed.clearBit (note);
                state.noteOff (midiChannel, note, 0.0f);
                keyPressUsed = true;
            }
        }
    }
    
    return keyPressUsed;
}

bool CustomMidiKeyboardComponent::keyPressed (const KeyPress& key)
{
    return keyPresses.contains (key);
}

void CustomMidiKeyboardComponent::focusLost (FocusChangeType)
{
    resetAnyKeysInUse();
}
