Use this repo to replace your plugins folder. Not the most elegant at the moment - should eventually fix.

### Development Notes

Signals appear to be floats that represent the actual voltage values one would expect. For example, the SEQ3 outputs 0-10V
to input into a 1V/O of an oscillator giving it a 10 octave range.

Similarly, we want our output audio signals to then be -5V to 5V.

Therefore, all we need to do is map 0 to 10V.

The Fundamental plugin oscillators default to **262Hz**, so we should default to 262Hz
