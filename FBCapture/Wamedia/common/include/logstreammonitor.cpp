#include "loggingcallcenter.h"
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "commonDefinitions.h"
using namespace std;

namespace libwamediacommon
{

CLogStreamMonitor::CLogStreamMonitor(EXTERNAL_LOG_CALLBACK pLogCB,
                                     void* pLogCBToken,
                                     PROGRESS_INDICATION_CALLBACK pProgressCB,
                                     void* pProgressCBToken)
  : m_pLogCB(pLogCB)
  , m_pLogCBToken(pLogCBToken)
  , m_pProgressCB(pProgressCB)
  , m_pProgressCBToken(pProgressCBToken)
  , m_bRepeatedMessagingInProgress(false)
  , m_nRepeatedMessagesCount(0)
{

}

CLogStreamMonitor::~CLogStreamMonitor()
{
  resetRepeatedMessagesInfrastructure();
}

void
CLogStreamMonitor::forwardMessage(eLOG_LEVEL logLevel, const char* pStrMessage, uint32_t nMsgLength)
{
  if(isRepeatedMessagingInProgress())
    forwardOutOfRhythmMessage(logLevel, pStrMessage);
  else
  {
    if(m_pLogCB)
      m_pLogCB(logLevel, pStrMessage, nMsgLength, m_pLogCBToken);
  }
}

void
CLogStreamMonitor::forwardRepeatedMessage(eLOG_LEVEL logLevel, uint32_t nSourceFileLineNumber, const char* pStrMessage, uint32_t nMsgLength)
{
  UNUSED(nMsgLength);

  string strCheck = pStrMessage;
  if(0 == strCheck.find(REPEATED_MESSAGES_START))
  {
    resetRepeatedMessagesInfrastructure();
    m_bRepeatedMessagingInProgress = true;
  }
  else if(0 == strCheck.find(REPEATED_MESSAGES_END))
  {
    m_bRepeatedMessagingInProgress = false;
    printOutAccumulatedMessages();
    resetRepeatedMessagesInfrastructure();
  }
  else
  {
    // keep accumulating the info about the received repeated messages:
    m_mapOfRepeatedMessages[nSourceFileLineNumber].level      = logLevel;
    m_mapOfRepeatedMessages[nSourceFileLineNumber].strMessage = pStrMessage;
    m_mapOfRepeatedMessages[nSourceFileLineNumber].listOfMessageSerialNumbers.push_back(m_nRepeatedMessagesCount);
    m_nRepeatedMessagesCount++;
  }
}

void
CLogStreamMonitor::forwardOutOfRhythmMessage(eLOG_LEVEL logLevel, const char* strMessage)
{
  REPEATED_MESSAGE_INFO rmi;
  rmi.level      = logLevel;
  rmi.strMessage = strMessage;
  rmi.listOfMessageSerialNumbers.push_back(m_nRepeatedMessagesCount);
  m_listOfOutOfRhythmMessages.push_back(rmi);
}

void
CLogStreamMonitor::reportCompletionPercent(uint32_t nPercent)
{
  if(m_pProgressCB)
    m_pProgressCB(nPercent, m_pProgressCBToken);
}

void
CLogStreamMonitor::resetRepeatedMessagesInfrastructure()
{
  m_mapOfRepeatedMessages.clear();
  m_listOfOutOfRhythmMessages.clear();
  m_nRepeatedMessagesCount = 0;
  m_bRepeatedMessagingInProgress = false;
}

void
CLogStreamMonitor::printOutAccumulatedMessages()
{
  // time to examine the repeated messages map
  uint32_t nNumberOfDifferentRepeatedMessages = m_mapOfRepeatedMessages.size();
  if(0 == nNumberOfDifferentRepeatedMessages)
  {
    // Usually this happens when none of the repeated messages happen for whatever reason.
    // (such as code execution taking unexpected path). But, there may be some 'out-of-rhythm'
    // messages to be printed out.
    printOutOnlyOutOfRhythmMessages();
    return;
  }

  if(0 == m_listOfOutOfRhythmMessages.size())
    printOutUninterruptedMessageStream();
  else
    printOutPeriodMessageStreamWithOccassionalOutOfRhythmMessages();
}

void
CLogStreamMonitor::printOutOnlyOutOfRhythmMessages()
{
  if(0 != m_listOfOutOfRhythmMessages.size())
  {
    for(uint32_t i = 0; i < m_listOfOutOfRhythmMessages.size(); i++)
    {
      REPEATED_MESSAGE_INFO rmi = m_listOfOutOfRhythmMessages[i];
      forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    }
  }
}

void
CLogStreamMonitor::printOutUninterruptedMessageStream()
{
  vector<uint32_t> mapKeys;
  getMapKeys(mapKeys);
  uint32_t nMapKeys = mapKeys.size();
  if(0 == nMapKeys)
    return; // should never happen, but just in case to avoid dividing by zero

  uint32_t nNumberOfRepetitions = m_nRepeatedMessagesCount / nMapKeys;
  bool bPerfectRepetition = (0 == (m_nRepeatedMessagesCount % nMapKeys));
  printNumberOfUninterruptedMessagePeriods(mapKeys, nNumberOfRepetitions);
  if(false == bPerfectRepetition)
  {
    WARNING("Uninterrupted, but imperfectly periodic message queue (some messages may have been not printed out)");
  }
  mapKeys.clear();
}

void
CLogStreamMonitor::printOutPeriodMessageStreamWithOccassionalOutOfRhythmMessages()
{
  vector<uint32_t> mapKeys;
  getMapKeys(mapKeys);
  uint32_t nMapKeys = mapKeys.size();
  if(0 == nMapKeys)
    return; // should never happen, but just in case to avoid dividing by zero

  uint32_t nPeriodOfMessages = nMapKeys;
  uint32_t nNumberOfRepetitions = m_nRepeatedMessagesCount / nPeriodOfMessages;

  vector<uint32_t> listOfInterruptedPeriodsIndices;
  uint32_t nPreviousInterruptedPeriodIndex = INVALID_INDEX;
  uint32_t nInterruptedPeriodIndex         = INVALID_INDEX;
  uint32_t nLastInterruptedPeriodIndex     = INVALID_INDEX;
  for(uint32_t i = 0; i < m_listOfOutOfRhythmMessages.size(); i++)
  {
    uint32_t nTrueIndexOfInterruptiveMessage = 1 + m_listOfOutOfRhythmMessages[i].listOfMessageSerialNumbers[0];
    nInterruptedPeriodIndex = nTrueIndexOfInterruptiveMessage/nPeriodOfMessages;
    if(nPreviousInterruptedPeriodIndex != nInterruptedPeriodIndex)
    {
      listOfInterruptedPeriodsIndices.push_back(nInterruptedPeriodIndex);
      nPreviousInterruptedPeriodIndex = nInterruptedPeriodIndex;
      nLastInterruptedPeriodIndex     = nInterruptedPeriodIndex;
    }
  }

  if(0 == listOfInterruptedPeriodsIndices.size())
  {
    ERROR("Logging error: failed retrieving messages interrupting periodicity of other messages");
    return;
  }

  uint32_t nUninterruptedPeriods;
  if(0 != listOfInterruptedPeriodsIndices[0])
  {
    nUninterruptedPeriods = listOfInterruptedPeriodsIndices[0];
    printNumberOfUninterruptedMessagePeriods(mapKeys, nUninterruptedPeriods);
  }

  for(uint32_t i = 0; i < listOfInterruptedPeriodsIndices.size() - 1; i++)
  {
    printInterruptedPeriodOfMessages(mapKeys, listOfInterruptedPeriodsIndices[i]);
    nUninterruptedPeriods = listOfInterruptedPeriodsIndices[i + 1] - listOfInterruptedPeriodsIndices[i] - 1;
    printNumberOfUninterruptedMessagePeriods(mapKeys, nUninterruptedPeriods);
  }
  printInterruptedPeriodOfMessages(mapKeys, nLastInterruptedPeriodIndex);

  if((nNumberOfRepetitions - 1) >= nLastInterruptedPeriodIndex)
  {
    nUninterruptedPeriods = nNumberOfRepetitions - 1 - nLastInterruptedPeriodIndex;
    printNumberOfUninterruptedMessagePeriods(mapKeys, nUninterruptedPeriods);
  }
  listOfInterruptedPeriodsIndices.clear();
  mapKeys.clear();
}

void
CLogStreamMonitor::printNumberOfUninterruptedMessagePeriods(vector<uint32_t> & mapKeys, uint32_t nNumberOfRepetitions)
{
  if(1 < nNumberOfRepetitions)
    INFO("---- followed by log messages sequence repeated %d times: ----", nNumberOfRepetitions);

  for(uint32_t i = 0; i < mapKeys.size(); i++)
  {
    REPEATED_MESSAGE_INFO rmi = m_mapOfRepeatedMessages[mapKeys[i]];
    forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
  }
  if(1 < nNumberOfRepetitions)
    INFO("----------------------------------------------------------------");
}

void
CLogStreamMonitor::printInterruptedPeriodOfMessages(vector<uint32_t> & mapKeys, uint32_t nInterruptedPeriodIndex)
{
  uint32_t nMapKeys = mapKeys.size();
  uint32_t nStartMessageIndex = nInterruptedPeriodIndex*nMapKeys;
  uint32_t nLastMessageIndex  = nStartMessageIndex + nMapKeys - 1;
  if(nInterruptedPeriodIndex == nLastMessageIndex)
  {
    // Message in the middle which basically happens when we break out of the loop
    if(0 != m_listOfOutOfRhythmMessages.size())
    {
      REPEATED_MESSAGE_INFO rmi = m_listOfOutOfRhythmMessages[0];
      forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    }
    return;
  }
  INFO("---- and then (interrupted message repetition period %d): -------", nInterruptedPeriodIndex);

  vector<uint32_t> listOfOutOfRhythmMessagesInThisPeriod;
  uint32_t nIndexOfFirstPertinentOutOfRhythmMessage = INVALID_INDEX;
  for(uint32_t i = 0; i < m_listOfOutOfRhythmMessages.size(); i++)
  {
    uint32_t nInterruptingMessageIndex = m_listOfOutOfRhythmMessages[i].listOfMessageSerialNumbers[0];
    if((nStartMessageIndex <= nInterruptingMessageIndex) && (nLastMessageIndex >= nInterruptingMessageIndex))
    {
      listOfOutOfRhythmMessagesInThisPeriod.push_back(nInterruptingMessageIndex);
      if(INVALID_INDEX == nIndexOfFirstPertinentOutOfRhythmMessage)
        nIndexOfFirstPertinentOutOfRhythmMessage = i;
    }
  }

  if(0 == listOfOutOfRhythmMessagesInThisPeriod.size())
  {
    ERROR("Logging error: failed retrieving messages interrupting periodicity of other messages in period %d", nInterruptedPeriodIndex);
    return;
  }

  if(nStartMessageIndex != listOfOutOfRhythmMessagesInThisPeriod[0])
  {
    uint32_t nIntraPeriodStartIndex = 0;
    uint32_t nIntraPeriodEndIndex   = listOfOutOfRhythmMessagesInThisPeriod[0] - nStartMessageIndex;
    for(uint32_t i = nIntraPeriodStartIndex; i < nIntraPeriodEndIndex; i++)
    {
      REPEATED_MESSAGE_INFO rmi = m_mapOfRepeatedMessages[mapKeys[i]];
      forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    }
  }

  uint32_t nStretchesInBetween = listOfOutOfRhythmMessagesInThisPeriod.size() - 1;
  for(uint32_t i = 0; i < nStretchesInBetween; i++)
  {
    uint32_t nIndex = listOfOutOfRhythmMessagesInThisPeriod[i];
    REPEATED_MESSAGE_INFO rmi = m_listOfOutOfRhythmMessages[nIndexOfFirstPertinentOutOfRhythmMessage + i];
    forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    uint32_t nNextIndex = listOfOutOfRhythmMessagesInThisPeriod[i + 1];
    for(uint32_t r = nIndex; r < nNextIndex; r++)
    {
      rmi = m_mapOfRepeatedMessages[mapKeys[i - nIndex]];
      forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    }
  }

  if((nIndexOfFirstPertinentOutOfRhythmMessage + nStretchesInBetween) > (m_listOfOutOfRhythmMessages.size() - 1))
  {
    // should never happen; some defensive coding here
    ERROR("Logging error: list index exceeds list range (%d) > (%d)",
          nIndexOfFirstPertinentOutOfRhythmMessage + nStretchesInBetween, m_listOfOutOfRhythmMessages.size() - 1);
    return;
  }

  REPEATED_MESSAGE_INFO rmi = m_listOfOutOfRhythmMessages[nIndexOfFirstPertinentOutOfRhythmMessage + nStretchesInBetween];
  forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);

