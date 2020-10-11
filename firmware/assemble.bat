avr-gcc -Wall -O2 -Iusbdrv -Ilcd -I. -mmcu=atmega8 -DF_CPU=12000000 -S -o source.S main.c
pause
