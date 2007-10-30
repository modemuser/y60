//============================================================================
//
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "DumbScale.h"

using namespace asl;
using namespace std;
using namespace dom;

namespace y60 {

    DumbScale::DumbScale(const std::string & theName) :
        Algorithm(theName),
        _myResultNode("result"),
        _mySourceRaster(0)
    {   
    }
  

    void 
    DumbScale::configure(const dom::Node & theNode) {
        
        for( unsigned int i=0; i<theNode.childNodesLength(); i++) {
            const std::string myName = theNode.childNode("property",i)->getAttribute("name")->nodeValue();
            const std::string myValue = theNode.childNode("property",i)->getAttribute("value")->nodeValue();
            dom::NodePtr myImage = _myScene->getSceneDom()->getElementById(myValue);
            AC_PRINT << "configure " << myName;
            if( myImage ) {
                if( myName == "sourceimage") {
                    _mySourceRaster = myImage->getFacade<y60::Image>()->getRasterValue();
                } else if( myName == "targetimage") {
                    _myTargetRaster =  myImage->getFacade<y60::Image>()->getRasterValue();
                    _myTargetImage = myImage->getFacade<y60::Image>();
                } 
            } else {
                if( myName == "width") {
                    asl::fromString(myValue, _myWidth);
                } else if( myName == "height") {
                    asl::fromString(myValue, _myHeight);
                }
            }
        }   
    }

	void 
    DumbScale::onFrame(double t) {
        
        const BGRRaster * mySourceFrame  = dom::dynamic_cast_Value<BGRRaster>(&*_mySourceRaster);
        const GRAYRaster * myTargetFrame = dom::dynamic_cast_Value<GRAYRaster>(&*_myTargetRaster);
        
        GRAYRaster::iterator itTrgt = const_cast<GRAYRaster::iterator>(myTargetFrame->begin());       
       
        unsigned int myLineSubCounter = 0;
        
        unsigned int myDebugCount = 0;
        bool myLineFlag = true; 
    
        for (BGRRaster::const_iterator itSrc = mySourceFrame->begin(); itSrc != mySourceFrame->end(); itSrc+=2) {
            
            // get only every second pixel in a line
            myLineSubCounter++;
            if (myLineSubCounter == (_myWidth/2)) {
                myLineSubCounter=0;
                
                myLineFlag = !myLineFlag; 
            }

            if (myLineFlag) {
                unsigned char myTarget = (*itTrgt).get();
                rgb_to_intensity((*itSrc)[2], (*itSrc)[1], (*itSrc)[0], myTarget);
                (*itTrgt).set(myTarget);

                itTrgt++;
                myDebugCount++;
            }
        }
        
        _myTargetImage->triggerUpload();
	}
}
