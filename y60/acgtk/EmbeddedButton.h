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

#ifndef ACGTK_EMBEDDED_BUTTON_INCLUDED
#define ACGTK_EMBEDDED_BUTTON_INCLUDED

#include "y60_acgtk_settings.h"

#if defined(_MSC_VER)
#pragma warning(push,1)
#endif //defined(_MSC_VER)
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4250 4275)
#endif //defined(_MSC_VER)

namespace acgtk {

class Y60_ACGTK_DECL EmbeddedButton : public Gtk::EventBox {
    public:
        EmbeddedButton();
        EmbeddedButton(const std::string & theIconFile);

        sigc::signal<void> signal_clicked() const { return _myClickedSignal; }
        virtual void released();
        virtual void leave();
        virtual void pressed();

    protected:
        void setup();
        virtual bool on_leave_notify_event(GdkEventCrossing * theEvent);
        virtual bool on_button_press_event(GdkEventButton * theEvent);
        virtual bool on_button_release_event(GdkEventButton * theEvent);
        virtual void on_clicked();
        virtual void on_realize();

        Gtk::Image                _myImage;
        Glib::RefPtr<Gdk::Pixbuf> _myIcon;
    private:

        bool                      _myClickAheadFlag;
        sigc::signal<void>        _myClickedSignal;

};

};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

#endif // ACGTK_EMBEDDED_BUTTON_INCLUDED



