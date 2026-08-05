#pragma once

#include "ECS/Entity.h"
#include "EngineAPI.h"
#include "Scene.h"
#include "AssetID.h"

#include "entt.hpp"
#include "json.hpp"

#include <functional>
#include <unordered_map>
#include <optional>
#include <string>

namespace Vortx
{

	class ENGINE_API SerializerLoader
	{
	public:
		using WriteFn = std::function<void(Entity entity, nlohmann::json& out)>;
		using ReadFn = std::function<void(Entity entity, const nlohmann::json& in)>;
		using CollectFn = std::function<void(Entity entity, std::vector<AssetID>& out)>;
		using JsonCollectFn = std::function<void(const nlohmann::json& componentJson, std::vector<AssetID>& out)>;

		static std::unordered_map<entt::id_type, WriteFn> writeFns;
		static std::unordered_map<entt::id_type, ReadFn> readFns;
		static std::unordered_map<entt::id_type, CollectFn> collectFns;
		static std::unordered_map<entt::id_type, JsonCollectFn> jsonCollectFns;

		static void Reset();

		static std::vector<uint8_t> SerializeScene(Scene& scene);

		static void DeserializeScene(Scene& scene, const std::vector<uint8_t>& cbor);

		static std::vector<AssetID> CollectSceneAssets(Scene& scene);
		static std::vector<AssetID> CollectSceneAssetsFromFile(const std::string& path);

		static void InvalidateSceneDependencyCache(const std::string& path);

	private:

		static void RegisterComponent(entt::id_type hash, WriteFn write, ReadFn read, CollectFn collect = nullptr, JsonCollectFn jsonCollect = nullptr)
		{
			writeFns[hash] = std::move(write);
			readFns[hash] = std::move(read);
			if (collect) collectFns[hash] = std::move(collect);
			if (jsonCollect) jsonCollectFns[hash] = std::move(jsonCollect);
		}

		template<typename T>
		static void RegisterComponent(WriteFn write, ReadFn read, CollectFn collect = nullptr, JsonCollectFn jsonCollect = nullptr)
		{
			RegisterComponent(entt::type_hash<T>::value(), std::move(write), std::move(read), std::move(collect), std::move(jsonCollect));
		}
	};
}