#pragma once

#include "core/ecs/dynamic_component_manager.h"
#include "core/ecs/dynamic_component_storage.h"
#include "core/ecs/components/sprite_component.h"
#include "core/logger.h"
#include "core/setting.h"

#include "graphics/texture_manager.h"

namespace neon::core::ecs
{
	class SpriteSystem
	{
		using TextureManager = graphics::gl::TextureManager;

		Logger& logger;
		Setting& setting;
		DynamicComponentManager& dynamicComponentManager;
		DynamicComponentStorage<SpriteComponent>& spriteComponents;
	public:
		SpriteSystem(Logger& logger, Setting& setting, DynamicComponentManager& dynamicComponentManager) :
			logger(logger),
			setting(setting),
			dynamicComponentManager(dynamicComponentManager),
			spriteComponents(dynamicComponentManager.registerComponent<SpriteComponent>())
		{}
		void update() {

		}
	};
}