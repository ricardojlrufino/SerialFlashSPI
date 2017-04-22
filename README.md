SerialFlashSPI
===

Flash Winbond / simimilar chips using Serial  
Using library: https://github.com/Marzogh/SPIFlash  
TIP: You can use a program like "Cutecom" to send files  

NOTE: In the tests I did using higher baud in the serial but the maximum  
that worked out was 115200 - Note that this is very slow, +/- 5 min for 4MB 

You should adjust the base file size to the multiples of 256. 
This can be configured directly on the serial console

## Wires
Remember that most flash memories are 3.3v and most arduinos are 5v. 
You can use a 3.3v arduino (ex.: Due) or make a signal conversion

![Diagram](/docs/diagram.png?raw=true "Diagram")

## Sequence

![Diagram](/docs/cute.png?raw=true "Diagram")


1. Press e - Erase
2. Press s - Set size to 512:  s 512
3. Press w - (Next click on 'Send File')


# For Dump

You need enable "Log to" in Cutecom.

After, remove non-data from beginning and end of file.

And convert to binary file using XDD

> xxd -r -p input.txt output.bin
