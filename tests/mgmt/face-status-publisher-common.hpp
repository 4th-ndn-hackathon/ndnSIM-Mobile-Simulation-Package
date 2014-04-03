/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Named Data Networking Project
 * See COPYING for copyright and distribution information.
 */

#ifndef NFD_TESTS_MGMT_FACE_STATUS_PUBLISHER_COMMON_HPP
#define NFD_TESTS_MGMT_FACE_STATUS_PUBLISHER_COMMON_HPP

#include "mgmt/face-status-publisher.hpp"
#include "mgmt/app-face.hpp"
#include "mgmt/internal-face.hpp"
#include "mgmt/face-flags.hpp"
#include "fw/forwarder.hpp"

#include "tests/test-common.hpp"
#include "tests/face/dummy-face.hpp"

#include <ndn-cpp-dev/management/nfd-face-status.hpp>

namespace nfd {
namespace tests {

class TestCountersFace : public DummyFace
{
public:

  TestCountersFace()
  {
  }

  virtual
  ~TestCountersFace()
  {
  }

  void
  setCounters(FaceCounter nInInterests,
              FaceCounter nInDatas,
              FaceCounter nOutInterests,
              FaceCounter nOutDatas)
  {
    FaceCounters& counters = getMutableCounters();
    counters.getNInInterests()  = nInInterests;
    counters.getNInDatas()      = nInDatas;
    counters.getNOutInterests() = nOutInterests;
    counters.getNOutDatas()     = nOutDatas;
  }


};

static inline uint64_t
readNonNegativeIntegerType(const Block& block,
                           uint32_t type)
{
  if (block.type() == type)
    {
      return readNonNegativeInteger(block);
    }
  std::stringstream error;
  error << "expected type " << type << " got " << block.type();
  throw ndn::Tlv::Error(error.str());
}

static inline uint64_t
checkedReadNonNegativeIntegerType(Block::element_const_iterator& i,
                                  Block::element_const_iterator end,
                                  uint32_t type)
{
  if (i != end)
    {
      const Block& block = *i;
      ++i;
      return readNonNegativeIntegerType(block, type);
    }
  throw ndn::Tlv::Error("Unexpected end of FaceStatus");
}

class FaceStatusPublisherFixture : public BaseFixture
{
public:

  FaceStatusPublisherFixture()
    : m_table(m_forwarder)
    , m_face(make_shared<InternalFace>())
    , m_publisher(m_table, m_face, "/localhost/nfd/FaceStatusPublisherFixture")
    , m_finished(false)
  {

  }

  virtual
  ~FaceStatusPublisherFixture()
  {

  }

  void
  add(shared_ptr<Face> face)
  {
    m_table.add(face);
  }

  void
  validateFaceStatus(const Block& statusBlock, const shared_ptr<Face>& reference)
  {
    ndn::nfd::FaceStatus status;
    BOOST_REQUIRE_NO_THROW(status.wireDecode(statusBlock));
    const FaceCounters& counters = reference->getCounters();

    BOOST_CHECK_EQUAL(status.getFaceId(), reference->getId());
    BOOST_CHECK_EQUAL(status.getRemoteUri(), reference->getRemoteUri().toString());
    BOOST_CHECK_EQUAL(status.getLocalUri(), reference->getLocalUri().toString());
    BOOST_CHECK_EQUAL(status.getFlags(), getFaceFlags(*reference));
    BOOST_CHECK_EQUAL(status.getNInInterests(), counters.getNInInterests());
    BOOST_CHECK_EQUAL(status.getNInDatas(), counters.getNInDatas());
    BOOST_CHECK_EQUAL(status.getNOutInterests(), counters.getNOutInterests());
    BOOST_CHECK_EQUAL(status.getNOutDatas(), counters.getNOutDatas());
  }

  void
  decodeFaceStatusBlock(const Data& data)
  {
    Block payload = data.getContent();

    m_buffer.appendByteArray(payload.value(), payload.value_size());

    BOOST_CHECK_NO_THROW(data.getName()[-1].toSegment());
    if (data.getFinalBlockId() != data.getName()[-1])
      {
        return;
      }

    // wrap the Face Statuses in a single Content TLV for easy parsing
    m_buffer.prependVarNumber(m_buffer.size());
    m_buffer.prependVarNumber(ndn::Tlv::Content);

    ndn::Block parser(m_buffer.buf(), m_buffer.size());
    parser.parse();

    BOOST_REQUIRE_EQUAL(parser.elements_size(), m_referenceFaces.size());

    std::list<shared_ptr<Face> >::const_iterator iReference = m_referenceFaces.begin();
    for (Block::element_const_iterator i = parser.elements_begin();
         i != parser.elements_end();
         ++i)
      {
        if (i->type() != ndn::tlv::nfd::FaceStatus)
          {
            BOOST_FAIL("expected face status, got type #" << i->type());
          }
        validateFaceStatus(*i, *iReference);
        ++iReference;
      }
    m_finished = true;
  }

protected:
  Forwarder m_forwarder;
  FaceTable m_table;
  shared_ptr<InternalFace> m_face;
  FaceStatusPublisher m_publisher;
  ndn::EncodingBuffer m_buffer;
  std::list<shared_ptr<Face> > m_referenceFaces;

protected:
  bool m_finished;
};

} // namespace tests
} // namespace nfd

#endif // NFD_TESTS_MGMT_FACE_STATUS_PUBLISHER_COMMON_HPP
