# Multimedia Framework in C++ #

A C++ based framework for creating objects which can be connected in a push or pull driven chain.

### Few simple apps based on the framework ###
[yuvplayer](yuvplayer.md)

## Source organization ##

At present the code is organized as described below. http://code.google.com/p/mmfcpp/source/browse/#svn%2Ftrunk

media\_core - The core framework http://code.google.com/p/mmfcpp/source/browse/trunk/media_core/include/abstract_media_object.h

## Available components ##

#### yuv\_file\_src - Yuv file source object ####

#### video\_renderer - Video renderer object ####

#### audio\_renderer - ALSA based audio renderer ####

#### wave\_file\_src - A wave file source object ####

#### av\_file\_src - ffmpeg/livavformat based file source ####

#### avcodec\_audio\_decoder - ffmpeg/libavformat based audio decoder ####

#### avcodec\_video\_decoder - ffmpeg/libavformat based video decoder ####

## How to get the sources ##
#### Checkout the sources on linux console using the instructions below ####
#### svn checkout http://mmfcpp.googlecode.com/svn/trunk/ mmfcpp ####