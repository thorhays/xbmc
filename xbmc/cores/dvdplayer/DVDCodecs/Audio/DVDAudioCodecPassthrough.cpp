/*
 *      Copyright (C) 2010-2013 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "DVDAudioCodecPassthrough.h"
#include "DVDCodecs/DVDCodecs.h"
#include "DVDStreamInfo.h"
#include "cores/AudioEngine/Utils/AEUtil.h"
#include "settings/Settings.h"
#include "utils/log.h"

#include "cores/AudioEngine/AEFactory.h"

CDVDAudioCodecPassthrough::CDVDAudioCodecPassthrough(void) :
  m_buffer    (NULL),
  m_bufferSize(0)
{
}

CDVDAudioCodecPassthrough::~CDVDAudioCodecPassthrough(void)
{
  Dispose();
}

bool CDVDAudioCodecPassthrough::Open(CDVDStreamInfo &hints, CDVDCodecOptions &options)
{
  /* dont open if AE doesnt support RAW */
  if (!CAEFactory::SupportsRaw())
    return false;

  bool bSupportsAC3Out    = false;
  bool bSupportsDTSOut    = false;
  bool bSupportsTrueHDOut = false;
  bool bSupportsDTSHDOut  = false;

  int audioMode = CSettings::Get().GetInt("audiooutput.mode");
  if (AUDIO_IS_BITSTREAM(audioMode))
  {
    bSupportsAC3Out = CSettings::Get().GetBool("audiooutput.ac3passthrough");
    bSupportsDTSOut = CSettings::Get().GetBool("audiooutput.dtspassthrough");
  }

  if (audioMode == AUDIO_HDMI)
  {
    bSupportsTrueHDOut = CSettings::Get().GetBool("audiooutput.truehdpassthrough");
    bSupportsDTSHDOut  = CSettings::Get().GetBool("audiooutput.dtshdpassthrough" ) && bSupportsDTSOut;
  }

  /* only get the dts core from the parser if we don't support dtsHD */
  m_info.SetCoreOnly(!bSupportsDTSHDOut);
  m_bufferSize = 0;

  if (
      (hints.codec == AV_CODEC_ID_AC3 && bSupportsAC3Out) ||
      (hints.codec == AV_CODEC_ID_DTS && bSupportsDTSOut) ||
      (audioMode == AUDIO_HDMI &&
        (
          (hints.codec == AV_CODEC_ID_EAC3   && bSupportsAC3Out   ) ||
          (hints.codec == AV_CODEC_ID_TRUEHD && bSupportsTrueHDOut)
        )
      )
  )
    return true;

  return false;
}

int CDVDAudioCodecPassthrough::GetSampleRate()
{
  return m_info.GetOutputRate();
}

int CDVDAudioCodecPassthrough::GetEncodedSampleRate()
{
  return m_info.GetSampleRate();
}

enum AEDataFormat CDVDAudioCodecPassthrough::GetDataFormat()
{
  switch(m_info.GetDataType())
  {
    case CAEStreamInfo::STREAM_TYPE_AC3:
      return AE_FMT_AC3;

    case CAEStreamInfo::STREAM_TYPE_DTS_512:
    case CAEStreamInfo::STREAM_TYPE_DTS_1024:
    case CAEStreamInfo::STREAM_TYPE_DTS_2048:
    case CAEStreamInfo::STREAM_TYPE_DTSHD_CORE:
      return AE_FMT_DTS;

    case CAEStreamInfo::STREAM_TYPE_EAC3:
      return AE_FMT_EAC3;

    case CAEStreamInfo::STREAM_TYPE_TRUEHD:
      return AE_FMT_TRUEHD;

    case CAEStreamInfo::STREAM_TYPE_DTSHD:
      return AE_FMT_DTSHD;

    default:
      return AE_FMT_INVALID; //Unknown stream type
  }
}

int CDVDAudioCodecPassthrough::GetChannels()
{
  return m_info.GetOutputChannels();
}

int CDVDAudioCodecPassthrough::GetEncodedChannels()
{
  return m_info.GetChannels();
}

CAEChannelInfo CDVDAudioCodecPassthrough::GetChannelMap()
{
  return m_info.GetChannelMap();
}

void CDVDAudioCodecPassthrough::Dispose()
{
  if (m_buffer)
  {
    delete[] m_buffer;
    m_buffer = NULL;
  }

  m_bufferSize = 0;
}

int CDVDAudioCodecPassthrough::Decode(uint8_t* pData, int iSize)
{
  if (iSize <= 0) return 0;

  unsigned int size = m_bufferSize;
  unsigned int used = m_info.AddData(pData, iSize, &m_buffer, &size);
  m_bufferSize = std::max(m_bufferSize, size);

  /* if we have a frame */
  if (size)
    m_packer.Pack(m_info, m_buffer, size);

  return used;
}

int CDVDAudioCodecPassthrough::GetData(uint8_t** dst)
{
  int size = m_packer.GetSize();
  *dst     = m_packer.GetBuffer();
  return size;
}

void CDVDAudioCodecPassthrough::Reset()
{
}

int CDVDAudioCodecPassthrough::GetBufferSize()
{
  return (int)m_info.GetBufferSize();
}
