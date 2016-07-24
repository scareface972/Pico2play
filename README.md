# Pico2play
A modified pico2wave version (Speech synthesis application very used on raspberry) to use text to speech without intermediate file, and without waiting time, the speech synthesis start before all the sentence was calculated.

Dependances
>sudo apt-get install portaudio19-dev   
>(not working) sudo apt-get install libttspico-dev (file not found)

How to compile it (into the "pico" folder)
>make 

To run it
>./pico2play -l fr-FR "test"   

Specials options (already in pico lib,just to remember)
>./pico2play -l fr-FR "\<pitch level='100'>\<speed level='80'>test\</speed>\</pitch>"

TODO:   
Update the makefile in the lib folder to generate the libportaudio.a file.   
The portaudio folder is useless ATM.
