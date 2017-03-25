/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef NDN_MGMT_CONTROL_RESPONSE_HPP
#define NDN_MGMT_CONTROL_RESPONSE_HPP

#include "../encoding/block.hpp"

namespace ndn {
namespace mgmt {

/** \brief ControlCommand response
 */
class ControlResponse
{
public:
  class Error : public tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : tlv::Error(what)
    {
    }
  };

  ControlResponse();

  ControlResponse(uint32_t code, const std::string& text);

  explicit
  ControlResponse(const Block& block);

  uint32_t
  getCode() const;

  ControlResponse&
  setCode(uint32_t code);

  const std::string&
  getText() const;

  ControlResponse&
  setText(const std::string& text);

  const Block&
  getBody() const;

  ControlResponse&
  setBody(const Block& body);

  const Block&
  wireEncode() const;

  void
  wireDecode(const Block& block);

protected:
  uint32_t m_code;
  std::string m_text;
  Block m_body;

  mutable Block m_wire;
};

inline uint32_t
ControlResponse::getCode() const
{
  return m_code;
}

inline ControlResponse&
ControlResponse::setCode(uint32_t code)
{
  m_code = code;
  m_wire.reset();
  return *this;
}

inline const std::string&
ControlResponse::getText() const
{
  return m_text;
}

inline ControlResponse&
ControlResponse::setText(const std::string& text)
{
  m_text = text;
  m_wire.reset();
  return *this;
}

inline const Block&
ControlResponse::getBody() const
{
  return m_body;
}

inline ControlResponse&
ControlResponse::setBody(const Block& body)
{
  m_body = body;
  m_body.encode(); // will do nothing if already encoded
  m_wire.reset();
  return *this;
}

std::ostream&
operator<<(std::ostream& os, const ControlResponse& response);

} // namespace mgmt
} // namespace ndn

#endif // NDN_MGMT_CONTRO_RESPONSE_HPP
