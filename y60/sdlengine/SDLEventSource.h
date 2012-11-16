/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below.
//
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/
//
//   $RCSfile: SDLEventSource.h,v $
//   $Author: janbo $
//   $Revision: 1.2 $
//   $Date: 2004/10/18 16:22:58 $
//
//
//=============================================================================
#ifndef _Y60_INPUT_SDLEVENTSOURCE_INCLUDED_
#define _Y60_INPUT_SDLEVENTSOURCE_INCLUDED_

#include "y60_sdlengine_settings.h"

#include <y60/input/Event.h>
#include <y60/input/IEventSource.h>
#include <y60/input/KeyCodes.h>

#include <SDL/SDL.h>
#include <asl/base/Ptr.h>

#include <vector>


class SDLEventSource : public y60::IEventSource {
    public:
        SDLEventSource();
        virtual void init();
        virtual std::vector<y60::EventPtr> poll();

    private:
        static std::vector<y60::KeyCode> myKeyCodeTranslationTable;
        y60::EventPtr createMouseMotionEvent(y60::Event::Type theType, const SDL_Event & theSDLEvent);
        y60::EventPtr createMouseButtonEvent(y60::Event::Type theType, const SDL_Event & theSDLEvent);
        y60::EventPtr createMouseWheelEvent(y60::Event::Type theType, const SDL_Event & theSDLEvent);
        y60::EventPtr createAxisEvent(const SDL_Event & theSDLEvent);
        y60::EventPtr createButtonEvent(y60::Event::Type theType, const SDL_Event & theSDLEvent);
        y60::EventPtr createKeyEvent(y60::Event::Type theType, const SDL_Event & theSDLEvent);
        void initJoysticks();
        void initTranslationTable();
};

#endif
