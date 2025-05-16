#pragma once

#include <string>
#include <map>
#include "GeoModelKernel/GeoVPhysVol.h"

#define volumeTags std::map<std::string, GeoVPhysVol* >
#define tagCatalog std::map<std::string, volumeTags >

class GeoVolumeTagCatalog {
public:
	void addTaggedVolume(std::string category, std::string tag, GeoVPhysVol* v)
	{
		theTags[category][tag]=v;	
	} 
	GeoVPhysVol* getTaggedVolume(std::string category, std::string tag)
	{
		return theTags[category][tag];
	}

	GeoVolumeTagCatalog() {}
	static GeoVolumeTagCatalog* VolumeTagCatalog() {
		static GeoVolumeTagCatalog* theCatalog=new GeoVolumeTagCatalog;
		return theCatalog;
	}
	tagCatalog theTags;
};
