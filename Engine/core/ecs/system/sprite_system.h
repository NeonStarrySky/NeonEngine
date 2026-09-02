#pragma once

#include "core/ecs/component_manager.h"
#include "core/ecs/component_storage.h"
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
		ComponentManager& componentManager;
		ComponentStorage<SpriteComponent>& spriteComponents;
	public:
		SpriteSystem(Logger& logger, Setting& setting, ComponentManager& componentManager) :
			logger(logger),
			setting(setting),
			componentManager(componentManager),
			spriteComponents(componentManager.registerComponent<SpriteComponent>())
		{}
		void update() {

		}
	};
}