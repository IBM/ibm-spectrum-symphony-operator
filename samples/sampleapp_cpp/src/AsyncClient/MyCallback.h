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

#include "soam.h"
#include "MyMessage.h"
#include <iostream>

using namespace soam;
using namespace std;

#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

class MySessionCallback : 
    public SessionCallback
{
    public:
        MySessionCallback()
            :m_tasksReceived(0), m_exception(false) 
        {

#ifndef WIN32
            pthread_mutexattr_t attr;
            pthread_mutexattr_init( &attr );
            pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE ); 

            pthread_mutex_init( &m_mutex, &attr );

            pthread_mutexattr_destroy( &attr );
#else
            InitializeCriticalSection(&m_criticalSection); 
#endif

            cout << "Callback created ... " << endl;
        }
        virtual ~MySessionCallback() 
        {
#ifndef WIN32
            pthread_mutex_destroy( &m_mutex );
#else
            DeleteCriticalSection(&m_criticalSection);
#endif
        }

        ////////////////////////////////////////////////////////////////////////
        // This handler is called once any exception occurs within the scope
        // of the session. 
        ////////////////////////////////////////////////////////////////////////
        virtual void onException(SoamException &exception) throw()
        {
            /****************************************************************
             * Although the Symphony API currently invokes this method with a
             * single callback thread, the number of threads used by the API
             * to invoke this method may increase in future revisions of the
             * API. Therefore, the developer should never assume that the
             * invocation of this method will be done serially and should
             * always implement this method in a thread-safe manner. 
             ****************************************************************/
            
            cout << "An exception occured on the callback.\nDetails : " <<  exception.what() << endl;

#ifndef WIN32
            pthread_mutex_lock( &m_mutex);
#else
            EnterCriticalSection(&m_criticalSection); 
#endif
            m_exception = true;
#ifndef WIN32
            pthread_mutex_unlock( &m_mutex);
#else
            LeaveCriticalSection(&m_criticalSection);
#endif

        }

        ////////////////////////////////////////////////////////////////////////
        // This handler is called once a message is returned asynchronously
        // from the system when a task completes. 
        ////////////////////////////////////////////////////////////////////////
        void onResponse(TaskOutputHandlePtr &output) throw()
        {
            /****************************************************************
             * Although the Symphony API currently invokes this method with a
             * single callback thread, the number of threads used by the API
             * to invoke this method may increase in future revisions of the
             * API. Therefore, the developer should never assume that the
             * invocation of this method will be done serially and should
             * always implement this method in a thread-safe manner. 
             ****************************************************************/
            
            try
            {
                // Check for success of task. 
                if (true == output->isSuccessful())
                {
                    // Get the message returned from the service. 
                    MyMessage outMsg;
                    output->populateTaskOutput(&outMsg);

                    // Display content of reply. 
                    cout << "Task Succeeded [" <<  output->getId() << "]" << endl;
                    cout << "Integer Value : " << outMsg.getInt() << endl;
                    if(isValidSoamDataBlock(outMsg.getData(), outMsg.getInt()))
                    {
                        cout << "Data block size: " << outMsg.getData()->size() << endl;
                    }
                    else
                    {
                        cout << "Invalid data block!" << endl;
                    }
                    cout << outMsg.getString() << endl;
                }
                else
                {
                    // Get the exception associated with this task. 
                    SoamExceptionPtr ex = output->getException();
                    cout << "Task Not Successful : " << ex->what() << endl;
                }

            }
            catch(SoamException &exception)
            {
                cout << "Exception occured in OnResponse() : " << exception.what() << endl;
            }
            
            // Update counter used to synchronize the controlling thread with
            // this callback object. 
#ifndef WIN32
            pthread_mutex_lock( &m_mutex);
#else
            EnterCriticalSection(&m_criticalSection); 
#endif
            ++m_tasksReceived;
#ifndef WIN32
            pthread_mutex_unlock( &m_mutex);
#else
            LeaveCriticalSection(&m_criticalSection);
#endif
        }
        
        ////////////////////////////////////////////////////////////////////////
        // This method is invoked when a task sending status is available. It
        // is optional to be implemented by the application developer. To
        // enable this function, use flag ReceiveAsyncTaskSentStatus when
        // creating a session. 
        ////////////////////////////////////////////////////////////////////////
        void onTaskSent(TaskInputHandlePtr &taskInput) throw(SoamException)
        {
            try
            {
                cout << "onTaskSent is invoked, the task state is <" <<  taskInput->getSubmissionState() << ">." << endl;
            }
            catch(SoamException &exception)
            {
                cout << "Exception occured in onTaskSent() : " << exception.what() << endl;
            }
        }

        inline long getReceived() 
        { 
            return m_tasksReceived; 
        }

        inline bool getDone()
        {
            return m_exception;
        }
                   
    private:
#ifndef WIN32
        pthread_mutex_t m_mutex;
#else
        CRITICAL_SECTION m_criticalSection;
#endif

        long m_tasksReceived;
        bool m_exception;
};
