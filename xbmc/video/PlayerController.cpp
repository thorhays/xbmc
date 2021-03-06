/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "PlayerController.h"
#include "utils/StdString.h"
#include "settings/AdvancedSettings.h"
#include "settings/DisplaySettings.h"
#include "settings/MediaSettings.h"
#include "settings/Settings.h"
#include "cores/IPlayer.h"
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUISliderControl.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "video/dialogs/GUIDialogAudioSubtitleSettings.h"
#include "video/windows/GUIWindowFullScreen.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#include "Application.h"
#include "utils/LangCodeExpander.h"

CPlayerController::CPlayerController()
{
  m_sliderAction = 0;
}

CPlayerController::~CPlayerController()
{
}

bool CPlayerController::OnAction(const CAction &action)
{
  const unsigned int MsgTime = 300;
  const unsigned int DisplTime = 2000;

  switch (action.GetID())
  {
    case ACTION_SHOW_SUBTITLES:
    {
      if (g_application.m_pPlayer->GetSubtitleCount() == 0)
        return true;

      CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn = !CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn;
      g_application.m_pPlayer->SetSubtitleVisible(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn);
      CStdString sub, lang;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn)
      {
        SPlayerSubtitleStreamInfo info;
        g_application.m_pPlayer->GetSubtitleStreamInfo(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream, info);
        if (!g_LangCodeExpander.Lookup(lang, info.language))
          lang = g_localizeStrings.Get(13205); // Unknown

        if (info.name.length() == 0)
          sub = lang;
        else
          sub.Format("%s - %s", lang.c_str(), info.name.c_str());
      }
      else
        sub = g_localizeStrings.Get(1223);
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                            g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
      return true;
    }

    case ACTION_NEXT_SUBTITLE:
    {
      if (g_application.m_pPlayer->GetSubtitleCount() == 0)
        return true;

      if(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream < 0)
        CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream = g_application.m_pPlayer->GetSubtitle();

      if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn)
      {
        CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream++;
        if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream >= g_application.m_pPlayer->GetSubtitleCount())
        {
          CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream = 0;
          CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn = false;
          g_application.m_pPlayer->SetSubtitleVisible(false);
        }
        g_application.m_pPlayer->SetSubtitle(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream);
      }
      else
      {
        CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn = true;
        g_application.m_pPlayer->SetSubtitleVisible(true);
      }

      CStdString sub, lang;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleOn)
      {
        SPlayerSubtitleStreamInfo info;
        g_application.m_pPlayer->GetSubtitleStreamInfo(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleStream, info);
        if (!g_LangCodeExpander.Lookup(lang, info.language))
          lang = g_localizeStrings.Get(13205); // Unknown

        if (info.name.length() == 0)
          sub = lang;
        else
          sub.Format("%s - %s", lang.c_str(), info.name.c_str());
      }
      else
        sub = g_localizeStrings.Get(1223);
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(287), sub, DisplTime, false, MsgTime);
      return true;
    }

    case ACTION_SUBTITLE_DELAY_MIN:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay -= 0.1f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay < -g_advancedSettings.m_videoSubsDelayRange)
        CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay = -g_advancedSettings.m_videoSubsDelayRange;
      if (g_application.m_pPlayer)
        g_application.m_pPlayer->SetSubTitleDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay);

      ShowSlider(action.GetID(), 22006, CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay,
                                        -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                         g_advancedSettings.m_videoSubsDelayRange);
      return true;
    }

    case ACTION_SUBTITLE_DELAY_PLUS:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay += 0.1f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay > g_advancedSettings.m_videoSubsDelayRange)
        CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay = g_advancedSettings.m_videoSubsDelayRange;
      if (g_application.m_pPlayer)
        g_application.m_pPlayer->SetSubTitleDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay);

      ShowSlider(action.GetID(), 22006, CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay,
                                        -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                         g_advancedSettings.m_videoSubsDelayRange);
      return true;
    }

    case ACTION_SUBTITLE_DELAY:
    {
      ShowSlider(action.GetID(), 22006, CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay,
                                        -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                         g_advancedSettings.m_videoSubsDelayRange, true);
      return true;
    }

    case ACTION_AUDIO_DELAY:
    {
      ShowSlider(action.GetID(), 297, CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay,
                                      -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                       g_advancedSettings.m_videoAudioDelayRange, true);
      return true;
    }

    case ACTION_AUDIO_DELAY_MIN:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay -= 0.025f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay < -g_advancedSettings.m_videoAudioDelayRange)
        CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay = -g_advancedSettings.m_videoAudioDelayRange;
      if (g_application.m_pPlayer)
        g_application.m_pPlayer->SetAVDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay);

      ShowSlider(action.GetID(), 297, CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay,
                                      -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                       g_advancedSettings.m_videoAudioDelayRange);
      return true;
    }

    case ACTION_AUDIO_DELAY_PLUS:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay += 0.025f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay > g_advancedSettings.m_videoAudioDelayRange)
        CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay = g_advancedSettings.m_videoAudioDelayRange;
      if (g_application.m_pPlayer)
        g_application.m_pPlayer->SetAVDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay);

      ShowSlider(action.GetID(), 297, CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay,
                                      -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                       g_advancedSettings.m_videoAudioDelayRange);
      return true;
    }

    case ACTION_AUDIO_NEXT_LANGUAGE:
    {
      if (g_application.m_pPlayer->GetAudioStreamCount() == 1)
        return true;

      if(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream < 0)
        CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream = g_application.m_pPlayer->GetAudioStream();

      CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream++;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream >= g_application.m_pPlayer->GetAudioStreamCount())
        CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream = 0;
      g_application.m_pPlayer->SetAudioStream(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream);    // Set the audio stream to the one selected
      CStdString aud;
      CStdString lan;
      SPlayerAudioStreamInfo info;
      g_application.m_pPlayer->GetAudioStreamInfo(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioStream, info);
      if (!g_LangCodeExpander.Lookup(lan, info.language))
        lan = g_localizeStrings.Get(13205); // Unknown
      if (info.name.empty())
        aud = lan;
      else
        aud.Format("%s - %s", lan.c_str(), info.name.c_str());
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(460), aud, DisplTime, false, MsgTime);
      return true;
    }

    case ACTION_ZOOM_IN:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount += 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount > 2.f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount = 2.f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 216, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_ZOOM_OUT:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount -= 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount < 0.5f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount = 0.5f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 216, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount, 0.5f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_INCREASE_PAR:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio += 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio > 2.f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount = 2.f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 217, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_DECREASE_PAR:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio -= 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomZoomAmount < 0.5f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio = 0.5f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 217, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomPixelRatio, 0.5f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_VSHIFT_UP:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift -= 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift < -2.0f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift = -2.0f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 225, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_VSHIFT_DOWN:
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift += 0.01f;
      if (CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift > 2.0f)
        CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift = 2.0f;
      CMediaSettings::Get().GetCurrentVideoSettings().m_ViewMode = ViewModeCustom;
      g_renderManager.SetViewMode(ViewModeCustom);
      ShowSlider(action.GetID(), 225, CMediaSettings::Get().GetCurrentVideoSettings().m_CustomVerticalShift, -2.0f, 0.1f, 2.0f);
      return true;
    }

    case ACTION_SUBTITLE_VSHIFT_UP:
    {
      RESOLUTION_INFO& res_info =  CDisplaySettings::Get().GetResolutionInfo(g_graphicsContext.GetVideoResolution());
      int subalign = CSettings::Get().GetInt("subtitles.align");
      if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
      {
        res_info.iSubtitles ++;
        if (res_info.iSubtitles >= res_info.iHeight)
          res_info.iSubtitles = res_info.iHeight - 1;

        ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
      }
      else
      {
        res_info.iSubtitles --;
        if (res_info.iSubtitles < 0)
          res_info.iSubtitles = 0;

        if (subalign == SUBTITLE_ALIGN_MANUAL)
          ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        else
          ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
      }
      return true;
    }

    case ACTION_SUBTITLE_VSHIFT_DOWN:
    {
      RESOLUTION_INFO& res_info =  CDisplaySettings::Get().GetResolutionInfo(g_graphicsContext.GetVideoResolution());
      int subalign = CSettings::Get().GetInt("subtitles.align");
      if ((subalign == SUBTITLE_ALIGN_BOTTOM_OUTSIDE) || (subalign == SUBTITLE_ALIGN_TOP_INSIDE))
      {
        res_info.iSubtitles--;
        if (res_info.iSubtitles < 0)
          res_info.iSubtitles = 0;

        ShowSlider(action.GetID(), 274, (float) res_info.iHeight - res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
      }
      else
      {
        res_info.iSubtitles++;
        if (res_info.iSubtitles >= res_info.iHeight)
          res_info.iSubtitles = res_info.iHeight - 1;

        if (subalign == SUBTITLE_ALIGN_MANUAL)
          ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles, 0.0f, 1.0f, (float) res_info.iHeight);
        else
          ShowSlider(action.GetID(), 274, (float) res_info.iSubtitles - res_info.iHeight, (float) -res_info.iHeight, -1.0f, 0.0f);
      }
      return true;
    }

    case ACTION_SUBTITLE_ALIGN:
    {
      RESOLUTION_INFO& res_info =  CDisplaySettings::Get().GetResolutionInfo(g_graphicsContext.GetVideoResolution());
      int subalign = CSettings::Get().GetInt("subtitles.align");

      subalign++;
      if (subalign > SUBTITLE_ALIGN_TOP_OUTSIDE)
        subalign = SUBTITLE_ALIGN_MANUAL;

      res_info.iSubtitles = res_info.iHeight - 1;

      CSettings::Get().SetInt("subtitles.align", subalign);
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info,
                                            g_localizeStrings.Get(21460),
                                            g_localizeStrings.Get(21461 + subalign), 
                                            TOAST_DISPLAY_TIME, false);
      return true;
    }

    case ACTION_VOLAMP_UP:
    case ACTION_VOLAMP_DOWN:
    {
      float sliderMax = VOLUME_DRC_MAXIMUM / 100.0f;
      float sliderMin = VOLUME_DRC_MINIMUM / 100.0f;

      if (action.GetID() == ACTION_VOLAMP_UP)
        CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification += 1.0f;
      else
        CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification -= 1.0f;

      CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification =
        std::max(std::min(CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification, sliderMax), sliderMin);

      if (g_application.m_pPlayer)
        g_application.m_pPlayer->SetDynamicRangeCompression((long)(CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification * 100));

      ShowSlider(action.GetID(), 660, CMediaSettings::Get().GetCurrentVideoSettings().m_VolumeAmplification, sliderMin, 1.0f, sliderMax);
      return true;
    }

    default:
      break;
  }
  return false;
}

