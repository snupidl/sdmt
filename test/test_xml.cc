// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "tinyxml2.h"

#include <gtest/gtest.h>

namespace xml = tinyxml2;

TEST(XMLTest, CheckConfig) {
    xml::XMLDocument doc;
    xml::XMLError err;
    xml::XMLNode* node;
    xml::XMLElement* element;

    // load config file
    err = doc.LoadFile("../config.xml");
    if (err != 0) {
        EXPECT_TRUE(false) << "wrong path for sdmt config file";
        return;
    }

    // get root node
    node = doc.FirstChild();
    
    // get path for sdmt archive
    std::string archive_path;
    element = node->FirstChildElement("ArchivePath");
    if (element == nullptr) {
        EXPECT_TRUE(false) << "parsing ArchivePath was failed";
    } else {
        archive_path = element->GetText();
    }

    // get path for fti_config
    std::string fti_config;
    element = node->FirstChildElement("FTIConfig");
    if (element == nullptr) {
        EXPECT_TRUE(false) << "parsing FTIConfig was failed";
    } else {
        fti_config = element->GetText();
    }
}
