#pragma once

#include <string>
#include <map>
#include "GeoModelKernel/GeoVPhysVol.h"

#define volumeTags std::map<std::string, PVLink >
#define tagCatalog std::map<std::string, volumeTags >

class GeoVolumeTagCatalog {
public:
	void addTaggedVolume(const std::string& category, const std::string& tag, PVLink v) {
		theTags[category][tag]=v;	
	} 
	GeoVPhysVol* getTaggedVolume(const std::string& category, const std::string& tag) 
	{
		return theTags[category][tag];
	}

	GeoVolumeTagCatalog() = default;
	static GeoVolumeTagCatalog* VolumeTagCatalog() {
		static GeoVolumeTagCatalog* theCatalog=new GeoVolumeTagCatalog;
		return theCatalog;
	}
	tagCatalog theTags;
};
