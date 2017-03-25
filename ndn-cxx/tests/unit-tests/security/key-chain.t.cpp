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

#include "security/key-chain.hpp"
#include "security/validator.hpp"
#include "security/signing-helpers.hpp"

#include "boost-test.hpp"
#include "dummy-keychain.hpp"
#include "../util/test-home-environment-fixture.hpp"
#include "key-chain-fixture.hpp"
#include "identity-management-fixture.hpp"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace ndn {
namespace security {
namespace tests {

using namespace ndn::tests;

BOOST_FIXTURE_TEST_SUITE(SecurityKeyChain, util::TestHomeEnvironmentFixture)

template<class Path>
class TestHomeAndPibFixture : public TestHomeFixture<Path>
{
public:
  TestHomeAndPibFixture()
  {
    unsetenv("NDN_CLIENT_PIB");
    unsetenv("NDN_CLIENT_TPM");
  }
};

struct PibPathSqlite3File
{
  const std::string PATH = "build/keys-sqlite3-file/";
};

BOOST_FIXTURE_TEST_CASE(ConstructorNormalConfig, TestHomeAndPibFixture<PibPathSqlite3File>)
{
  createClientConf({"pib=pib-sqlite3:%PATH%", "tpm=tpm-file:%PATH%"});

  BOOST_REQUIRE_NO_THROW(KeyChain());

  KeyChain keyChain;
  BOOST_CHECK_EQUAL(keyChain.getPib().getPibLocator(), "pib-sqlite3:" + m_pibDir);
  BOOST_CHECK_EQUAL(keyChain.getPib().getTpmLocator(), "tpm-file:" + m_pibDir);
  BOOST_CHECK_EQUAL(keyChain.getTpm().getTpmLocator(), "tpm-file:" + m_pibDir);
}

struct PibPathSqlite3Empty
{
  const std::string PATH = "build/keys-sqlite3-empty/";
};

BOOST_FIXTURE_TEST_CASE(ConstructorEmptyConfig, TestHomeAndPibFixture<PibPathSqlite3Empty>)
{
  createClientConf({"pib=pib-sqlite3:%PATH%"});

#if defined(NDN_CXX_HAVE_OSX_SECURITY)
  std::string oldHOME;
  if (std::getenv("OLD_HOME"))
    oldHOME = std::getenv("OLD_HOME");

  std::string HOME;
  if (std::getenv("HOME"))
    HOME = std::getenv("HOME");

  if (!oldHOME.empty())
    setenv("HOME", oldHOME.c_str(), true);
  else
    unsetenv("HOME");
#endif

  BOOST_REQUIRE_NO_THROW(KeyChain());
  KeyChain keyChain;
  BOOST_CHECK_EQUAL(keyChain.getPib().getPibLocator(), "pib-sqlite3:" + m_pibDir);

#if defined(NDN_CXX_HAVE_OSX_SECURITY)
  BOOST_CHECK_EQUAL(keyChain.getPib().getTpmLocator(), "tpm-osxkeychain:");
  BOOST_CHECK_EQUAL(keyChain.getTpm().getTpmLocator(), "tpm-osxkeychain:");
#else
  BOOST_CHECK_EQUAL(keyChain.getPib().getTpmLocator(), "tpm-file:");
  BOOST_CHECK_EQUAL(keyChain.getTpm().getTpmLocator(), "tpm-file:");
#endif

#if defined(NDN_CXX_HAVE_OSX_SECURITY)
  if (!HOME.empty())
    setenv("HOME", HOME.c_str(), true);
  else
    unsetenv("HOME");
#endif
}

struct PibPathEmptyFile
{
  const std::string PATH = "build/keys-empty-file/";
};

BOOST_FIXTURE_TEST_CASE(ConstructorEmpty2Config, TestHomeAndPibFixture<PibPathEmptyFile>)
{
  createClientConf({"tpm=tpm-file:%PATH%"});

  BOOST_REQUIRE_NO_THROW(KeyChain());

  KeyChain keyChain;
  BOOST_CHECK_EQUAL(keyChain.getPib().getPibLocator(), "pib-sqlite3:");
  BOOST_CHECK_EQUAL(keyChain.getPib().getTpmLocator(), "tpm-file:" + m_pibDir);
  BOOST_CHECK_EQUAL(keyChain.getTpm().getTpmLocator(), "tpm-file:" + m_pibDir);
}

BOOST_FIXTURE_TEST_CASE(ConstructorMalConfig, TestHomeAndPibFixture<DefaultPibDir>)
{
  createClientConf({"pib=lord", "tpm=ring"});

  BOOST_REQUIRE_THROW(KeyChain(), KeyChain::Error); // Wrong configuration. Error expected.
}

BOOST_FIXTURE_TEST_CASE(ConstructorMal2Config, TestHomeAndPibFixture<DefaultPibDir>)
{
  createClientConf({"pib=pib-sqlite3:%PATH%", "tpm=just-wrong"});
  BOOST_REQUIRE_THROW(KeyChain(), KeyChain::Error); // Wrong configuration. Error expected.
}

BOOST_FIXTURE_TEST_CASE(ExportIdentity, IdentityManagementFixture)
{
  Name identity("/TestKeyChain/ExportIdentity/");
  identity.appendVersion();
  addIdentity(identity);

  shared_ptr<SecuredBag> exported = m_keyChain.exportIdentity(identity, "1234");

  Block block = exported->wireEncode();

  Name keyName = m_keyChain.getDefaultKeyNameForIdentity(identity);
  Name certName = m_keyChain.getDefaultCertificateNameForKey(keyName);

  m_keyChain.deleteIdentity(identity);

  BOOST_CHECK_EQUAL(m_keyChain.doesIdentityExist(identity), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesKeyExistInTpm(keyName, KeyClass::PRIVATE), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesKeyExistInTpm(keyName, KeyClass::PUBLIC), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName), false);

