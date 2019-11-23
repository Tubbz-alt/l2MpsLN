/**
 *-----------------------------------------------------------------------------
 * Title         : L2MPS - LCLS1 BSA
 * ----------------------------------------------------------------------------
 * File          : l2mps_l1bsa.cpp
 * Created       : 2019-11-22
 *-----------------------------------------------------------------------------
 * Description :
 *    LCLS1 BSA support for the  LCLS2 MPS Link Node
 *-----------------------------------------------------------------------------
 * This file is part of l2MpsLN. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of l2MpsLN, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/

#include "l2mps_l1bsa.h"

// Initialize static members
bool        L2MpsL1Bsa::enable_      = true;
bool        L2MpsL1Bsa::debug_       = false;
std::size_t L2MpsL1Bsa::strmCounter_ = 0;

L2MpsL1Bsa::L2MpsL1Bsa(const std::string& streamName, const std::string& recordPrefix)
:
    // This are the parameters names for each of the 24 data words in the data stream.
    // The order here should match the order of the data words in the stream, as defined
    // in the FW application.
    // The BSA PV name will be '${recordPrefix}:' followed by this parameter name.
    bsaChannelNames_ {
        "LC1_BSA_00",
        "LC1_BSA_01",
        "LC1_BSA_02",
        "LC1_BSA_03",
        "LC1_BSA_04",
        "LC1_BSA_05",
        "LC1_BSA_06",
        "LC1_BSA_07",
        "LC1_BSA_08",
        "LC1_BSA_09",
        "LC1_BSA_10",
        "LC1_BSA_11",
        "LC1_BSA_12",
        "LC1_BSA_13",
        "LC1_BSA_14",
        "LC1_BSA_15",
        "LC1_BSA_16",
        "LC1_BSA_17",
        "LC1_BSA_18",
        "LC1_BSA_19",
        "LC1_BSA_20",
        "LC1_BSA_21",
        "LC1_BSA_22",
        "LC1_BSA_23",
    },
    bsaChannels_(recordPrefix, bsaChannelNames_),
    strm_(IStream::create(cpswGetRoot()->findByName(streamName.c_str()))),
    run_(true),
    streamThread_( std::thread( &L2MpsL1Bsa::streamTask, this) )
{
    // Set the name for the 'streamThread_' thread
    if( pthread_setname_np( streamThread_.native_handle(), "L2MpsLcls1Bsa" ) )
        std::cerr <<  "pthread_setname_np failed for L2MpsL1Bsa thread" << std::endl;

    bsaChannels_.printChannelIds();
}

L2MpsL1Bsa::~L2MpsL1Bsa()
{
    // Stop the thread
    run_ = false;
    streamThread_.join();
}

void L2MpsL1Bsa::streamTask()
{
    uint8_t *buf = new uint8_t[sizeof(stream_data_t)];
    std::size_t got;

    //std::size_t count {0};
    for(;;)
    {
        // Check if we should break the loop
        if (!run_)
            break;

        // Read the data stream. This call is blocking with a 1s timeout
        got = strm_->read( buf, sizeof(stream_data_t), CTimeout(1000000) );

        // If BSA processing is disable, just discard the stream data
        // We still need to read the buffer to avoid back-pressuring the FW
        if ( !enable_ )
            continue;

        // Check if the stream read timed out
        if ( 0 == got )
            continue;

        // If we received data, check that we received the correct number of bytes
        if ( sizeof(stream_data_t) != got )
        {
            std::cerr << "ERROR: LCLS1 BSA Stream: bad size. Received " << got << ", instead of " << sizeof(stream_data_t) << " bytes" << std::endl;
            continue;
        }

        // Increment the stream counter
        ++strmCounter_;

        // Create a stream data pointer on the received data buffer
        stream_data_t* pSD{ (stream_data_t*)buf };

        // Temporal fix: the FW app is swapping the word
        // order of the timestamp, so let do a swap in SW
        // for now
        epicsTimeStamp etimeStamp;
        etimeStamp.secPastEpoch = pSD->timeStamp_H;
        etimeStamp.nsec = pSD->timeStamp_L;

        // Copy the LCLS1 BSA data from the stream into the BsaCore facility
        for (std::size_t i{0}; i < bsaChannelNames_.size(); ++i)
        {
            BSA_StoreData(
                bsaChannels_.at(i),
                etimeStamp, //pSD->timeStamp,
                static_cast<double>(pSD->data[i]),
                static_cast<BsaStat>(epicsAlarmNone),  // For now, we don't support alarm
                static_cast<BsaSevr>(epicsSevNone));   // nor severity.
        }

        // If debug mode is enabled, print out
        // 1/360 frames into the IOC shell
        if ( debug_ )
        {
            if ( 0 == (strmCounter_ % 360) )
            {
                std::cout << "Stream data:" << std::endl;
                std::cout << "===================" << std::endl;
                std::cout << std::hex;
                std::cout << "Header     = 0x" <<  pSD->header    << std::endl;
                std::cout << "Time stamp = 0x" <<  etimeStamp.secPastEpoch << ", " << etimeStamp.nsec << std::endl;
                for (std::size_t i{0}; i < 6; ++i)
                    std::cout << "DMOD       = 0x" <<  pSD->dmod[i] << std::endl;
                std::cout << "edefInit   = 0x" <<  pSD->edefInit  << std::endl;
                std::cout << "edefMajor  = 0x" <<  pSD->edefMajor << std::endl;
                std::cout << "edefMinor  = 0x" <<  pSD->edefMinor << std::endl;
                std::cout << "edefAvgDn  = 0x" <<  pSD->edefAvgDn << std::endl;
                std::cout << std::dec;
                for (std::size_t i{0}; i < 24; ++i)
                    std::cout << "DATA[" << std::setw(2) << i << "]   = " <<  pSD->data[i] << std::endl;
                std::cout << "===================" << std::endl;
                std::cout << "Stream received counter = " << strmCounter_ << std::endl;
                std::cout << std::endl;
            }
        }
    }
}

// L2MpsL1BsaConfig
extern "C" int L2MpsL1BsaConfig(const char* streamName, const char* recordPrefix)
{
    if ( (!streamName) || ('\0' == streamName[0]) )
    {
        fprintf( stderr, "Error: The name of the LCLS1 BSA stream is empty\n");
        return -1;
    }

    if ( (!recordPrefix) || ('\0' == recordPrefix[0]) )
    {
        fprintf( stderr, "Error: The record prefix for the LCLS1 BSA PVs is empty\n");
        return -1;
    }

    new L2MpsL1Bsa(streamName, recordPrefix);

    return 0;
}

static const iocshArg confArg0 = { "streamName",   iocshArgString };
static const iocshArg confArg1 = { "recordPrefix", iocshArgString };

static const iocshArg * const confArgs[] =
{
    &confArg0,
    &confArg1
};

static const iocshFuncDef configFuncDef = {"L2MpsL1BsaConfig", 2, confArgs};

static void configCallFunc(const iocshArgBuf *args)
{
    L2MpsL1BsaConfig(args[0].sval, args[1].sval);
}

// L2MpsL1BsaEnable
extern "C" int L2MpsL1BsaEnable(bool e)
{
    L2MpsL1Bsa::enable_ = e;

    return 0;
}

static const iocshArg setEnableArg0 = { "Flag (0=off, 1=on)",   iocshArgInt };

static const iocshArg * const setEnableArgs[] =
{
    &setEnableArg0,
};

static const iocshFuncDef setEnableFuncDef = {"L2MpsL1BsaEnable", 1, setEnableArgs};

static void setEnableCallFunc(const iocshArgBuf *args)
{
    L2MpsL1BsaEnable(args[0].ival);
}

// L2MpsL1BsaDebug
extern "C" int L2MpsL1BsaDebug(bool d)
{
    L2MpsL1Bsa::debug_ = d;

    return 0;
}

static const iocshArg setDebugArg0 = { "Flag (0=off, 1=on)",   iocshArgInt };

static const iocshArg * const setDebugArgs[] =
{
    &setDebugArg0,
};

static const iocshFuncDef setDebugFuncDef = {"L2MpsL1BsaDebug", 1, setDebugArgs};

static void setDebugCallFunc(const iocshArgBuf *args)
{
    L2MpsL1BsaDebug(args[0].ival);
}

// L2MpsL1BsaPrintCounter
extern "C" int L2MpsL1BsaPrintCounter()
{
    printf("Stream received counter = %zu\n", L2MpsL1Bsa::strmCounter_);

    return 0;
}

static const iocshFuncDef printCounterFuncDef = {"L2MpsL1BsaPrintCounter", 0};

static void printCounterCallFunc(const iocshArgBuf *args)
{
    L2MpsL1BsaPrintCounter();
}

// iocshRegister
void L2MpsL1BsaRegister(void)
{
    iocshRegister( &configFuncDef,        configCallFunc       );
    iocshRegister( &setEnableFuncDef,     setEnableCallFunc    );
    iocshRegister( &setDebugFuncDef,      setDebugCallFunc     );
    iocshRegister( &printCounterFuncDef,  printCounterCallFunc );
}

extern "C" {
    epicsExportRegistrar(L2MpsL1BsaRegister);
}
