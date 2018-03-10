## Desklights128, a fork of DeskLights2 ( https://github.com/mnlagrasta/DeskLights2 )

1. New function added to draw characters/strings on the table (needs parameter L before the string in C)
`http://server/write?l=1&c=H`
will draw an H
`http://server/write?l=5&c=hello`
will draw hello
2. Grid2Pixel function fixed for grid-style tables
3. UDP server added for audio VU, with the command
`http://server/vu?v=################`
where each # corresponds to a column (our table uses 16 columns) and the value of the # corresponds to a row height
`http://server/vu?v=1234567887654321`
would set

> - 1,1 to on
> - 2,1 to on
> - 2,2 to on
> - the entire column 8 & 9 etc

4. The web page response has had commands added and correct HTML headers to work with Safari

## Android Application Source (Glass, Phone and Wear)
https://github.com/AlecH92/DeskLights128.android

A new application was created to use as a notification tool straight from Android:
https://github.com/AlecH92/DeskLights128.notifier

## Pebble Application Source
https://github.com/AlecH92/DeskLights128.pebble

## Visualizer
This works using the program Spectrum Lab and its feature of exporting FFT to a built in web server using JSON data.
Source code: https://github.com/AlecH92/DeskLights128.visualizer