# Pico2play
A modified pico2wave version to use text to speech without intermediate file, and without waiting time, the speech synthesis start before all the sentence was calculated.

Dependances
>sudo apt-get install portaudio19-dev   
>(not working) sudo apt-get install libttspico-dev (file not found)

How to compile it (into the "pico" folder)
>make 

To run it
>./pico2play -l fr-FR "test"   

Specials options
>./pico2play -l fr-FR "<pitch level='100'><speed level='80'>test</speed></pitch>"

