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
 *
 * @author Eric Newberry <enewberry@email.arizona.edu>
 */

#ifndef NDN_CXX_LP_GEO_TAG_HPP
#define NDN_CXX_LP_GEO_TAG_HPP

#include "../common.hpp"
#include "../tag.hpp"
#include "../encoding/encoding-buffer.hpp"
#include "../encoding/block-helpers.hpp"

#include "tlv.hpp"

namespace ndn {
namespace lp {

/**
 * \brief represents a CachePolicy header field
 */
class GeoTag
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : ndn::tlv::Error(what)
    {
    }
  };

  GeoTag();

  explicit
  GeoTag(const Block& block);

  /**
   * \brief prepend GeoTag to encoder
   */
  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  /**
   * \brief encode GeoTag into wire format
   */
  const Block&
  wireEncode() const;

  /**
   * \brief get GeoTag from wire format
   */
  void
  wireDecode(const Block& wire);

public: // get & set GeoTag
  /**
   * \return position x of GeoTag
   */
  double
  getPosX() const;

  /**
   * \return position y of GeoTag
   */
  double
  getPosY() const;

  /**
   * \brief set position x
   */
  void
  setPosX(double pos_x);

  /**
   * \brief set position y
   */
  void
  setPosY(double pos_y);

private:
  double m_pos_x;
  double m_pos_y;
  mutable Block m_wire;
};

} // namespace lp
} // namespace ndn

#endif // NDN_CXX_LP_GEOTAG_HPP
