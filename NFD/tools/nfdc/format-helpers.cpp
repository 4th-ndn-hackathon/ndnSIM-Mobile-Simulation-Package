/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "format-helpers.hpp"

namespace nfd {
namespace tools {
namespace nfdc {

namespace xml {

void
printHeader(std::ostream& os)
{
  os << "<?xml version=\"1.0\"?>"
     << "<nfdStatus xmlns=\"ndn:/localhost/nfd/status/1\">";
}

void
printFooter(std::ostream& os)
{
  os << "</nfdStatus>";
}

std::ostream&
operator<<(std::ostream& os, const Text& text)
{
  for (char ch : text.s) {
    switch (ch) {
      case '"':
        os << "&quot;";
        break;
      case '&':
        os << "&amp;";
        break;
      case '\'':
        os << "&apos;";
        break;
      case '<':
        os << "&lt;";
        break;
      case '>':
        os << "&gt;";
        break;
      default:
        os << ch;
        break;
    }
  }
  return os;
}

std::string
formatSeconds(time::seconds d)
{
  return "PT" + to_string(d.count()) + "S";
}

std::string
formatTimestamp(time::system_clock::TimePoint t)
{
  return time::toString(t, "%Y-%m-%dT%H:%M:%S%F");
}

} // namespace xml

namespace text {

std::ostream&
operator<<(std::ostream& os, const Spaces& spaces)
{
  for (int i = 0; i < spaces.nSpaces; ++i) {
    os << ' ';
  }
  return os;
}

Separator::Separator(const std::string& first, const std::string& subsequent)
  : m_first(first)
  , m_subsequent(subsequent)
  , m_count(0)
{
}

Separator::Separator(const std::string& subsequent)
  : Separator("", subsequent)
{
}

std::ostream&
operator<<(std::ostream& os, Separator& sep)
{
  if (++sep.m_count == 1) {
    return os << sep.m_first;
  }
  return os << sep.m_subsequent;
}

std::string
formatSeconds(time::seconds d, bool isLong)
{
  return to_string(d.count()) + (isLong ? " seconds" : "s");
}

std::string
formatTimestamp(time::system_clock::TimePoint t)
{
  return time::toIsoString(t);
}

} // namespace text

} // namespace nfdc
} // namespace tools
} // namespace nfd
