///////////////////////////////////////////////////////////////////////////////
//
// This file is a part of the SampleApp" sample which demonstrates the most
// basic use of the Symphony API allowing both synchronous and asynchronous
// clients to interact with a service. 
// This file contains code which is related to the service implementation of
// the sample. 
//
//


//
// This exposed source code is the confidential and proprietary property of
// IBM Corporation. Your right to use is strictly limited by the terms of the
// license agreement entered into with IBM Corporation. 
//
///////////////////////////////////////////////////////////////////////////////

#include "../Common/MyMessage.h"
#include "soam.h"
#include <iostream>

using namespace soam;
using namespace std;

class MyServiceContainer : public ServiceContainer
{
public:

    virtual void onCreateService(ServiceContextPtr& serviceContext)
    {
        /********************************************************************
         * Do your service initialization here. 
         ********************************************************************/
    }

    virtual void onSessionEnter(SessionContextPtr& sessionContext)
    {
        /********************************************************************
         * Do your session-specific initialization here, when common data is
         * provided. 
         ********************************************************************/
    }

    virtual void onInvoke(TaskContextPtr& taskContext)
    {
        /********************************************************************
         * Do your service logic here. This call applies to each task
         * submission. 
         ********************************************************************/
        
        // Get the input that was sent from the client 
        MyMessage inMsg;
        taskContext->populateTaskInput(inMsg);

        // We simply echo the data back to the client 
        MyMessage outMsg;
        outMsg.setInt(inMsg.getInt());
        // We return received SoamDataBlock as is. 
        outMsg.setData(inMsg.getData());

        std::string str = "you sent : ";
        str += inMsg.getString();
        str += "\nwe replied : Hello Client !!\n>>> ";

        if (inMsg.getIsSync())
        {
            str += "Synchronously.\n";
        }
        else
        {
            str += "Asynchronously.\n";
        }
        outMsg.setString(str.c_str());

        // Set our output message 
        taskContext->setTaskOutput(outMsg);
    }
    
    virtual void onSessionLeave()
    { 
        /********************************************************************
         * Do your session-specific uninitialization here, when common data
         * is provided. 
         ********************************************************************/
    }

    virtual void onSessionLeave(SessionContextPtr & sessionContext)
    {
        /********************************************************************
         * Do your session-specific uninitialization here, when common data
         * is provided. 
         ********************************************************************/
    }

    virtual void onDestroyService()
    {
        /********************************************************************
         * Do your service uninitialization here. 
         ********************************************************************/
    }

    virtual void onApplicationAttach(ServiceContextPtr& serviceContext)
    {
        std::cout << "onApplicationAttach()" << std::endl;
    }

    virtual void onApplicationDetach()
    {
        std::cout << "onApplicationDetach()" << std::endl;
    }
};


// Entry point to the service 
int main(int argc, char* argv[])
{
    // Return value of our service program 
    int retVal = 0;

    try
    {
        /********************************************************************
         * Do not implement any service initialization before calling the
         * ServiceContainer::run() method. If any service initialization
         * needs to be done, implement the onCreateService() handler for your
         * service container. 
         ********************************************************************/
        
        // Create the container and run it 
        MyServiceContainer myContainer;
        myContainer.run();
        
        /********************************************************************
         * Do not implement any service uninitialization after calling the
         * ServiceContainer::run() method. If any service uninitialization
         * needs to be done, implement the onDestroyService() handler for
         * your service container since there is no guarantee that the
         * remaining code in main() will be executed after calling
         * ServiceContainer::run(). Also, in some cases, the remaining code
         * can even cause an orphan service instance if the code cannot be
         * finished. 
         ********************************************************************/
    }
    catch(SoamException& exp)
    {
        // Report the exception to stdout 
        cout << "exception caught ... " << exp.what() << endl;
        retVal = -1;
    }

    /************************************************************************
     * NOTE: Although our service program will return an overall failure or
     * success code it will always be ignored in the current revision of the
     * middleware. The value being returned here is for consistency. 
     ************************************************************************/
    return retVal;
} 
