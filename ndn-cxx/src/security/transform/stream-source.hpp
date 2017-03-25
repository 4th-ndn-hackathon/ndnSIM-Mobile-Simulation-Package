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

#ifndef NDN_CXX_SECURITY_TRANSFORM_STREAM_SOURCE_HPP
#define NDN_CXX_SECURITY_TRANSFORM_STREAM_SOURCE_HPP

#include "transform-base.hpp"
#include <iostream>

namespace ndn {
namespace security {
namespace transform {

/**
 * @brief A source taking an std::istream as input
 */
class StreamSource : public Source
{
public:
  /**
   * @brief Construst a source using @p is as input.
   *
   * @param is The input stream
   * @param bufLen The internal buffer size. The default size is 1024.
   * @pre bufLen must be larger than 0.
   */
  explicit
  StreamSource(std::istream& is, size_t bufLen = DEFAULT_BUFFER_LEN);

private:
  /**
   * @brief Read bytes from the input stream until EOF is reached and write them into the next module.
   */
  virtual void
  doPump() final;

public:
  static const std::size_t DEFAULT_BUFFER_LEN;

private:
  std::istream& m_is;
  size_t m_bufferSize;
};

typedef StreamSource streamSource;

} // namespace transform
} // namespace security
} // namespace ndn

#endif // NDN_CXX_SECURITY_TRANSFORM_STREAM_SOURCE_HPP
