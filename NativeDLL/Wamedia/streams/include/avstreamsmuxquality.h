#pragma once

#ifdef __cplusplus
namespace libwamediastreams
{
extern "C"
{
#endif // __cplusplus

typedef enum
{
  MUX_QUALITY_UNKNOWN,
  MUX_QUALITY_EXCELLENT,
  MUX_QUALITY_SATISFACTORY,
  MUX_QUALITY_BORDERLINE,
  MUX_QUALITY_POOR_GENERALLY_IRREGULAR_FILE,
  MUX_QUALITY_POOR_ONLY_ONE_SEEK_POINT,
  MUX_QUALITY_POOR_CHUNK_DURATION_TOO_LONG,
  MUX_QUALITY_POOR_AV_CHUNKS_INTERLEAVING
} eMUX_QUALITY;

typedef struct
{
  eMUX_QUALITY muxQuality;
  float fMaxAudioVideoInterChunkTimestampDifference;
  float fAvgAudioVideoInterChunkTimestampDifference;
  float fMaxVideoChunkDuration;
  float fAverageVideoChunkDuration;
  float fMaxAudioChunkDuration;
  float fAverageAudioChunkDuration;
} MUX_QUALITY_REPORT;

void printMuxQualityReports(MUX_QUALITY_REPORT mqr, bool bDetailed);

#ifdef __cplusplus
}

}; // namespace libwamediastreams
#endif // __cplusplus

