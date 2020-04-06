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

#ifndef CPP_8E50D01D_B9A5_4753_A34B_A6DCEB5B9867_SAMPLEAPP
#define CPP_8E50D01D_B9A5_4753_A34B_A6DCEB5B9867_SAMPLEAPP

#include "soam.h"


class MyMessage :
    public soam::Message
{
public:
    MyMessage();
    MyMessage(int i, bool isSync, char* str, soam::SoamDataBlockPtr& data);
    virtual ~MyMessage(void);

    void onSerialize(
        /*[in]*/ soam::OutputStreamPtr &stream) throw (soam::SoamException); 

    void onDeserialize(
    /*[in]*/ soam::InputStreamPtr &stream) throw (soam::SoamException); 

// accessors
public:
    int getInt() const{return m_int;}
    void setInt(int _int) {m_int = _int;}
    const char* getString() const {return m_string;}
    void setString(const char* str) {freeString(m_string); m_string = copyString(str);}
    bool getIsSync() const {return (m_isSync != 0);}
    void setIsSync(bool isSync) {m_isSync = isSync;}
    soam::SoamDataBlockPtr& getData() { return m_data; }
    void setData(soam::SoamDataBlockPtr& data) { m_data = data; }

private:
    char* copyString(const char* strSource);
    void freeString(char* strToFree);

private:
    int m_int;
    bool m_isSync;
    char* m_string;
    soam::SoamDataBlockPtr m_data;
};

soam::SoamDataBlockPtr createSoamDataBlock(int taskCount);
bool isValidSoamDataBlock(soam::SoamDataBlockPtr const& data, int taskCount);

#endif
