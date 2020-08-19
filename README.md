FONTXEdit FONTX font editor
===========================

**FONTXEdit** is a Qt port of the original windows program which can be downloaded [here](http://elm-chan.org/fsw_e.html#fontxedit)
Additional features include generating a C/C++ .h file containing a fontd definition for use in imbedded processors. Such fonts are
used by the [Hardware Agnostic Graphics Library](https://github.com/tuupola/hagl/tree/3e62a895a9072890a0484596cc0283d08fa439a8).

HAGL only comes with a 3 fonts (3 fonts: font5x7.h, font5x8.h and font6x9.h) and I wanted to use larger ones. In order to use a font in a project using HAGL, the font needs to be defined by means of a C .h header file. This header file can then be included in a project using HAGL.The originalFontXEdit program is a Windows program and therefore requires someting like [WINE](https://www.winehq.org/) to run on a Linux OS like Ubuntu. This port of FontX is done with Qt. Qt programs can be compiled and run on Windows, Linux and MacOS platforms. 

FNT file
--------
An FNT file is a font file used by the Windows operating system and often saved within the system fonts directory. It stores a generic font and may include vector and raster glyphs. FNT files have largely been replaced by TrueType (. TTF) and OpenType (. OTF) fonts. This is the preferred storage method for fonts tat can be edited by **FontXEdit**. 

BDF file
--------
BDF files (Bit Distribution Format) are text files that provide a macine independent method foe describinf fonts. **FontXEdit** can import these fonts and save them as .fnt or .h files.


An explanation of using FONTX fonts can be viewed [here](http://elm-chan.org/docs/dosv/fontx_e.html)
(HAGL)
