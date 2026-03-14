#pragma once

#include <array>
#include <iostream>
//#include <GLFW/glfw3.h>
struct GLFWwindow;

namespace neon::core
{
	enum class Key : int
	{
		unknown = -1,

		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T, U,
		V, W, X, Y, Z,

		Num0, Num1, Num2, Num3, Num4,
		Num5, Num6, Num7, Num8, Num9,

		Escape,
		Space,
		Enter,
		Tab,
		Backspace,

		Left,
		Right,
		Up,
		Down,

		LeftShift,
		RightShift,

		LeftCtrl,
		RightCtrl,

		LeftAlt,
		RightAlt,

		F1, F2, F3, F4, F5,
		F6, F7, F8, F9, F10,
		F11, F12,

		Count
	};

	class InputSystem
	{
	private:
		struct Input
		{
			std::array<bool, static_cast<size_t>(Key::Count)> curent_keys = { false };
			std::array<bool, static_cast<size_t>(Key::Count)> previous_keys = { false };
		} input;
		/// <summary>
		/// 把GLFW的按键码转换为Key枚举值
		/// </summary>
		/// <param name="glfwKey">GLFW的枚举</param>
		/// <returns>返回枚举类Key</returns>
		static constexpr Key glfwKeyToKey(int glfwKey);
	public:
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
			int mods);

		InputSystem();
		~InputSystem() = default;

		bool init(GLFWwindow* window);

		void swap(); // Update the input state (called every frame)

		const Input& getInput() const { return input; }

		bool keyPressed(Key key) const;
		bool keyJustPressed(Key key) const;
		bool keyJustReleased(Key key) const;
		bool keyReleased(Key key) const;
		bool anyKeyPressed() const;
		bool anyKeyJustPressed() const;
		bool anyKeyJustReleased() const;
		bool anyKeyReleased() const;
		bool keyPressing(Key key) const;
	};
}