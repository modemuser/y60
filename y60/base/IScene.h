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
//   $Id: IScene.h,v 1.1 2005/03/24 23:35:56 christian Exp $
//   $RCSfile: IScene.h,v $
//   $Author: valentin $
//   $Revision: 1.1 $
//   $Date: 2005/03/24 23:35:56 $
//
//=============================================================================

#ifndef _ISCENE_INCLUDED
#define _ISCENE_INCLUDED

#include "y60_base_settings.h"

#include <asl/base/Ptr.h>
#include <asl/dom/ThreadingModel.h>

namespace y60 {
    class IResourceManager;
    class Y60_BASE_DECL IScene {
        public:
            virtual ~IScene();
            virtual const IResourceManager * getResourceManager() const = 0;
            virtual IResourceManager * getResourceManager() = 0;
    };

    typedef asl::Ptr<IScene, dom::ThreadingModel> IScenePtr;

}
#endif
