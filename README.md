# _Desklights128_
=============

## Desklights128, a fork of DeskLights2 ( https://github.com/mnlagrasta/DeskLights2 )

1. New function added to draw characters on the table
`http://server/write?c=H`
will draw an H
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

## Google Glass Application Source
https://github.com/AlecH92/DeskLights128.glass

## Android Application Source
https://github.com/AlecH92/DeskLights128.phone

## Pebble Application Source
https://github.com/AlecH92/DeskLights128.pebble

## Winamp Connector Application Source (for use with the VU command)
https://github.com/AlecH92/DeskLights128.winamp