  SecuredBag imported;
  imported.wireDecode(block);
  m_keyChain.importIdentity(imported, "1234");

  BOOST_CHECK(m_keyChain.doesIdentityExist(identity));
  BOOST_CHECK(m_keyChain.doesPublicKeyExist(keyName));
  BOOST_CHECK(m_keyChain.doesKeyExistInTpm(keyName, KeyClass::PRIVATE));
  BOOST_CHECK(m_keyChain.doesKeyExistInTpm(keyName, KeyClass::PUBLIC));
  BOOST_CHECK(m_keyChain.doesCertificateExist(certName));
}

BOOST_FIXTURE_TEST_CASE(PrepareIdentityCertificate, IdentityManagementFixture)
{
  Name identity("/TestKeyChain/PrepareIdentityCertificate/");
  identity.appendVersion();
  addIdentity(identity);

  std::vector<v1::CertificateSubjectDescription> subjectDescription;
  Name lowerIdentity = identity;
  lowerIdentity.append("Lower").appendVersion();
  Name lowerKeyName = m_keyChain.generateRsaKeyPair(lowerIdentity, true);
  shared_ptr<v1::IdentityCertificate> idCert =
    m_keyChain.prepareUnsignedIdentityCertificate(lowerKeyName, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription);
  BOOST_CHECK(static_cast<bool>(idCert));
  BOOST_CHECK_EQUAL(idCert->getName().getPrefix(5),
                    Name().append(identity).append("KEY").append("Lower"));
  BOOST_CHECK(idCert->getFreshnessPeriod() >= time::milliseconds::zero());

  shared_ptr<v1::IdentityCertificate> idCert11 =
    m_keyChain.prepareUnsignedIdentityCertificate(lowerKeyName, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription,
                                                  lowerIdentity);
  BOOST_CHECK(static_cast<bool>(idCert11));
  BOOST_CHECK_EQUAL(idCert11->getName().getPrefix(6),
              Name().append(lowerIdentity).append("KEY"));

  Name anotherIdentity("/TestKeyChain/PrepareIdentityCertificate/Another/");
  anotherIdentity.appendVersion();
  Name anotherKeyName = m_keyChain.generateRsaKeyPair(anotherIdentity, true);
  shared_ptr<v1::IdentityCertificate> idCert2 =
    m_keyChain.prepareUnsignedIdentityCertificate(anotherKeyName, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription);
  BOOST_CHECK(static_cast<bool>(idCert2));
  BOOST_CHECK_EQUAL(idCert2->getName().getPrefix(5), Name().append(anotherIdentity).append("KEY"));


  Name wrongKeyName1;
  shared_ptr<v1::IdentityCertificate> idCert3 =
    m_keyChain.prepareUnsignedIdentityCertificate(wrongKeyName1, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription);
  BOOST_CHECK_EQUAL(static_cast<bool>(idCert3), false);


  Name wrongKeyName2("/TestKeyChain/PrepareIdentityCertificate");
  shared_ptr<v1::IdentityCertificate> idCert4 =
    m_keyChain.prepareUnsignedIdentityCertificate(wrongKeyName2, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription);
  BOOST_CHECK_EQUAL(static_cast<bool>(idCert4), false);


  Name wrongKeyName3("/TestKeyChain/PrepareIdentityCertificate/ksk-1234");
  shared_ptr<v1::IdentityCertificate> idCert5 =
    m_keyChain.prepareUnsignedIdentityCertificate(wrongKeyName3, identity,
                                                  time::system_clock::now(),
                                                  time::system_clock::now() + time::days(365),
                                                  subjectDescription);
  BOOST_CHECK_EQUAL(static_cast<bool>(idCert5), false);
}

BOOST_FIXTURE_TEST_CASE(Delete, IdentityManagementFixture)
{
  Name identity("/TestSecPublicInfoSqlite3/Delete");
  identity.appendVersion();

  Name certName1;
  BOOST_REQUIRE_NO_THROW(certName1 = m_keyChain.createIdentity(identity));

  Name keyName1 = v1::IdentityCertificate::certificateNameToPublicKeyName(certName1);
  Name keyName2;
  BOOST_REQUIRE_NO_THROW(keyName2 = m_keyChain.generateRsaKeyPairAsDefault(identity));

  shared_ptr<v1::IdentityCertificate> cert2;
  BOOST_REQUIRE_NO_THROW(cert2 = m_keyChain.selfSign(keyName2));
  Name certName2 = cert2->getName();
  BOOST_REQUIRE_NO_THROW(m_keyChain.addCertificateAsKeyDefault(*cert2));

  Name keyName3;
  BOOST_REQUIRE_NO_THROW(keyName3 = m_keyChain.generateRsaKeyPairAsDefault(identity));

  shared_ptr<v1::IdentityCertificate> cert3;
  BOOST_REQUIRE_NO_THROW(cert3 = m_keyChain.selfSign(keyName3));
  Name certName3 = cert3->getName();
  BOOST_REQUIRE_NO_THROW(m_keyChain.addCertificateAsKeyDefault(*cert3));
  shared_ptr<v1::IdentityCertificate> cert4;
  BOOST_REQUIRE_NO_THROW(cert4 = m_keyChain.selfSign(keyName3));
  Name certName4 = cert4->getName();
  BOOST_REQUIRE_NO_THROW(m_keyChain.addCertificateAsKeyDefault(*cert4));
  shared_ptr<v1::IdentityCertificate> cert5;
  BOOST_REQUIRE_NO_THROW(cert5 = m_keyChain.selfSign(keyName3));
  Name certName5 = cert5->getName();
  BOOST_REQUIRE_NO_THROW(m_keyChain.addCertificateAsKeyDefault(*cert5));

  BOOST_CHECK_EQUAL(m_keyChain.doesIdentityExist(identity), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName1), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName2), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName3), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName1), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName2), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName3), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName4), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName5), true);

  BOOST_REQUIRE_NO_THROW(m_keyChain.deleteCertificate(certName5));
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName5), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName3), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName4), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName3), true);

  BOOST_REQUIRE_NO_THROW(m_keyChain.deleteKey(keyName3));
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName4), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName3), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName3), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName2), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName1), true);
  BOOST_CHECK_EQUAL(m_keyChain.doesIdentityExist(identity), true);

  BOOST_REQUIRE_NO_THROW(m_keyChain.deleteIdentity(identity));
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName2), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName2), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesCertificateExist(certName1), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesPublicKeyExist(keyName1), false);
  BOOST_CHECK_EQUAL(m_keyChain.doesIdentityExist(identity), false);
}

