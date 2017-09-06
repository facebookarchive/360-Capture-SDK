#pragma once

#ifdef __cplusplus
namespace libmp4operations
{
#endif // __cplusplus

typedef enum
{
  MP4_FILE_PERFECT_REPAIR_NOT_NEEDED = 0,
  MP4_FILE_DAMAGED_BEYOND_REPAIR,
  MP4_FILE_FLAVOR_REPAIR_NOT_SUPPORTED,
  MP4_FILE_REPAIRABLE_REPAIR_NEEDED,
} eMP4_REPAIR_SUMMARY_REPORT;

typedef struct
{
  uint64_t nMVHDDuration;
  uint64_t nMVHDTimescale;
  uint64_t nAudioTrakEDTSCumulativeDuration;
  uint64_t nVideoTrakEDTSCumulativeDuration;
} EDIT_LIST_DURATIONS_INFO;


#ifdef __cplusplus
}; // namespace libmp4operations
#endif // __cplusplus

