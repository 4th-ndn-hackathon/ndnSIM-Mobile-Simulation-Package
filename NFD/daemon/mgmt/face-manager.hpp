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

#ifndef NFD_DAEMON_MGMT_FACE_MANAGER_HPP
#define NFD_DAEMON_MGMT_FACE_MANAGER_HPP

#include "nfd-manager-base.hpp"
#include <ndn-cxx/mgmt/nfd/face-status.hpp>
#include <ndn-cxx/mgmt/nfd/face-query-filter.hpp>
#include <ndn-cxx/mgmt/nfd/face-event-notification.hpp>
#include "face/face.hpp"

namespace nfd {

class FaceTable;
class NetworkInterfaceInfo;
class ProtocolFactory;

/**
 * @brief implement the Face Management of NFD Management Protocol.
 * @sa http://redmine.named-data.net/projects/nfd/wiki/FaceMgmt
 */
class FaceManager : public NfdManagerBase
{
public:
  FaceManager(FaceTable& faceTable,
              Dispatcher& dispatcher,
              CommandAuthenticator& authenticator);

  /**
   * @brief Subscribe to face_system section for the config file
   */
  void
  setConfigFile(ConfigFile& configFile);

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // ControlCommand
  void
  createFace(const Name& topPrefix, const Interest& interest,
             const ControlParameters& parameters,
             const ndn::mgmt::CommandContinuation& done);

  void
  updateFace(const Name& topPrefix, const Interest& interest,
             const ControlParameters& parameters,
             const ndn::mgmt::CommandContinuation& done);

  void
  destroyFace(const Name& topPrefix, const Interest& interest,
              const ControlParameters& parameters,
              const ndn::mgmt::CommandContinuation& done);

  /**
   * \deprecated use Flags+Mask in faces/update instead
   */
  void
  enableLocalControl(const Name& topPrefix, const Interest& interest,
                     const ControlParameters& parameters,
                     const ndn::mgmt::CommandContinuation& done);

  /**
   * \deprecated use Flags+Mask in faces/update instead
   */
  void
  disableLocalControl(const Name& topPrefix, const Interest& interest,
                      const ControlParameters& parameters,
                      const ndn::mgmt::CommandContinuation& done);

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // helpers for ControlCommand
  void
  afterCreateFaceSuccess(const ControlParameters& parameters,
                         const shared_ptr<Face>& newFace,
                         const ndn::mgmt::CommandContinuation& done);

  void
  afterCreateFaceFailure(uint32_t status,
                         const std::string& reason,
                         const ndn::mgmt::CommandContinuation& done);

  Face*
  findFaceForLocalControl(const Interest& request,
                          const ControlParameters& parameters,
                          const ndn::mgmt::CommandContinuation& done);

  static void
  setLinkServiceOptions(Face& face,
                        const ControlParameters& parameters,
                        ControlParameters& response);

PUBLIC_WITH_TESTS_ELSE_PRIVATE: // StatusDataset
  void
  listFaces(const Name& topPrefix, const Interest& interest,
            ndn::mgmt::StatusDatasetContext& context);

  void
  listChannels(const Name& topPrefix, const Interest& interest,
               ndn::mgmt::StatusDatasetContext& context);

  void
  queryFaces(const Name& topPrefix, const Interest& interest,
             ndn::mgmt::StatusDatasetContext& context);

private: // helpers for StatusDataset handler
  bool
  matchFilter(const ndn::nfd::FaceQueryFilter& filter, const Face& face);

  /** \brief get status of face, including properties and counters
   */
  static ndn::nfd::FaceStatus
  collectFaceStatus(const Face& face, const time::steady_clock::TimePoint& now);

  /** \brief copy face properties into traits
   *  \tparam FaceTraits either FaceStatus or FaceEventNotification
   */
  template<typename FaceTraits>
  static void
  collectFaceProperties(const Face& face, FaceTraits& traits);

private: // NotificationStream
  void
  notifyFaceEvent(const Face& face, ndn::nfd::FaceEventKind kind);

  void
  connectFaceStateChangeSignal(const Face& face);

private: // configuration
  void
  processConfig(const ConfigSection& configSection, bool isDryRun,
                const std::string& filename);

  void
  processSectionUnix(const ConfigSection& configSection, bool isDryRun);

  void
  processSectionTcp(const ConfigSection& configSection, bool isDryRun);

  void
  processSectionUdp(const ConfigSection& configSection, bool isDryRun,
                    const std::vector<NetworkInterfaceInfo>& nicList);

  void
  processSectionEther(const ConfigSection& configSection, bool isDryRun,
                      const std::vector<NetworkInterfaceInfo>& nicList);

  void
  processSectionWebSocket(const ConfigSection& configSection, bool isDryRun);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::map<std::string /*protocol*/, shared_ptr<ProtocolFactory>> m_factories;
  FaceTable& m_faceTable;
  std::map<FaceId, signal::ScopedConnection> m_faceStateChangeConn;

private:
  ndn::mgmt::PostNotification m_postNotification;
  signal::ScopedConnection m_faceAddConn;
  signal::ScopedConnection m_faceRemoveConn;
};

} // namespace nfd

#endif // NFD_DAEMON_MGMT_FACE_MANAGER_HPP
