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

#include "geo-tag.hpp"

namespace ndn {
namespace lp {

GeoTag::GeoTag()
  : m_pos_x(0.0)
  , m_pos_y(0.0)
{
}

GeoTag::GeoTag(const Block& block)
{
  wireDecode(block);
}

template<encoding::Tag TAG>
size_t
GeoTag::wireEncode(EncodingImpl<TAG>& encoder) const
{
  if (m_pos_x == 0.0 && m_pos_y == 0.0) {
    BOOST_THROW_EXCEPTION(Error("Positions must be set"));
  }
  size_t length = 0;
  // encode position X
  length += prependNonNegativeIntegerBlock(encoder, tlv::GeoTagPos,
                                           static_cast<uint32_t>(m_pos_x * 10));
  // encode position Y
  length += prependNonNegativeIntegerBlock(encoder, tlv::GeoTagPos,
                                           static_cast<uint32_t>(m_pos_y * 10));
  length += encoder.prependVarNumber(length);
  length += encoder.prependVarNumber(tlv::GeoTag);
  return length;
}

template size_t
GeoTag::wireEncode<encoding::EncoderTag>(EncodingImpl<encoding::EncoderTag>& encoder) const;

template size_t
GeoTag::wireEncode<encoding::EstimatorTag>(EncodingImpl<encoding::EstimatorTag>& encoder) const;

const Block&
GeoTag::wireEncode() const
{
  if (m_pos_x == 0.0 && m_pos_y == 0.0) {
    BOOST_THROW_EXCEPTION(Error("Positions must be set"));
  }

  if (m_wire.hasWire()) {
    return m_wire;
  }

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
GeoTag::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::GeoTag) {
    BOOST_THROW_EXCEPTION(Error("expecting GeoTag block"));
  }

  m_wire = wire;
  m_wire.parse();

  Block::element_const_iterator it = m_wire.elements_begin();
  // decode position y first
  if (it != m_wire.elements_end() && it->type() == tlv::GeoTagPos) {
    m_pos_y = static_cast<uint32_t>(readNonNegativeInteger(*it)) / 10.0;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("expecting CachePolicyType block for position Y"));
  }
  // move and decode position x then
  it++;
  if (it != m_wire.elements_end() && it->type() == tlv::GeoTagPos) {
    m_pos_x = static_cast<uint32_t>(readNonNegativeInteger(*it)) / 10.0;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("expecting CachePolicyType block for position X"));
  }
  // if both positions are 0
  if (m_pos_x == 0.0 && m_pos_y == 0.0) {
    BOOST_THROW_EXCEPTION(Error("positions must be set"));
  }
}

double
GeoTag::getPosX() const
{
  return m_pos_x;
}

double
GeoTag::getPosY() const
{
  return m_pos_y;
}

void
GeoTag::setPosX(double pos_x)
{
  m_pos_x = pos_x;
}

void
GeoTag::setPosY(double pos_y)
{
  m_pos_y = pos_y;
}

} // namespace lp
} // namespace ndn
