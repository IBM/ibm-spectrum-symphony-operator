///////////////////////////////////////////////////////////////////////////////
//
// This file is a part of the SampleApp" sample which demonstrates the most
// basic use of the Symphony API allowing both synchronous and asynchronous
// clients to interact with a service. 
// This file contains code which demonstrates the use of the Symphony API
// within the client for this sample. 
//
//


//
// This exposed source code is the confidential and proprietary property of
// IBM Corporation. Your right to use is strictly limited by the terms of the
// license agreement entered into with IBM Corporation. 
//
///////////////////////////////////////////////////////////////////////////////

#include "../Common/MyMessage.h"
#include "MyCallback.h"
#include "soam.h"
#include "unistd.h"
#include <iostream>


using namespace soam;
using namespace std;

// A simple cross-platform wrapper for sleep(...). 
void ourSleep(unsigned int sleepInSeconds)
{
#ifdef WIN32
    soam::Sleep(sleepInSeconds * 1000);
#else
    sleep(sleepInSeconds);
#endif
}

int main(int argc, char* argv[])
{
    try
    {
        /********************************************************************
         * We should initialize the API before using any API calls. 
         ********************************************************************/
        SoamFactory::initialize();

        // Set up application specific information to be supplied to
        // Symphony. 
        const char appName[] = "SampleAppCPP";

        // Set up application authentication information using the default
        // security provider. Ensure it exists for the lifetime of the
        // connection. 
        DefaultSecurityCallback securityCB("Guest", "Guest");

        // Connect to the specified application. 
        ConnectionPtr conPtr = SoamFactory::connect(appName, &securityCB);

        // Retrieve and print our connection ID. 
        cout << "connection ID=" << conPtr->getId() << endl; 

        // Create session callback. 
        MySessionCallback  myCallback;

        // Set up session creation attributes. 
        SessionCreationAttributes attributes;
        attributes.setSessionName("mySession");
        attributes.setSessionType("ShortRunningTasks");
        attributes.setSessionFlags(Session::ReceiveAsync);
        attributes.setSessionCallback(&myCallback); // Associate callback
                                                    // with session. 

        // Create a session with the provided attributes. 
        SessionPtr sesPtr = conPtr->createSession(attributes);

        // Retrieve and print session ID. 
        cout << "Session ID:" << sesPtr->getId() << endl;

        // Now we will send some messages to our service. 
        int tasksToSend = 10;
        for (int taskCount = 0; taskCount < tasksToSend; taskCount++)
        {
            // Create a message. 
            char hello[] = "Hello Grid !!";
            soam::SoamDataBlockPtr data = createSoamDataBlock(taskCount);
            MyMessage inMsg(taskCount, false, hello, data);

            // Create task attributes. 
            TaskSubmissionAttributes attrTask;
            attrTask.setTaskInput(&inMsg);
            
            // Send it. 
            TaskInputHandlePtr input = sesPtr->sendTaskInput(attrTask);

            // Retrieve and print task ID. 
            cout << "task submitted with ID : " << input->getId() << endl;
        }
            
        // We will wait until all replies have been received asynchronously
        // by our callback. For illustrative purposes we will poll here until
        // all replies are in. 
        while ((myCallback.getReceived() < tasksToSend) && !myCallback.getDone()) 
        {
            ourSleep(2);
        }
    }
    catch(SoamException& exp)
    {
        // Report exception. 
        cout << "exception caught ... " << exp.what() << endl;
    }

    /************************************************************************
     * It is important that we always uninitialize the API. This is the only
     * way to ensure proper shutdown of the interaction between the client
     * and the system. 
     ************************************************************************/
    SoamFactory::uninitialize();

    cout << endl << "All Done !!" << endl;

    return 0;
}
