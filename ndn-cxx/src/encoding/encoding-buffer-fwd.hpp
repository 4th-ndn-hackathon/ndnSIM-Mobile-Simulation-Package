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

#ifndef NDN_ENCODING_ENCODING_BUFFER_FWD_HPP
#define NDN_ENCODING_ENCODING_BUFFER_FWD_HPP

namespace ndn {
namespace encoding {

typedef bool Tag;

/**
 * @brief Tag for EncodingImpl to indicate that Encoder is requested
 * Implementation of the tag may change to class. Use of true directly
 * as a template parameter is discouraged.
 */
static const Tag EncoderTag = true;

/**
 * @brief Tag for EncodingImpl to indicate that Estimator is requested
 * Implementation of the tag may change to class. Use of false directly
 * as a template parameter is discouraged.
 */
static const Tag EstimatorTag = false;

template<Tag TAG>
class EncodingImpl;

typedef EncodingImpl<EncoderTag> EncodingBuffer;
typedef EncodingImpl<EstimatorTag> EncodingEstimator;

} // namespace encoding

using encoding::EncodingImpl;
using encoding::EncodingBuffer;
using encoding::EncodingEstimator;

} // namespace ndn

#endif // NDN_ENCODING_ENCODING_BUFFER_FWD_HPP
