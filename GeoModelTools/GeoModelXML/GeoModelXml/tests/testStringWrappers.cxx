/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/StringWrappers.h"

#include <gtest/gtest.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace xercesc;

namespace {

class XMLChHolder {
public:
  explicit XMLChHolder(const char* s)
    : m_ptr(XMLString::transcode(s)) {}

  ~XMLChHolder() {
    XMLString::release(&m_ptr);
  }

  XMLChHolder(const XMLChHolder&) = delete;
  XMLChHolder& operator=(const XMLChHolder&) = delete;

  const XMLCh* get() const noexcept { return m_ptr; }

private:
  XMLCh* m_ptr{};
};

class XercesEnvironment final : public ::testing::Environment {
public:
  void SetUp() override { XMLPlatformUtils::Initialize(); }
  void TearDown() override { XMLPlatformUtils::Terminate(); }
};

struct DOMDocumentDeleter {
  void operator()(DOMDocument* doc) const {
    if (doc) {
      doc->release();
    }
  }
};

using DOMDocumentPtr = std::unique_ptr<DOMDocument, DOMDocumentDeleter>;

DOMDocumentPtr makeDocument(const char* rootName = "root") {
  DOMImplementation* impl =
      DOMImplementationRegistry::getDOMImplementation(XMLChHolder("Core").get());
  if (!impl) {
    return {};
  }

  DOMDocument* doc =
      impl->createDocument(nullptr, XMLChHolder(rootName).get(), nullptr);
  return DOMDocumentPtr{doc};
}

void setAttribute(DOMElement& element, const char* name, const char* value) {
  element.setAttribute(XMLChHolder(name).get(), XMLChHolder(value).get());
}

DOMElement* appendChildElement(DOMDocument& doc,
                               DOMElement& parent,
                               const char* tagName) {
  DOMElement* child = doc.createElement(XMLChHolder(tagName).get());
  parent.appendChild(child);
  return child;
}

}  // namespace

TEST(StringWrappers, xml2Str_nullptr) {
  EXPECT_EQ(GeoXML::xml2Str(nullptr), "");
}

TEST(StringWrappers, xml2Str_basic) {
  XMLChHolder text("hello");
  EXPECT_EQ(GeoXML::xml2Str(text.get()), "hello");
}

TEST(StringWrappers, hasAttribute) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  setAttribute(*root, "name", "atlas");

  EXPECT_TRUE(GeoXML::hasAttribute(*root, "name"));
  EXPECT_FALSE(GeoXML::hasAttribute(*root, "missing"));
}

TEST(StringWrappers, nodeName) {
  auto doc = makeDocument("myRoot");
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  EXPECT_EQ(GeoXML::nodeName(*root), "myRoot");
}

TEST(StringWrappers, tagName) {
  auto doc = makeDocument("root");
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  DOMElement* child = appendChildElement(*doc, *root, "subdetector");
  ASSERT_NE(child, nullptr);

  EXPECT_EQ(GeoXML::tagName(*child), "subdetector");
}

TEST(StringWrappers, fetchAttribute_existing) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  setAttribute(*root, "material", "silicon");

  EXPECT_EQ(GeoXML::fetchAttribute(*root, "material"), "silicon");
}

TEST(StringWrappers, fetchAttribute_missing) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  EXPECT_EQ(GeoXML::fetchAttribute(*root, "missing"), "");
}

TEST(StringWrappers, rangeList_single_values) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  setAttribute(*root, "ids", "1 4 7");

  const std::vector<int> expected{1, 4, 7};
  EXPECT_EQ(GeoXML::rangeList(*root, "ids"), expected);
}

TEST(StringWrappers, rangeList_ranges_and_values) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  setAttribute(*root, "ids", "1-3 5 8-10");

  const std::vector<int> expected{1, 2, 3, 5, 8, 9, 10};
  EXPECT_EQ(GeoXML::rangeList(*root, "ids"), expected);
}

TEST(StringWrappers, rangeList_missing_attribute) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  EXPECT_TRUE(GeoXML::rangeList(*root, "ids").empty());
}

TEST(StringWrappers, rangeList_empty_attribute) {
  auto doc = makeDocument();
  ASSERT_TRUE(doc);

  DOMElement* root = doc->getDocumentElement();
  ASSERT_NE(root, nullptr);

  setAttribute(*root, "ids", "");

  EXPECT_TRUE(GeoXML::rangeList(*root, "ids").empty());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new XercesEnvironment);
  return RUN_ALL_TESTS();
}