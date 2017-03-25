/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2016 Regents of the University of California.
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

#ifndef NDN_SECURITY_SECURED_BAG_HPP
#define NDN_SECURITY_SECURED_BAG_HPP

#include "../common.hpp"
#include "v1/identity-certificate.hpp"

namespace ndn {
namespace security {

class SecuredBag
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

  SecuredBag();

  explicit
  SecuredBag(const Block& wire);

  SecuredBag(const v1::IdentityCertificate& cert,
             ConstBufferPtr key);

  virtual
  ~SecuredBag();

  void
  wireDecode(const Block& wire);

  const Block&
  wireEncode() const;

  const v1::IdentityCertificate&
  getCertificate() const
  {
    return m_cert;
  }

  ConstBufferPtr
  getKey() const
  {
    return m_key;
  }

private:
  v1::IdentityCertificate m_cert;
  ConstBufferPtr m_key;

  mutable Block m_wire;
};

} // namespace security

using security::SecuredBag;

} // namespace ndn

#endif // NDN_SECURITY_SECURED_BAG_HPP
