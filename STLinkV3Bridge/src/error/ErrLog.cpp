/**
  ******************************************************************************
  * @file    ErrLog.cpp
  * @author  MCD Application Team
  * @brief   All tracing mechanisms (Error, Trace)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ErrLog.h"
#include <iostream>
#include <iomanip>
#ifdef WIN32
#include <stdlib.h> // for _countof
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// static variables

/* Global variables ----------------------------------------------------------*/
/* Class Functions Definition ------------------------------------------------*/

/*****************************************************************************/    
cErrLog::cErrLog()
{
    // Get date and time to put at top of log
    time(&now);

    // Start a high res clock for timestamps
    start = std::chrono::_V2::high_resolution_clock::now();

    // Debug output indicating log has started
    std::cerr << "ST-LINK Bridge DLL error trace log started "
              << ctime(&now) << std::endl;
}

/*****************************************************************************/    
void cErrLog::Init (const char *pSzFileName, bool bResetFile)
{
    // Get now
    auto logTime = std::chrono::_V2::high_resolution_clock::now();
    // Get the timestamp for the log entry
    auto timeStampHRS = std::chrono::duration_cast<std::chrono::hours>(logTime - start);
    auto timeStampMIN = std::chrono::duration_cast<std::chrono::minutes>(logTime - start);
    auto timeStampSEC = std::chrono::duration_cast<std::chrono::seconds>(logTime - start);
    auto timeStampMSC = std::chrono::duration_cast<std::chrono::milliseconds>(logTime - start);

    if(bResetFile)
    {
        // File is not opened for append
        file.open(pSzFileName);

        if(!file.is_open())
        {
            // Signal error to debug output
            std::cerr << "Unable to open file - " << pSzFileName << " for output" << std::endl;
        }
        else
        {
            // Initial entry into log file is the date and time
            file << "["
                 << std::setw(2) << std::setfill('0') << timeStampHRS.count() << ":"
                 << std::setw(2) << std::setfill('0') << timeStampMIN.count() << ":"
                 << std::setw(2) << std::setfill('0') << timeStampSEC.count() << "."
                 << std::setw(4) << std::setfill('0') << timeStampMSC.count()
                 << "] ST-LINK Bridge DLL error trace log started "
                 << ctime(&now);
        }
    }
    else
    {
        // File opened for append
        file.open(pSzFileName, std::ios::app);

        if(!file.is_open())
        {
            // Signal error to debug output
            std::cerr << "Unable to open file - " << pSzFileName << " for append" << std::endl;
        }
        else
        {
            // Inital entry into log file is the date and time
            file << "["
                 << std::setw(2) << std::setfill('0') << timeStampHRS.count() << ":"
                 << std::setw(2) << std::setfill('0') << timeStampMIN.count() << ":"
                 << std::setw(2) << std::setfill('0') << timeStampSEC.count() << "."
                 << std::setw(4) << std::setfill('0') << timeStampMSC.count()
                 << "] ST-LINK Bridge DLL error trace log started "
                 << ctime(&now);
        }
    }

}
/*****************************************************************************/    
void cErrLog::Dump()
{
	// To be completed: implementation defined
	// possible implementation:
	// if records available, open errorLog file
	// copy errorLog buffer in the errorLog file, add timestamp ...
	// close errorLog file
	// reset errorLog buffer

    std::cerr << "Dump() called, but no implementation defined" << std::endl;
}

void cErrLog::LogTrace(const char *pMessage, ...)
{
	// Trace the specified string into log file

	va_list args; // used to manage the variable argument list
	va_start(args, pMessage);     

	LogTrace(pMessage, args);

	va_end(args);     
}

void cErrLog::LogTrace(const char *pMessage, va_list Args)
{
	// Trace the specified string into log file

    // Get now
    auto logTime = std::chrono::_V2::high_resolution_clock::now();
    // Get the timestamp for log entry
    auto timeStampHRS = std::chrono::duration_cast<std::chrono::hours>(logTime - start);
    auto timeStampMIN = std::chrono::duration_cast<std::chrono::minutes>(logTime - start);
    auto timeStampSEC = std::chrono::duration_cast<std::chrono::seconds>(logTime - start);
    auto timeStampMSC = std::chrono::duration_cast<std::chrono::milliseconds>(logTime - start);

    // Since this function is using c-style va_list
    //  we make a string from it
    char buf[LOG_TRACE_BUF_SIZE];
    vsprintf(buf, pMessage, Args);

    // We will also send all error log traces to debug output
    std::cerr << "["
              << std::setw(2) << std::setfill('0') << timeStampHRS.count() << ":"
              << std::setw(2) << std::setfill('0') << timeStampMIN.count() << ":"
              << std::setw(2) << std::setfill('0') << timeStampSEC.count() << "."
              << std::setw(4) << std::setfill('0') << timeStampMSC.count()
              << "] "
              << buf
              << std::endl;

    // Write to file
    file << "["
         << std::setw(2) << std::setfill('0') << timeStampHRS.count() << ":"
         << std::setw(2) << std::setfill('0') << timeStampMIN.count() << ":"
         << std::setw(2) << std::setfill('0') << timeStampSEC.count() << "."
         << std::setw(4) << std::setfill('0') << timeStampMSC.count()
         << "] "
         << buf
         << std::endl;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
