#pragma once

#include "ECS/Entity.h"
#include "Scene.h"

#include "entt.hpp"
#include "json.hpp"

#include <functional>
#include <unordered_map>
#include <optional>
#include <string>

static class SerializerLoader
{
public:
	using WriteFn = std::function<void(Entity entity, nlohmann::json& out)>;
	using ReadFn = std::function<void(Entity entity, const nlohmann::json& in)>;

	static std::unordered_map<entt::id_type, WriteFn> writeFns;
	static std::unordered_map<entt::id_type, ReadFn>  readFns;

	static void Reset();

	static std::string SerializeScene(Scene& scene);

	static void DeserializeScene(Scene& scene, const std::string& json);

private:

	static void RegisterComponent(entt::id_type hash, WriteFn write, ReadFn read)
	{
		writeFns[hash] = std::move(write);
		readFns[hash] = std::move(read);
	}

	template<typename T>
	static void RegisterComponent(WriteFn write, ReadFn read)
	{
		RegisterComponent(entt::type_hash<T>::value(), std::move(write), std::move(read));
	}
};