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

#ifndef _ac_jslib_JSFrameSocket_h_
#define _ac_jslib_JSFrameSocket_h_

#include "y60_net_settings.h"

#include <y60/jsbase/JSWrapper.h>
#include <asl/net/FrameSocket.h>


class JSFrameSocket : public jslib::JSWrapper<inet::FrameSocket, asl::Ptr<inet::FrameSocket> , jslib::StaticAccessProtocol> {
        JSFrameSocket() {}
    public:
        typedef inet::FrameSocket NATIVE;
        typedef asl::Ptr<NATIVE> OWNERPTR;
        typedef jslib::JSWrapper<NATIVE,OWNERPTR, jslib::StaticAccessProtocol> Base;

        JSFrameSocket(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        ~JSFrameSocket();

        static const char * ClassName() {
            return "FrameSocket";
        }
        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
        };
        static JSPropertySpec * Properties();

        static JSPropertySpec * StaticProperties();
        static JSFunctionSpec * StaticFunctions();

        virtual unsigned long length() const {
            return 1;
        }

        static JSBool
        Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        static
        JSObject * Construct(JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
            return Base::Construct(cx, theOwner, theNative);
        }

        static jslib::JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);

        static JSFrameSocket & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSFrameSocket &>(JSFrameSocket::getJSWrapper(cx,obj));
        }
};

namespace jslib {
template <>
struct JSClassTraits<JSFrameSocket::NATIVE>
    : public JSClassTraitsWrapper<JSFrameSocket::NATIVE , JSFrameSocket> {};

} // namespace jslib

Y60_NET_DECL bool convertFrom(JSContext *cx, jsval theValue, JSFrameSocket::NATIVE & theFrameSocket);

Y60_NET_DECL jsval as_jsval(JSContext *cx, JSFrameSocket::OWNERPTR theOwner);
Y60_NET_DECL jsval as_jsval(JSContext *cx, JSFrameSocket::OWNERPTR theOwner, JSFrameSocket::NATIVE * theFrameSocket);

#endif

