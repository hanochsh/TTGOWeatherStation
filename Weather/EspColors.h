#pragma once

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B

#define RGBColor(r,g,b)  ((byte)(r) << 11 | (byte)(g) << 5 | (byte)(b))

#define MyDarkGREY RGBColor(1,1,1)
#define FrameGRAY  RGBColor(2,2,2)