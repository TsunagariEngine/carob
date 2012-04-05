/*********************************
** Tsunagari Tile Engine        **
** sound.cpp                    **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#include <boost/shared_ptr.hpp>
#include <Gosu/Audio.hpp>

#include "python.h"
#include "sound.h"

Sound::Sound(Gosu::Sample* source)
	: source(source)
{
}

void Sound::play()
{
	source->play();
}


void exportSound()
{
	boost::python::class_<Sound, boost::shared_ptr<Sound> >
		("Sound", boost::python::no_init)
		.def("play", &Sound::play);
}

