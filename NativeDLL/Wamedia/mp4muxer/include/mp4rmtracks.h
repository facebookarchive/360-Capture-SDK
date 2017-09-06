#pragma once
#include "mp4errors.h"

#ifdef __cplusplus
namespace libmp4operations
{

extern "C"
{
#endif // __cplusplus

typedef void* HMP4RMTRACKS;

typedef struct
{
  uint32_t nTrackID;
  char*    pStrTrakDescription;
} TRAK_DETAILS;

HMP4RMTRACKS openMp4TrackRemover(void);
uint32_t reportTracksFoundInFile(HMP4RMTRACKS hMp4RmTracks,
                                 const char* pStrInputFilename,
                                 TRAK_DETAILS** ppTrackDetails,
                                 uint32_t* pnTracksCount);
uint32_t removeMp4Tracks(HMP4RMTRACKS hMp4RmTracks,
	                     const char* pStrInputFilename,
	                     const char* pStrOutputFilename,
	                     uint32_t* pTrackIndices,
	                     uint32_t nTracksToRemove);
uint32_t closeMp4TrackRemover(HMP4RMTRACKS hMp4RmTracks);
void deallocateTrakDetails(TRAK_DETAILS** ppTrackDetails, uint32_t nTracksCount);


// wrappers
uint32_t eliminateAllTracksMatchingDescription(const char* pStrInputFilename,
                                               const char* pStrOutputFilename,
                                               const char* pStrTrakDescription);
#ifdef __cplusplus
}

}; // namespace libmp4operations
#endif // __cplusplus

