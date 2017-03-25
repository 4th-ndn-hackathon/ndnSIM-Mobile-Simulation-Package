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

#ifndef NDN_SECURITY_CERTIFICATE_CONTAINER_HPP
#define NDN_SECURITY_CERTIFICATE_CONTAINER_HPP

#include <set>
#include "v1/identity-certificate.hpp"

namespace ndn {
namespace security {

class PibImpl;

/// @brief A handler to search or enumerate certificates of a key.
class CertificateContainer
{
public:
  class const_iterator
  {
  public:
    friend class CertificateContainer;

  public:
    v1::IdentityCertificate
    operator*();

    const_iterator&
    operator++();

    const_iterator
    operator++(int);

    bool
    operator==(const const_iterator& other);

    bool
    operator!=(const const_iterator& other);

  private:
    const_iterator(std::set<Name>::const_iterator it, shared_ptr<PibImpl> impl);

  private:
    std::set<Name>::const_iterator m_it;
    shared_ptr<PibImpl> m_impl;
  };

  typedef const_iterator iterator;

public:
  CertificateContainer();

  CertificateContainer(std::set<Name>&& certNames, shared_ptr<PibImpl> impl);

  const_iterator
  begin() const;

  const_iterator
  end() const;

  const_iterator
  find(const Name& certName) const;

  size_t
  size() const;

private:
  std::set<Name> m_certNames;
  shared_ptr<PibImpl> m_impl;
};

} // namespace security
} // namespace ndn

#endif // NDN_SECURITY_CERTIFICATE_CONTAINER_HPP
