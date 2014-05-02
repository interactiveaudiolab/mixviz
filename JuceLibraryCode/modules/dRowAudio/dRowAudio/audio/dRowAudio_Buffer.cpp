/*
  ==============================================================================

  This file is part of the dRowAudio JUCE module
  Copyright 2004-13 by dRowAudio.

  ------------------------------------------------------------------------------

  dRowAudio is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
  SOFTWARE.

  ==============================================================================
*/



Buffer::Buffer (int size)
    : bufferSize (size)
{
	buffer.allocate (bufferSize, true);
}

Buffer::Buffer (const Buffer& otherBuffer)
:	bufferSize (otherBuffer.bufferSize)
{
	buffer.allocate (bufferSize, false);
	memcpy (buffer, otherBuffer.buffer, bufferSize * sizeof (float));
}

Buffer::~Buffer()
{
}

void Buffer::setSize (int newSize)
{
	buffer.realloc (newSize);
	
	if (newSize > bufferSize)
		zeromem (buffer + bufferSize, (newSize - bufferSize) * sizeof (float));

	bufferSize = newSize;
}

void Buffer::applyBuffer (float* samples, int numSamples)
{
	const int numToApply = jmin (bufferSize, numSamples);
	for (int i = 0; i < numToApply; i++)
		samples[i] *= buffer[i];

	if (bufferSize < numSamples)
		zeromem (samples + numToApply, (numSamples - numToApply) * sizeof (float));
}

void Buffer::updateListeners()
{
    listeners.call (&Listener::bufferChanged, this);
}

//==============================================================================
void Buffer::addListener (Buffer::Listener* const listener)
{
    listeners.add (listener);
}

void Buffer::removeListener (Buffer::Listener* const listener)
{
    listeners.remove (listener);
}

