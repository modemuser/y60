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

/*globals use, plug, UnitTest, millisec, msleep, Async, ENSURE, SUCCESS, FAILURE, gc, UnitTestSuite,
          print, exit, RequestManager, Request, Logger */

use("UnitTest.js");

plug("NetAsync");

var TIMEOUT = 1000;

function HttpClientUnitTest() {
    this.Constructor(this, "HttpClientUnitTest");
}

HttpClientUnitTest.prototype.Constructor = function (obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    function testFireAndForget() {
        var done = false;

        Logger.info("creating client");
        var myClient = new Async.HttpClient({
            url: "http://127.0.0.1:3003/foo",
            success: function () {
                SUCCESS("FireAndForget");
                done = true;
            },
            error: function () {
                FAILURE("FireAndForget");
                done = true;
            }
        });
        myClient = null;
        gc();

        while (true) {
            Async.onFrame();
            gc();
            msleep(50);
            if (done) {
                break;
            }
        }
    }
    function testBlocking() {
        var done = false;
        obj.myClient = new Async.HttpClient({
            url: "http://127.0.0.1:88/foo",
            async: false,
            verbose: true,
            success: function() {
                done = true;
            },
            error: function() {
                done = true;
            }
        });

        if (done) {
            SUCCESS("testBlocking");
        } else {
            FAILURE("testBlocking");
        }
    }
    function testError() {
        var done = false;
        Logger.info("creating client");
        obj.myClient = new Async.HttpClient({
            url: "http://127.0.0.1:8765/foobar",
            connecttimeout: 5,
            success: function() {
                FAILURE("testError");
                done = true;
            },
            error: function() {
                SUCCESS("testError");
                done = true;
            }
        });


        while (true) {
            Async.onFrame();
            gc();
            msleep(50);
            if (done) {
                break;
            }
        }
    }
    function testBigRequest() {
        var done = false;

        var received = 0;
        Logger.info("creating client");
        obj.myClient = new Async.HttpClient({
            url: "http://127.0.0.1:3003/big",
            // url: "http://files.t-gallery.act/data/repository/original/vol0/24/vater_der_braut_de_hd_eff5390ebdbdf0bd62f86b716f8f8adf3d9512d6.mp4",
            progress: function(theBlock) {
                received += theBlock.size;
                theBlock.resize(0);
                return true;
            },
            error: function() {
                FAILURE("testBigRequest");
                done = true;
            },
            success: function() {
                print("received in Progress: "+received);
                print("last responseBlock:"+this.responseBlock.size);
                done = true;
                if (received + this.responseBlock.size === 1024*1024*5) {
                    SUCCESS("testBigRequest");
                } else {
                    FAILURE("testBigRequest");
                }
            }
        });


        while (true) {
            Async.onFrame();
            gc();
            msleep(1);
            if (done) {
                break;
            }
        }
    }

    function testRequestProgressError() {
        var done = false;

        var received = 0;
        Logger.info("creating client");
        obj.myClient = new Async.HttpClient({
            url: "http://127.0.0.1:3003/big",
            progress: function(theBlock) {
                return false;
            },
            error: function(a, b) {
                print (a,b);
                SUCCESS("testRequestProgressError");
                done = true;
            },
            success: function() {
                FAILURE("testRequestProgressError");
                done = true;
            }
        });


        while (true) {
            Async.onFrame();
            gc();
            msleep(1);
            if (done) {
                break;
            }
        }
    }
    
    function testRequestAbort() {
        var done = false;

        var received = 0;
        Logger.info("creating client");
        obj.myClient = new Async.HttpClient({
            url: "http://127.0.0.1:3003/big",
            error: function(a, b) {
                print (a,b);
                SUCCESS("testRequestAbort");
                done = true;
            },
            success: function() {
                FAILURE("testRequestAbort");
                done = true;
            }
        });


        while (true) {
            Async.onFrame();
            gc();
            msleep(1);
            obj.myClient.abort();
            if (done) {
                break;
            }
        }
    }
    function testSmallRequests() {
        // test many small requests
        var i = 10;

        var iterate = function() {
            if (i>0) {
                obj.myClient = new Async.HttpClient({
                    url: "http://127.0.0.1:3003/foo",
                    success: function() {
                        SUCCESS("testSmallRequests");
                        i--;
                        iterate();
                    },
                    error: function() {
                        FAILURE("testSmallRequests");
                        i = 0;
                    }
                });
            }
        };
        iterate();
        while (true) {
            Async.onFrame();
            gc();
            msleep(50);
            if (i <= 0) {
                break;
            }
        }
    }

    obj.run = function () {
        obj.myServer = new Async.HttpServer();
        var myServer = {
            foo: function(theMethod, theBody) {
                     return "ok";
                 },
            big: function(theMethod, theBody) {
                return new Array(1024*1024+1).join("Fubar"); // 100 MB
                 }

        };
        obj.myServer.start("0.0.0.0", "3003");
        obj.myServer.registerCallback("/foo", myServer, myServer.foo);
        obj.myServer.registerCallback("/big", myServer, myServer.big);

        testBlocking();
        testFireAndForget();
        testError();
        testSmallRequests();
        testBigRequest();
        testRequestProgressError();
        testRequestAbort();

        obj.myServer = null;
        gc();
    };

};

var myTestName = "testHttpClient.tst.js";
var mySuite = new UnitTestSuite(myTestName);

mySuite.addTest(new HttpClientUnitTest());
mySuite.run();

print(">> Finished test suite '" + myTestName + "', return status = " + mySuite.returnStatus() + ".");
exit(mySuite.returnStatus());
