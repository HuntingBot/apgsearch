#pragma once
#include <string>
#include <sstream>

#include "happyhttp.h"

class ProcessedResponse {
public:
    int m_Status;           // Status-Code
    std::string m_Reason;   // Reason-Phrase
    int length;             // String length (in bytes)
    bool completed;         // Indicates whether successful
    std::stringstream contents;

    ProcessedResponse() {
        length = 0;
        m_Status = 0;
        completed = false;
    }
};

void OnBegin( const happyhttp::Response* r, void* userdata )
{
    ProcessedResponse* presp = static_cast<ProcessedResponse*>(userdata);

    presp->m_Status = r->getstatus();
    presp->m_Reason = r->getreason();
    presp->length = 0;
}

void OnData( const happyhttp::Response* r, void* userdata, const unsigned char* data, int n )
{
    ProcessedResponse* presp = static_cast<ProcessedResponse*>(userdata);
    for (int i = 0; i < n; i++)
        presp->contents << data[i];
    presp->length += n;
    (void) (r);
}

void OnComplete( const happyhttp::Response* r, void* userdata )
{
    ProcessedResponse* presp = static_cast<ProcessedResponse*>(userdata);
    presp->completed = true;
    (void) (r);
}

