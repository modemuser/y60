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
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/
//
//   $RCSfile: Task.h,v $
//   $Author: valentin $
//   $Revision: 1.12 $
//   $Date: 2005/02/23 15:29:10 $
//
//
//=============================================================================

#ifndef _ac_y60_Task_h_
#define _ac_y60_Task_h_

#include <asl/base/Ptr.h>
#include <string>
#include <vector>
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>

namespace y60 {
            
    enum TaskCallback;
    class TaskWindow;

    class Task {
        public:
            enum PRIORITY {
                ABOVE_NORMAL = -99,
                BELOW_NORMAL,
                HIGH_PRIORITY,
                IDLE_PRIORITY,
                NORMAL_PRIORITY,
                REALTIME_PRIORITY
            };

            Task(std::string theCommand, std::string thePath, bool theShowFlag);
            Task();
            ~Task();
           
            // Terminates a process non-gracefully
            void terminate(); 
            
            // returns if the task is ready for user input or timeout expired
            bool waitForInputIdle(unsigned theTimeout);
            
            /// Takes a screenshot
            void captureDesktop(std::string theFilename); 
            
            bool isActive() const;  
            void setPriority(int thePrio);     

            /// Add windows to the task that do not belong to the process
            void addExternalWindow(std::string theWindowName);

            /// Returns the name of the active window
            std::string getActiveWindowName() const;

            /// Returns all Windows attached to this task
            std::vector<asl::Ptr<TaskWindow> > getWindows() const;

            void addTaskWindow(HWND theWindow) {
                _myTaskWindows.push_back(theWindow);
            }

        private:
            void collectWindows() const;
            HWND getActiveWindow() const;

            PROCESS_INFORMATION       _myProcessInfo;
            mutable std::vector<HWND> _myTaskWindows;
            std::vector<HWND>         _myExternalWindows;
    };
}

#endif