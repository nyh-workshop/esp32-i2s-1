# ESP32 I2S FM synth player
This is an ESP32 version of the FM synth player for the PIC32MX: https://github.com/uncle-yong/sk1632-i2s-dma
Since the ESP32 contains an FPU, all the calculations are done using floating point unit instead of fixed-point.
There are six FM channels in this program. Operating it with more than six FM channels has not been tested yet.

The DAC for this program is a PCM5102 module, which can be easily obtained from the eBay.

The score has been converted from MIDI to byte array by using Len Shustek's miditones: https://github.com/LenShustek/miditones

Limitations: 
- There are very faint clicking sounds between switching notes. The clicking in between some very fast notes are not being fully damped yet.
- Due to preventing the loud clicking sounds between switching notes, note stop commands are being placed after the note has been played. Only the older version of the miditones have this feature, which is included inside the repository.