void CPlayerController::ShowSlider(int action, int label, float value, float min, float delta, float max, bool modal)
{
  m_sliderAction = action;
  if (modal)
    CGUIDialogSlider::ShowAndGetInput(g_localizeStrings.Get(label), value, min, delta, max, this);
  else
    CGUIDialogSlider::Display(label, value, min, delta, max, this);
}

void CPlayerController::OnSliderChange(void *data, CGUISliderControl *slider)
{
  if (!slider)
    return;

  if (m_sliderAction == ACTION_ZOOM_OUT || m_sliderAction == ACTION_ZOOM_IN ||
      m_sliderAction == ACTION_INCREASE_PAR || m_sliderAction == ACTION_DECREASE_PAR ||
      m_sliderAction == ACTION_VSHIFT_UP || m_sliderAction == ACTION_VSHIFT_DOWN ||
      m_sliderAction == ACTION_SUBTITLE_VSHIFT_UP || m_sliderAction == ACTION_SUBTITLE_VSHIFT_DOWN)
  {
    CStdString strValue;
    strValue.Format("%1.2f",slider->GetFloatValue());
    slider->SetTextValue(strValue);
  }
  else if (m_sliderAction == ACTION_VOLAMP_UP || m_sliderAction == ACTION_VOLAMP_DOWN)
    slider->SetTextValue(CGUIDialogAudioSubtitleSettings::FormatDecibel(slider->GetFloatValue(), 1.0f));
  else
    slider->SetTextValue(CGUIDialogAudioSubtitleSettings::FormatDelay(slider->GetFloatValue(), 0.025f));

  if (g_application.m_pPlayer)
  {
    if (m_sliderAction == ACTION_AUDIO_DELAY)
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetAVDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_AudioDelay);
    }
    else if (m_sliderAction == ACTION_SUBTITLE_DELAY)
    {
      CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetSubTitleDelay(CMediaSettings::Get().GetCurrentVideoSettings().m_SubtitleDelay);
    }
  }
}
