/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis
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
 **/

#include "network.hpp"

#include <boost/utility/value_init.hpp>
#include <algorithm>

namespace nfd {

Network::Network() = default;

Network::Network(const boost::asio::ip::address& minAddress,
                 const boost::asio::ip::address& maxAddress)
  : m_minAddress(minAddress)
  , m_maxAddress(maxAddress)
{
}

const Network&
Network::getMaxRangeV4()
{
  using boost::asio::ip::address_v4;
  static Network range{address_v4{}, address_v4{0xffffffff}};
  return range;
}

const Network&
Network::getMaxRangeV6()
{
  using boost::asio::ip::address_v6;
  static address_v6::bytes_type maxV6 = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
  static Network range{address_v6{}, address_v6{maxV6}};
  return range;
}

bool
Network::isValidCidr(const std::string& cidr)
{
  auto pos = cidr.find('/');
  if (pos == std::string::npos) {
    return false;
  }

  boost::system::error_code invalidIp;
  boost::asio::ip::address_v4::from_string(cidr.substr(0, pos), invalidIp);
  if (invalidIp) {
    return false;
  }

  auto prefixLenStr = cidr.substr(pos + 1);
  if (!std::all_of(prefixLenStr.begin(), prefixLenStr.end(), ::isdigit)) {
    return false;
  }
  int prefixLen = -1;
  try {
    prefixLen = boost::lexical_cast<int>(prefixLenStr);
  }
  catch (const boost::bad_lexical_cast&) {
    return false;
  }
  if (prefixLen < 0 || prefixLen > 32) {
    return false;
  }

  return true;
}

std::ostream&
operator<<(std::ostream& os, const Network& network)
{
  return os << network.m_minAddress << " <-> " << network.m_maxAddress;
}

std::istream&
operator>>(std::istream& is, Network& network)
{
  namespace ip = boost::asio::ip;

  std::string networkStr;
  is >> networkStr;

  size_t position = networkStr.find('/');
  if (position == std::string::npos) {
    network.m_minAddress = ip::address::from_string(networkStr);
    network.m_maxAddress = ip::address::from_string(networkStr);
  }
  else {
    ip::address address = ip::address::from_string(networkStr.substr(0, position));
    size_t mask = boost::lexical_cast<size_t>(networkStr.substr(position+1));

    if (address.is_v4()) {
      ip::address_v4::bytes_type maskBytes = boost::initialized_value;
      for (size_t i = 0; i < mask; i++) {
        size_t byteId = i / 8;
        size_t bitIndex = 7 - i % 8;
        maskBytes[byteId] |= (1 << bitIndex);
      }

      ip::address_v4::bytes_type addressBytes = address.to_v4().to_bytes();
      ip::address_v4::bytes_type min;
      ip::address_v4::bytes_type max;

      for (size_t i = 0; i < addressBytes.size(); i++) {
        min[i] = addressBytes[i] & maskBytes[i];
        max[i] = addressBytes[i] | ~(maskBytes[i]);
      }

      network.m_minAddress = ip::address_v4(min);
      network.m_maxAddress = ip::address_v4(max);
    }
    else {
      ip::address_v6::bytes_type maskBytes = boost::initialized_value;
      for (size_t i = 0; i < mask; i++) {
        size_t byteId = i / 8;
        size_t bitIndex = 7 - i % 8;
        maskBytes[byteId] |= (1 << bitIndex);
      }

      ip::address_v6::bytes_type addressBytes = address.to_v6().to_bytes();
      ip::address_v6::bytes_type min;
      ip::address_v6::bytes_type max;

      for (size_t i = 0; i < addressBytes.size(); i++) {
        min[i] = addressBytes[i] & maskBytes[i];
        max[i] = addressBytes[i] | ~(maskBytes[i]);
      }

      network.m_minAddress = ip::address_v6(min);
      network.m_maxAddress = ip::address_v6(max);
    }
  }

  return is;
}

} // namespace nfd
