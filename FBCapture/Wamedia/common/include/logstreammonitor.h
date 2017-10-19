#pragma once
#include "logsrecipient.h"
#include "logssender.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

namespace libwamediacommon
{

class CLogStreamMonitor
{
public:
  CLogStreamMonitor(EXTERNAL_LOG_CALLBACK pLogCB,
                    void* pLogCBToken,
                    PROGRESS_INDICATION_CALLBACK pProgressCB,
                    void* pProgressCBToken);
  virtual ~CLogStreamMonitor();

  void forwardMessage(eLOG_LEVEL level, const char* strMessage, uint32_t nMsgLength);
  void forwardRepeatedMessage(eLOG_LEVEL level, uint32_t nSourceFileLineNumber, const char* pStrMessage, uint32_t nMsgLength);
  void forwardOutOfRhythmMessage(eLOG_LEVEL level, const char* strMessage);
  bool isRepeatedMessagingInProgress(void){ return m_bRepeatedMessagingInProgress;};
  void reportCompletionPercent(uint32_t nPercent);

private:
  EXTERNAL_LOG_CALLBACK        m_pLogCB;
  void*                        m_pLogCBToken;

  PROGRESS_INDICATION_CALLBACK m_pProgressCB;
  void*                        m_pProgressCBToken;

  // repeated messages infrastructure
  bool     m_bRepeatedMessagingInProgress;
  uint32_t m_nRepeatedMessagesCount;
  typedef struct
  {
    eLOG_LEVEL       level;
    string           strMessage;
    vector<uint32_t> listOfMessageSerialNumbers;
  } REPEATED_MESSAGE_INFO;
  map<uint32_t, REPEATED_MESSAGE_INFO> m_mapOfRepeatedMessages;
  vector<REPEATED_MESSAGE_INFO> m_listOfOutOfRhythmMessages;

  typedef struct
  {
    uint32_t nKey;
    uint32_t nOrderOfAppearance;
  } MAP_KEY_INFO;
  static bool mapKeysComparisonFunction(MAP_KEY_INFO first, MAP_KEY_INFO second){ return first.nOrderOfAppearance < second.nOrderOfAppearance;};

private:
  void resetRepeatedMessagesInfrastructure();
  void printOutAccumulatedMessages();
  void printOutOnlyOutOfRhythmMessages();
  void printOutUninterruptedMessageStream();
  void printOutPeriodMessageStreamWithOccassionalOutOfRhythmMessages();
  void printNumberOfUninterruptedMessagePeriods(vector<uint32_t> & mapKeys, uint32_t nNumberOfRepetitions);
  void printInterruptedPeriodOfMessages(vector<uint32_t> & mapKeys, uint32_t nInterruptedPeriodIndex);
  void getMapKeys(vector<uint32_t> & mapKeys);
};

}; // namespace libwamediacommon
