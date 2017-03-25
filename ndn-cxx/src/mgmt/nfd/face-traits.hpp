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

#ifndef NDN_MGMT_NFD_FACE_TRAITS_HPP
#define NDN_MGMT_NFD_FACE_TRAITS_HPP

#include "../../encoding/tlv-nfd.hpp"

namespace ndn {
namespace nfd {

/** \ingroup management
 *  \brief providers getters and setters of face information fields
 *  \tparam C the concrete class; it must provide .wireReset() method
            to clear wire encoding when a field changes
 */
template<class C>
class FaceTraits
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

  FaceTraits()
    : m_faceId(0)
    , m_faceScope(FACE_SCOPE_NON_LOCAL)
    , m_facePersistency(FACE_PERSISTENCY_PERSISTENT)
    , m_linkType(LINK_TYPE_POINT_TO_POINT)
    , m_flags(0x0)
  {
  }

  uint64_t
  getFaceId() const
  {
    return m_faceId;
  }

  C&
  setFaceId(uint64_t faceId)
  {
    wireReset();
    m_faceId = faceId;
    return static_cast<C&>(*this);
  }

  const std::string&
  getRemoteUri() const
  {
    return m_remoteUri;
  }

  C&
  setRemoteUri(const std::string& remoteUri)
  {
    wireReset();
    m_remoteUri = remoteUri;
    return static_cast<C&>(*this);
  }

  const std::string&
  getLocalUri() const
  {
    return m_localUri;
  }

  C&
  setLocalUri(const std::string& localUri)
  {
    wireReset();
    m_localUri = localUri;
    return static_cast<C&>(*this);
  }

  FaceScope
  getFaceScope() const
  {
    return m_faceScope;
  }

  C&
  setFaceScope(FaceScope faceScope)
  {
    wireReset();
    m_faceScope = faceScope;
    return static_cast<C&>(*this);
  }

  FacePersistency
  getFacePersistency() const
  {
    return m_facePersistency;
  }

  C&
  setFacePersistency(FacePersistency facePersistency)
  {
    wireReset();
    m_facePersistency = facePersistency;
    return static_cast<C&>(*this);
  }

  LinkType
  getLinkType() const
  {
    return m_linkType;
  }

  C&
  setLinkType(LinkType linkType)
  {
    wireReset();
    m_linkType = linkType;
    return static_cast<C&>(*this);
  }

  uint64_t
  getFlags() const
  {
    return m_flags;
  }

  C&
  setFlags(uint64_t flags)
  {
    wireReset();
    m_flags = flags;
    return static_cast<C&>(*this);
  }

  bool
  getFlagBit(size_t bit) const
  {
    if (bit >= 64) {
      BOOST_THROW_EXCEPTION(std::out_of_range("bit must be within range [0, 64)"));
    }

    return m_flags & (1 << bit);
  }

  C&
  setFlagBit(size_t bit, bool value)
  {
    if (bit >= 64) {
      BOOST_THROW_EXCEPTION(std::out_of_range("bit must be within range [0, 64)"));
    }

    wireReset();

    if (value) {
      m_flags |= (1 << bit);
    }
    else {
      m_flags &= ~(1 << bit);
    }

    return static_cast<C&>(*this);
  }

protected:
  virtual void
  wireReset() const = 0;

protected:
  uint64_t m_faceId;
  std::string m_remoteUri;
  std::string m_localUri;
  FaceScope m_faceScope;
  FacePersistency  m_facePersistency;
  LinkType m_linkType;
  uint64_t m_flags;
};

} // namespace nfd
} // namespace ndn

#endif // NDN_MGMT_NFD_FACE_TRAITS_HPP
