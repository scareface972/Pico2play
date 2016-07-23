# Pico2play
A modified pico2wave version to use text to speech without intermediate file, and without waiting time, the speech synthesis start before all the sentence was calculated.

Dependances
>sudo apt-get install portaudio19-dev   
>sudo apt-get install libttspico-dev (useless, file not found)

How to compile it (into the "pico" folder)
>gcc -Wall -I lib -g -O2 -c -o pico2play.o bin/pico2play.c   
>gcc -o pico2play pico2play.o -lpopt -lm lib/libttspico.a portaudio/libportaudio.a -lpthread -lasound -ljack   

To run it
>./pico2play -l fr-FR "test"   