BOOST_AUTO_TEST_CASE(KeyChainWithCustomTpmAndPib)
{
  BOOST_REQUIRE_NO_THROW((KeyChain("pib-dummy", "tpm-dummy")));
  BOOST_REQUIRE_NO_THROW((KeyChain("pib-dummy2", "tpm-dummy2")));
  BOOST_REQUIRE_NO_THROW((KeyChain("dummy", "dummy")));
  BOOST_REQUIRE_NO_THROW((KeyChain("dummy:", "dummy:")));
  BOOST_REQUIRE_NO_THROW((KeyChain("dummy:/something", "dummy:/something")));

  KeyChain keyChain("dummy", "dummy");
  BOOST_CHECK_EQUAL(keyChain.getPib().getPibLocator(), "pib-dummy:");
  BOOST_CHECK_EQUAL(keyChain.getPib().getTpmLocator(), "tpm-dummy:");
  BOOST_CHECK_EQUAL(keyChain.getTpm().getTpmLocator(), "tpm-dummy:");
  BOOST_CHECK_EQUAL(keyChain.getDefaultIdentity(), "/dummy/key");
}

BOOST_FIXTURE_TEST_CASE(GeneralSigningInterface, IdentityManagementFixture)
{
  Name id("/id");
  Name certName = m_keyChain.createIdentity(id);
  shared_ptr<v1::IdentityCertificate> idCert = m_keyChain.getCertificate(certName);
  Name keyName = idCert->getPublicKeyName();
  m_keyChain.setDefaultIdentity(id);

  Name id2("/id2");
  Name cert2Name = m_keyChain.createIdentity(id2);
  shared_ptr<v1::IdentityCertificate> id2Cert = m_keyChain.getCertificate(cert2Name);

  // SigningInfo is set to default
  Data data1("/data1");
  m_keyChain.sign(data1);
  BOOST_CHECK(Validator::verifySignature(data1, idCert->getPublicKeyInfo()));
  BOOST_CHECK_EQUAL(data1.getSignature().getKeyLocator().getName(), certName.getPrefix(-1));

  Interest interest1("/interest1");
  m_keyChain.sign(interest1);
  BOOST_CHECK(Validator::verifySignature(interest1, idCert->getPublicKeyInfo()));
  SignatureInfo sigInfo1(interest1.getName()[-2].blockFromValue());
  BOOST_CHECK_EQUAL(sigInfo1.getKeyLocator().getName(), certName.getPrefix(-1));

  // SigningInfo is set to Identity
  Data data2("/data2");
  m_keyChain.sign(data2, SigningInfo(SigningInfo::SIGNER_TYPE_ID, id2));
  BOOST_CHECK(Validator::verifySignature(data2, id2Cert->getPublicKeyInfo()));
  BOOST_CHECK_EQUAL(data2.getSignature().getKeyLocator().getName(), cert2Name.getPrefix(-1));

  Interest interest2("/interest2");
  m_keyChain.sign(interest2, SigningInfo(SigningInfo::SIGNER_TYPE_ID, id2));
  BOOST_CHECK(Validator::verifySignature(interest2, id2Cert->getPublicKeyInfo()));
  SignatureInfo sigInfo2(interest2.getName()[-2].blockFromValue());
  BOOST_CHECK_EQUAL(sigInfo2.getKeyLocator().getName(), cert2Name.getPrefix(-1));

  // SigningInfo is set to Key
  Data data3("/data3");
  m_keyChain.sign(data3, SigningInfo(SigningInfo::SIGNER_TYPE_KEY, keyName));
  BOOST_CHECK(Validator::verifySignature(data3, idCert->getPublicKeyInfo()));
  BOOST_CHECK_EQUAL(data3.getSignature().getKeyLocator().getName(), certName.getPrefix(-1));

  Interest interest3("/interest3");
  m_keyChain.sign(interest3);
  BOOST_CHECK(Validator::verifySignature(interest3, idCert->getPublicKeyInfo()));
  SignatureInfo sigInfo3(interest1.getName()[-2].blockFromValue());
  BOOST_CHECK_EQUAL(sigInfo3.getKeyLocator().getName(), certName.getPrefix(-1));

  // SigningInfo is set to Cert
  Data data4("/data4");
  m_keyChain.sign(data4, SigningInfo(SigningInfo::SIGNER_TYPE_CERT, certName));
  BOOST_CHECK(Validator::verifySignature(data4, idCert->getPublicKeyInfo()));
  BOOST_CHECK_EQUAL(data4.getSignature().getKeyLocator().getName(), certName.getPrefix(-1));

  Interest interest4("/interest4");
  m_keyChain.sign(interest4, SigningInfo(SigningInfo::SIGNER_TYPE_CERT, certName));
  BOOST_CHECK(Validator::verifySignature(interest4, idCert->getPublicKeyInfo()));
  SignatureInfo sigInfo4(interest4.getName()[-2].blockFromValue());
  BOOST_CHECK_EQUAL(sigInfo4.getKeyLocator().getName(), certName.getPrefix(-1));


  // SigningInfo is set to DigestSha256
  Data data5("/data5");
  m_keyChain.sign(data5, SigningInfo(SigningInfo::SIGNER_TYPE_SHA256));
  BOOST_CHECK(Validator::verifySignature(data5, DigestSha256(data5.getSignature())));

  Interest interest5("/interest4");
  m_keyChain.sign(interest5, SigningInfo(SigningInfo::SIGNER_TYPE_SHA256));
  BOOST_CHECK(Validator::verifySignature(interest5,
                                         DigestSha256(Signature(interest5.getName()[-2].blockFromValue(),
                                                                interest5.getName()[-1].blockFromValue()))));
}

