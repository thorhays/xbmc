/*
 *      Copyright (C) 2013 Team XBMC
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

#include "System.h"
#include "jutils/jutils-details.hpp"

using namespace jni;

std::string CJNISystem::getProperty(const std::string &property)
{
  return jcast<std::string>(call_static_method<jhstring>("java/lang/System",
    "getProperty", "(Ljava/lang/String;)Ljava/lang/String;",
    jcast<jhstring>(property)));
}

std::string CJNISystem::getProperty(const std::string &property, const std::string &defaultValue)
{
  return jcast<std::string>(call_static_method<jhstring>("java/lang/System",
    "getProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
    jcast<jhstring>(property), jcast<jhstring>(defaultValue)));
}

std::string CJNISystem::setProperty(const std::string &property, const std::string &defaultValue)
{
  return jcast<std::string>(call_static_method<jhstring>("java/lang/System",
    "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
    jcast<jhstring>(property), jcast<jhstring>(defaultValue)));
}

std::string CJNISystem::clearProperty(const std::string &property)
{
  return jcast<std::string>(call_static_method<jhstring>("java/lang/System",
    "clearProperty", "(Ljava/lang/String;)Ljava/lang/String;",
    jcast<jhstring>(property)));
}
