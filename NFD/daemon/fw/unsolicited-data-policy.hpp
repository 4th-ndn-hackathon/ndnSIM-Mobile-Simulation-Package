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

#ifndef NFD_DAEMON_FW_UNSOLICITED_DATA_POLICY_HPP
#define NFD_DAEMON_FW_UNSOLICITED_DATA_POLICY_HPP

#include "face/face.hpp"

namespace nfd {
namespace fw {

/** \brief a decision made by UnsolicitedDataPolicy
 */
enum class UnsolicitedDataDecision {
  DROP, ///< the Data should be dropped
  CACHE ///< the Data should be cached in the ContentStore
};

std::ostream&
operator<<(std::ostream& os, UnsolicitedDataDecision d);

/** \brief determines how to process an unsolicited Data
 *
 *  An incoming Data is unsolicited if it does not match any PIT entry.
 *  This class assists forwarding pipelines to decide whether to drop an unsolicited Data
 *  or admit it into the ContentStore.
 */
class UnsolicitedDataPolicy : noncopyable
{
public:
  virtual ~UnsolicitedDataPolicy() = default;

  virtual UnsolicitedDataDecision
  decide(const Face& inFace, const Data& data) const = 0;

public: // registry
  template<typename P>
  static void
  registerPolicy(const std::string& key)
  {
    Registry& registry = getRegistry();
    BOOST_ASSERT(registry.count(key) == 0);
    registry[key] = [] { return make_unique<P>(); };
  }

  /** \return an UnsolicitedDataPolicy identified by \p key, or nullptr if \p key is unknown
   */
  static unique_ptr<UnsolicitedDataPolicy>
  create(const std::string& key);

private:
  typedef std::function<unique_ptr<UnsolicitedDataPolicy>()> CreateFunc;
  typedef std::map<std::string, CreateFunc> Registry; // indexed by key

  static Registry&
  getRegistry();
};

/** \brief drops all unsolicited Data
 */
class DropAllUnsolicitedDataPolicy : public UnsolicitedDataPolicy
{
public:
  virtual UnsolicitedDataDecision
  decide(const Face& inFace, const Data& data) const final;
};

/** \brief admits unsolicited Data from local faces
 */
class AdmitLocalUnsolicitedDataPolicy : public UnsolicitedDataPolicy
{
public:
  virtual UnsolicitedDataDecision
  decide(const Face& inFace, const Data& data) const final;
};

/** \brief admits unsolicited Data from non-local faces
 */
class AdmitNetworkUnsolicitedDataPolicy : public UnsolicitedDataPolicy
{
public:
  virtual UnsolicitedDataDecision
  decide(const Face& inFace, const Data& data) const final;
};

/** \brief admits all unsolicited Data
 */
class AdmitAllUnsolicitedDataPolicy : public UnsolicitedDataPolicy
{
public:
  virtual UnsolicitedDataDecision
  decide(const Face& inFace, const Data& data) const final;
};

/** \brief the default UnsolicitedDataPolicy
 */
typedef DropAllUnsolicitedDataPolicy DefaultUnsolicitedDataPolicy;

} // namespace fw
} // namespace nfd

/** \brief registers an unsolicited data policy
 *  \param P a subclass of nfd::fw::UnsolicitedDataPolicy
 *  \param key the policy keyword, which is available for selection in NFD config file
 */
#define NFD_REGISTER_UNSOLICITED_DATA_POLICY(P, key)                \
static class NfdAuto ## P ## UnsolicitedDataPolicyRegistrationClass \
{                                                                   \
public:                                                             \
  NfdAuto ## P ## UnsolicitedDataPolicyRegistrationClass()          \
  {                                                                 \
    ::nfd::fw::UnsolicitedDataPolicy::registerPolicy<P>(key);       \
  }                                                                 \
} g_nfdAuto ## P ## UnsolicitedDataPolicyRegistrationVariable

#endif // NFD_DAEMON_FW_UNSOLICITED_DATA_POLICY_HPP