BOOST_FIXTURE_TEST_CASE(EcdsaSigningByIdentityNoCert, IdentityManagementFixture)
{
  Data data("/test/data");

  Name nonExistingIdentity = Name("/non-existing/identity").appendVersion();

  BOOST_CHECK_NO_THROW(m_keyChain.sign(data, signingByIdentity(nonExistingIdentity)));
  BOOST_CHECK_EQUAL(data.getSignature().getType(),
                    KeyChain::getSignatureType(KeyChain::DEFAULT_KEY_PARAMS.getKeyType(),
                                               DigestAlgorithm::SHA256));
  BOOST_CHECK(nonExistingIdentity.isPrefixOf(data.getSignature().getKeyLocator().getName()));

  Name ecdsaIdentity = Name("/ndn/test/ecdsa").appendVersion();
  Name ecdsaKeyName = m_keyChain.generateEcdsaKeyPairAsDefault(ecdsaIdentity, false, 256);
  BOOST_CHECK_NO_THROW(m_keyChain.sign(data, signingByIdentity(ecdsaIdentity)));
  BOOST_CHECK_EQUAL(data.getSignature().getType(),
                    KeyChain::getSignatureType(EcdsaKeyParams().getKeyType(), DigestAlgorithm::SHA256));
  BOOST_CHECK(ecdsaIdentity.isPrefixOf(data.getSignature().getKeyLocator().getName()));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace security
} // namespace ndn
