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

#include "available-commands.hpp"
#include "help.hpp"
#include "status.hpp"
#include "legacy-status.hpp"
#include "legacy-nfdc.hpp"

namespace nfd {
namespace tools {
namespace nfdc {

void
registerCommands(CommandParser& parser)
{
  registerHelpCommand(parser);
  registerStatusCommands(parser);
  registerLegacyStatusCommand(parser);

  struct LegacyNfdcCommandDefinition
  {
    std::string subcommand;
    std::string title;
  };
  const std::vector<LegacyNfdcCommandDefinition> legacyNfdcSubcommands{
    {"register", "register a prefix"},
    {"unregister", "unregister a prefix"},
    {"create", "create a face"},
    {"destroy", "destroy a face"},
    {"set-strategy", "set strategy choice on namespace"},
    {"unset-strategy", "unset strategy choice on namespace"},
    {"add-nexthop", "add FIB nexthop"},
    {"remove-nexthop", "remove FIB nexthop"}
  };
  for (const LegacyNfdcCommandDefinition& lncd : legacyNfdcSubcommands) {
    CommandDefinition def(lncd.subcommand, "");
    def.setTitle(lncd.title);
    def.addArg("args", ArgValueType::ANY, Required::NO, Positional::YES);
    parser.addCommand(def, &legacyNfdcMain);
  }
}

} // namespace nfdc
} // namespace tools
} // namespace nfd
