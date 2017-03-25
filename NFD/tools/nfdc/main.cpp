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
#include "core/version.hpp"

namespace nfd {
namespace tools {
namespace nfdc {

static int
main(int argc, char** argv)
{
  std::vector<std::string> args(argv + 1, argv + argc);

  CommandParser parser;
  registerCommands(parser);

  if (args.empty() || args[0] == "-h") {
    helpList(std::cout, parser);
    return 0;
  }

  if (args[0] == "-V") {
    std::cout << NFD_VERSION_BUILD_STRING << std::endl;
    return 0;
  }

  std::string noun, verb;
  CommandArguments ca;
  ExecuteCommand execute;
  try {
    std::tie(noun, verb, ca, execute) = parser.parse(args, ParseMode::ONE_SHOT);
  }
  catch (const std::invalid_argument& e) {
    std::cerr << e.what() << std::endl;
    return 2;
  }

  try {
    ndn::Face face;
    ndn::KeyChain keyChain;
    ExecuteContext ctx{noun, verb, ca, face, keyChain};
    return execute(ctx);
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}

} // namespace nfdc
} // namespace tools
} // namespace nfd

int
main(int argc, char** argv)
{
  return nfd::tools::nfdc::main(argc, argv);
}
