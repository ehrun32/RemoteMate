# RemoteMate 🦾

A speech recognizer for Spotify commands

## Project Summary

This project is intended to create a speech recognition model that can be used to recognize Spotify commands. In the future, this model will be connected to a computer where given voice input, the model will process and recognize the command and execute it.

## Methodology

### Training Data Creation

1. Recorded 10 sample commands
2. Used Spotify's Pedalboard Audio Processor, to change: Reverb, Pitch, Loudness
3. Created 2000 examples for each sample command


## Next steps

Things to improve:

1. Train model 2 for longer (after getting hardware upgrade 😢, or use NVIDIA's cloud computing)
2. Implement execution on a computer

## References

-   [DeepSpeech2 — OpenSeq2Seq 0.2 documentation (nvidia.github.io)](https://nvidia.github.io/OpenSeq2Seq/html/speech-recognition/deepspeech2.html)
-   [Automatic Speech Recognition using CTC (keras.io)](https://keras.io/examples/audio/ctc_asr/)
-   [spotify/pedalboard: 🎛 🔊 A Python library for manipulating audio. (github.com)](https://github.com/spotify/pedalboard)

