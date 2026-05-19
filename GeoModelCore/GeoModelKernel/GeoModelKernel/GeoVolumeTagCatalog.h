/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#pragma once


#include "GeoModelKernel/GeoVPhysVol.h"
#include <string>
#include <string_view>
#include <map>



class GeoVolumeTagCatalog {
public:
  using VolumeTags = std::map<std::string, PVLink, std::less<>>;
  using TagCatalog = std::map<std::string, VolumeTags, std::less<>>;
  
	void addTaggedVolume(std::string_view category, std::string_view tag, PVLink v) {
		theTags[std::string{category}][std::string{tag}] = std::move(v);	
	} 
	GeoVPhysVol* getTaggedVolume(const std::string& category, const std::string& tag) 
	{
		auto categoryIt = theTags.find(category);
    if (categoryIt == theTags.end()) {
      return nullptr;
    }
    auto tagIt = categoryIt->second.find(tag);
    if (tagIt == categoryIt->second.end()) {
      return nullptr;
    }
    return tagIt->second;
	}

	GeoVolumeTagCatalog() = default;
	static GeoVolumeTagCatalog* VolumeTagCatalog() {
		static GeoVolumeTagCatalog* theCatalog=new GeoVolumeTagCatalog;
		return theCatalog;
	}
	TagCatalog theTags;
};
