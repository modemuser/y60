//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSARROW_INCLUDED_
#define _Y60_ACGTKSHELL_JSARROW_INCLUDED_

#include "JSMisc.h"

#include <asl/base/string_functions.h>
#include <y60/jsbase/JSWrapper.h>

#include <gtkmm/arrow.h>


namespace jslib {

class JSArrow : public JSWrapper<Gtk::Arrow, asl::Ptr<Gtk::Arrow>, StaticAccessProtocol> {
    private:
        JSArrow();  // hide default constructor
        typedef JSMisc JSBASE;
    public:
        virtual ~JSArrow() {
        }
        typedef Gtk::Arrow NATIVE;
        typedef asl::Ptr<Gtk::Arrow> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "Arrow";
        };
        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
            PROP_END = JSBASE::PROP_END
        };

        static JSPropertySpec * Properties();

        virtual unsigned long length() const {
            return 1;
        }

        static JSBool getPropertySwitch(NATIVE & theNative, unsigned long theID,
                JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        static JSBool setPropertySwitch(NATIVE & theNative, unsigned long theID, JSContext *cx,
                JSObject *obj, jsval id, jsval *vp);
        virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

        static JSBool
        Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        static
        JSObject * Construct(JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
            return Base::Construct(cx, theOwner, theNative);
        }

        JSArrow(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSArrow & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSArrow &>(JSArrow::getJSWrapper(cx,obj));
        }

    private:
};

template <>
struct JSClassTraits<JSArrow::NATIVE>
    : public JSClassTraitsWrapper<JSArrow::NATIVE, JSArrow> {};

jsval as_jsval(JSContext *cx, JSArrow::OWNERPTR theOwner, JSArrow::NATIVE * theArrow);

} // namespace

#endif

