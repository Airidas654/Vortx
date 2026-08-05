#pragma once

#include "AssetID.h"
#include "Core/Asset.h"
#include "Core/AssetMeta.h"

#include <functional>
#include <string>
#include <unordered_map>

using namespace Vortx;

class AssetPropertiesLoader
{

	struct AssetData
	{
		bool hasApply;
	};

public:

	using DrawFn = std::function<void(const Vortx::AssetID& id, AssetMeta&)>;

	static std::unordered_map<AssetType, AssetData> assetData;
	static std::unordered_map<AssetType, DrawFn> assetFunc;
	
	static void Reset();

	static void DrawPropertiesPanel(const AssetID& id, const std::string& path);

private:

	static void RegisterComponent(AssetType type, bool hasApply, DrawFn drawFn);
};