  uint32_t nLastListIndex = listOfOutOfRhythmMessagesInThisPeriod.size() - 1;
  uint32_t nIndexOfLastOutOfRhythmMessageInThisPeriod = listOfOutOfRhythmMessagesInThisPeriod[nLastListIndex];
  if(nIndexOfLastOutOfRhythmMessageInThisPeriod != nLastMessageIndex)
  {
    uint32_t nIntraPeriodStartIndex = nIndexOfLastOutOfRhythmMessageInThisPeriod - nStartMessageIndex;
    uint32_t nIntraPeriodLastIndex  = nLastMessageIndex - nStartMessageIndex;
    for(uint32_t i = nIntraPeriodStartIndex; i <= nIntraPeriodLastIndex; i++)
    {
      rmi = m_mapOfRepeatedMessages[mapKeys[i]];
      forwardMessage(rmi.level, rmi.strMessage.c_str(), rmi.strMessage.length() + 1);
    }
  }
  listOfOutOfRhythmMessagesInThisPeriod.clear();
  INFO("----------------------------------------------------------------");
}

void
CLogStreamMonitor::getMapKeys(vector<uint32_t> & mapKeys)
{
  //
  // This method provides the map keys which are normally the source file lines numbers of
  // the lines from which the log message came. Given the fact that the order in which the
  // functions reside in the source file may not be the order in which they are called, we
  // will sort the keys not based on the line nubmer, but based on the order of arrival.
  //
  map<uint32_t, REPEATED_MESSAGE_INFO>::iterator itMap;
  vector<MAP_KEY_INFO> listOfMapKeyInfos;
  for(itMap = m_mapOfRepeatedMessages.begin(); itMap != m_mapOfRepeatedMessages.end(); ++itMap)
  {
    MAP_KEY_INFO mki;
    memset(&mki, 0, sizeof(MAP_KEY_INFO));
    mki.nKey                 = itMap->first;
    if(0 != itMap->second.listOfMessageSerialNumbers.size())
      mki.nOrderOfAppearance = itMap->second.listOfMessageSerialNumbers[0];
    listOfMapKeyInfos.push_back(mki);
  }

  std::sort(listOfMapKeyInfos.begin(), listOfMapKeyInfos.end(), mapKeysComparisonFunction);
  for(uint32_t i = 0; i < listOfMapKeyInfos.size(); i++)
  {
    mapKeys.push_back(listOfMapKeyInfos[i].nKey);
  }
  listOfMapKeyInfos.clear();
}

}; // namespace libwamediacommon
