///////////////////////////////////////////////////////////////////////////////
//
// This file is a part of the SampleApp" sample which demonstrates the most
// basic use of the Symphony API allowing both synchronous and asynchronous
// clients to interact with a service. 
// This file contains code which is common to the service and client
// implementation of the sample. 
//


//
// This exposed source code is the confidential and proprietary property of
// IBM Corporation. Your right to use is strictly limited by the terms of the
// license agreement entered into with IBM Corporation. 
//
///////////////////////////////////////////////////////////////////////////////

#include "MyMessage.h"
#include "soam.h"
#include <iostream>

using namespace soam;

MyMessage::MyMessage()
{
    m_int = 0;
    m_string = copyString("");
    m_data = NULL;
}

MyMessage::MyMessage(int i, bool isSync, char* str, soam::SoamDataBlockPtr& data)
{
    m_int = i;
    m_isSync = isSync;
    m_string = copyString(str);
    m_data = data;
}

MyMessage::~MyMessage(void)
{
    freeString(m_string);
}

void MyMessage::onSerialize(OutputStreamPtr &stream) throw (SoamException)
{
    stream->write(m_int);
    stream->write(m_isSync);
    stream->write(m_string);
    stream->write(m_data);
}

void MyMessage::onDeserialize(InputStreamPtr &stream) throw (SoamException)
{
    stream->read(m_int);
    stream->read(m_isSync);

    // we now own the string that was read
    freeString(m_string);
    stream->read(m_string);
    stream->read(m_data);
}

char* MyMessage::copyString(const char* strSource)
{
    SOAM_ASSERT(0 != strSource);

    size_t len = strlen(strSource);
    char* newString = new char[len+1];
    
    SOAM_ASSERT(0 != newString);
    strcpy(newString, strSource);

    return newString;
}

void MyMessage::freeString(char* strToFree)
{
    if (0 != strToFree)
    {
        delete []strToFree;
    }
}

// Creates a SoamDataBlock and fills the data depending on task count. 
soam::SoamDataBlockPtr createSoamDataBlock(int taskCount)
{
    size_t  data_size = ((taskCount + 1) * 100);
    char    data_value = (char)taskCount;

    SoamDataBlockPtr    data(new SoamDataBlock(data_size));
    if(data->buf())
    {
        memset(data->buf(), data_value, data->size());
    }

    return data;
}

// Validates if data in SoamDataBlock is as filled in createSoamDataBlock. 
bool isValidSoamDataBlock(soam::SoamDataBlockPtr const& data, int taskCount)
{
    bool    ret = false;

    size_t  data_size = ((taskCount + 1) * 100);
    char    data_value = (char)taskCount;

    if(!data.isNull() && (data_size == data->size()))
    {
        ret = true;
        const char*     data_buf = data->buf();
        for(size_t i = 0; i < data_size; ++i)
        {
            if(data_buf[i] != data_value)
            {
                ret = false;
                break;
            }
        }
    }

    return ret;
}
