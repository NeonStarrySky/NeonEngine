
#include "Engine.h"//glad
#include "input_system.h"//glfw
#include <GLFW/glfw3.h>

namespace neon::core
{
	InputSystem::InputSystem() {

	}

	bool InputSystem::init(GLFWwindow* window)//Initialize the input system 
	{

		glfwSetKeyCallback(window, keyCallback); // Set the key callback for GLFW
		input.curent_keys.fill(false);
		input.previous_keys.fill(false);
		return true; // Return true if initialization is successful

	}

	void InputSystem::swap()
	{

		input.previous_keys = input.curent_keys;//±£¥Ê…œ“ª÷°◊¥Ã¨

	}

	bool InputSystem::keyPressed(Key key) const
	{

		size_t index = static_cast<size_t>(key);

		size_t count = static_cast<size_t>(Key::Count);
		//return index < count && input.curent_keys[index];
		return count > index && index >= 0 && input.curent_keys[index];
	}
	bool InputSystem::keyPressing(Key key) const
	{
		size_t index = static_cast<size_t>(key);

		size_t count = static_cast<size_t>(Key::Count);
		//return index < count && input.curent_keys[index];
		return count > index && index >= 0 && input.curent_keys[index] && input.previous_keys[index];
		return false;
	}

	void InputSystem::keyCallback(GLFWwindow* window, int key, int, int action, int)
	{
		auto* engine =
			(core::Engine*)glfwGetWindowUserPointer(window); // Retrieve the pointer to the Engine instance

		//switch (action)

		if (action == GLFW_PRESS) {
			//std::cerr << "Key Pressed: " << static_cast<size_t>(glfwKeyToKey(key)) << " (Key Code)\n";
			engine->getInputSystem().input.curent_keys[static_cast<size_t>(glfwKeyToKey(key))] = true;
		}
		else if (action == GLFW_RELEASE) {
			//std::cerr << "Key Release: " << static_cast<size_t>(glfwKeyToKey(key)) << " (Key Code)\n";
			engine->getInputSystem().input.curent_keys[static_cast<size_t>(glfwKeyToKey(key))] = false;
		}
	}

	//Mapping functions
	constexpr Key InputSystem::glfwKeyToKey(int glfwKey)// Map GLFW key codes to our Key enum
	{
		//glfwKey  « GLFW µƒ∞¥º¸¬Î£¨∑∂Œß¥” 0 µΩ GLFW_KEY_LAST£®GLFW_KEY_LAST  « GLFW ∂®“Âµƒ◊Ó¥Û∞¥º¸¬Î£¨Õ®≥£ « 348£©
		if (glfwKey >= GLFW_KEY_A && glfwKey <= GLFW_KEY_Z) {
			return static_cast<Key>((glfwKey - GLFW_KEY_A) + static_cast<int>(Key::A));
		}
		if (glfwKey >= GLFW_KEY_0 && glfwKey <= GLFW_KEY_9) {
			return static_cast<Key>(glfwKey - GLFW_KEY_0 + static_cast<int>(Key::Num0));
		}
		switch (glfwKey) {
		case GLFW_KEY_ESCAPE: return Key::Escape;
		case GLFW_KEY_SPACE: return Key::Space;
		case GLFW_KEY_ENTER: return Key::Enter;
		case GLFW_KEY_TAB: return Key::Tab;
		case GLFW_KEY_BACKSPACE: return Key::Backspace;
		case GLFW_KEY_LEFT: return Key::Left;
		case GLFW_KEY_RIGHT: return Key::Right;
		case GLFW_KEY_UP: return Key::Up;
		case GLFW_KEY_DOWN: return Key::Down;
		case GLFW_KEY_LEFT_SHIFT: return Key::LeftShift;
		case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
		case GLFW_KEY_LEFT_CONTROL: return Key::LeftCtrl;
		case GLFW_KEY_RIGHT_CONTROL: return Key::RightCtrl;
		case GLFW_KEY_LEFT_ALT: return Key::LeftAlt;
		case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;
		case GLFW_KEY_F1: return Key::F1;
		case GLFW_KEY_F2: return Key::F2;
		case GLFW_KEY_F3: return Key::F3;
		case GLFW_KEY_F4: return Key::F4;
		case GLFW_KEY_F5: return Key::F5;
		case GLFW_KEY_F6: return Key::F6;
		case GLFW_KEY_F7: return Key::F7;
		case GLFW_KEY_F8: return Key::F8;
		case GLFW_KEY_F9: return Key::F9;
		case GLFW_KEY_F10: return Key::F10;
		case GLFW_KEY_F11: return Key::F11;
		case GLFW_KEY_F12: return Key::F12;
		default: return Key::unknown; // Return unknown for unmapped keys
		}
	}

}