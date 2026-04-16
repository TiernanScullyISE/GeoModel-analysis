#include <gtest/gtest.h>

#include "GeoModelXml/StrictErrorHandler.h"

#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace xercesc;

namespace {

class TestDOMLocator final : public DOMLocator {
public:
  TestDOMLocator(const XMLCh* uri, XMLFileLoc line, XMLFileLoc column)
    : m_uri(uri), m_line(line), m_column(column) {}

  XMLFileLoc getLineNumber() const override { return m_line; }
  XMLFileLoc getColumnNumber() const override { return m_column; }
  XMLFilePos getByteOffset() const override { return 0; }
  XMLFilePos getUtf16Offset() const override { return 0; }
  DOMNode* getRelatedNode() const override { return nullptr; }
  const XMLCh* getURI() const override { return m_uri; }

private:
  const XMLCh* m_uri{};
  XMLFileLoc m_line{};
  XMLFileLoc m_column{};
};

class TestDOMError final : public DOMError {
public:
  TestDOMError(ErrorSeverity severity,
               const XMLCh* message,
               DOMLocator* location)
    : m_severity(severity), m_message(message), m_location(location) {}

  ErrorSeverity getSeverity() const override { return m_severity; }
  const XMLCh* getMessage() const override { return m_message; }
  DOMLocator* getLocation() const override { return m_location; }
  void* getRelatedException() const override { return nullptr; }
  const XMLCh* getType() const override { return nullptr; }
  void* getRelatedData() const override { return nullptr; }

private:
  ErrorSeverity m_severity;
  const XMLCh* m_message{};
  DOMLocator* m_location{};
};

class XMLChGuard {
public:
  explicit XMLChGuard(const char* s) : m_ptr(XMLString::transcode(s)) {}
  ~XMLChGuard() { XMLString::release(&m_ptr); }

  XMLChGuard(const XMLChGuard&) = delete;
  XMLChGuard& operator=(const XMLChGuard&) = delete;

  const XMLCh* get() const { return m_ptr; }

private:
  XMLCh* m_ptr{};
};

class XercesEnvironment : public ::testing::Environment {
public:
  void SetUp() override { XMLPlatformUtils::Initialize(); }
  void TearDown() override { XMLPlatformUtils::Terminate(); }
};

}  // namespace

TEST(StrictErrorHandlerTest, HandleErrorReturnsTrueAndSetsErrorFlag) {
  XMLChGuard uri("test.xml");
  XMLChGuard message("bad element");

  TestDOMLocator locator(uri.get(), 12, 34);
  TestDOMError error(DOMError::DOM_SEVERITY_ERROR, message.get(), &locator);

  StrictErrorHandler handler;

  EXPECT_TRUE(handler.handleError(error));
  EXPECT_TRUE(handler.getSawErrors());   // or whatever your accessor is called
}

TEST(StrictErrorHandlerTest, HandleWarningAlsoSetsErrorFlag) {
  XMLChGuard uri("warn.xml");
  XMLChGuard message("minor issue");

  TestDOMLocator locator(uri.get(), 5, 9);
  TestDOMError error(DOMError::DOM_SEVERITY_WARNING, message.get(), &locator);

  StrictErrorHandler handler;

  EXPECT_TRUE(handler.handleError(error));
  EXPECT_TRUE(handler.getSawErrors());
}

TEST(StrictErrorHandlerTest, HandleFatalAlsoSetsErrorFlag) {
  XMLChGuard uri("fatal.xml");
  XMLChGuard message("catastrophic problem");

  TestDOMLocator locator(uri.get(), 99, 1);
  TestDOMError error(DOMError::DOM_SEVERITY_FATAL_ERROR, message.get(), &locator);

  StrictErrorHandler handler;

  EXPECT_TRUE(handler.handleError(error));
  EXPECT_TRUE(handler.getSawErrors());
}

TEST(StrictErrorHandlerDeathTest, NullLocationDies) {
  XMLChGuard message("bad element");
  TestDOMError error(DOMError::DOM_SEVERITY_ERROR, message.get(), nullptr);

  StrictErrorHandler handler;

  EXPECT_DEATH(
    {
      handler.handleError(error);
    },
    ""
  );
}

TEST(StrictErrorHandlerDeathTest, NullUriDies) {
  XMLChGuard message("bad element");
  TestDOMLocator locator(nullptr, 12, 34);
  TestDOMError error(DOMError::DOM_SEVERITY_ERROR, message.get(), &locator);

  StrictErrorHandler handler;
 //on a mac, this will die (test passes). 
  #if defined(__APPLE__)
  EXPECT_DEATH(
    {
      handler.handleError(error);
    },
    ""
  );
  #else
  //On ubuntu, it doesn't die but further output is truncated
  EXPECT_TRUE(handler.handleError(error));
  #endif
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new XercesEnvironment{});
  return RUN_ALL_TESTS();
}