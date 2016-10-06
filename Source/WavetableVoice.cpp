/*
  ==============================================================================

    WavetableVoice.cpp
    Created: 6 Oct 2016 9:35:15am
    Author:  Alexander Heemann

  ==============================================================================
*/

#include "WavetableVoice.h"

WavetableVoice::WavetableVoice(Wavetable& wavetable) : angleDelta(0.0), tailOff(0.0), wavetable(wavetable)
{
    
}

WavetableVoice::~WavetableVoice()
{
    
}

void WavetableVoice::startNote(int midiNoteNumber, float velocity,
               SynthesiserSound* sound,
               int currentPitchWheelPosition)
{
    currentAngle = 0.0;
    level = velocity * 0.15;
    tailOff = 0.0;
    
    frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    double frqRad = (2.0 * double_Pi) / getSampleRate();
    period = 1 / frequency;
    
    angleDelta = frqRad * frequency;
}

void WavetableVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff)
    {
        // start a tail-off by setting this flag. The render callback will pick up on
        // this and do a fade out, calling clearCurrentNote() when it's finished.
        
        if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
            // stopNote method could be called more than once.
            tailOff = 1.0;
    }
    else
    {
        // we're being told to stop playing immediately, so reset everything..
        
        clearCurrentNote();
        angleDelta = 0.0;
        currentAngle = 0.0;
    }
}

template <typename FloatType>
void WavetableVoice::processBlock(AudioBuffer<FloatType>& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        float* subtable = wavetable.getSubtableForFrequency(frequency);
        int tableSize = wavetable.getTableSize();
        double twoPi = 2.0 * double_Pi;
        if (tailOff > 0)
        {
            while (--numSamples >= 0)
            {
                int index = (int)((currentAngle / twoPi) * tableSize);
                FloatType currentSample = level * subtable[index];
                //DBG(subtable[index]);
                
                for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                if (currentAngle >= twoPi)
                {
                    currentAngle -= twoPi;
                }
                ++startSample;
                
                tailOff *= 0.99;
                
                if (tailOff <= 0.005)
                {
                    clearCurrentNote();
                    currentAngle = 0.0;
                    angleDelta = 0.0;
                    break;
                }
            }
        }
        else
        {
            while (--numSamples >= 0)
            {
                int index = (int)((currentAngle / twoPi) * tableSize);
                FloatType currentSample = level * subtable[index];
                //DBG(subtable[index]);
                
                for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);
                
                currentAngle += angleDelta;
                if (currentAngle >= twoPi)
                {
                    currentAngle -= twoPi;
                }
                ++startSample;
            }
        }
    }
}
