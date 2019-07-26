#ifndef GEAROENIX_CORE_EVENT_BUTTON_BUTTON_HPP
#define GEAROENIX_CORE_EVENT_BUTTON_BUTTON_HPP
#include "../cr-types.hpp"
namespace gearoenix::core::event::button {
	struct DeviceId {
		typedef enum {
			JOYSTICK = 1,
			KEYBOARD = 2,
			MOTION = 3,
			MOUSE = 4,
		} Type;
	};
	struct KeyId {
		typedef enum {
			LEFT,
			RIGHT,
			MIDDLE,
			UP,
			DOWN,
			FRONT,
			BACK,
			BACKSCAPE,
			ESCAPE,
			ENTER,
			SELECT,
			START,
			HOME,
			END,
			DELETE,
			TILDA,
			TAB,
			CAPS_LOCK,
			RIGHT_SHIFT,
			LEFT_SHIFT,
			RIGHT_CONTROL,
			LEFT_CONTROL,
			SPACE,
			LEFT_SUPER,
			RIGHT_SUPER,
			LEFT_ALT,
			RIGHT_ALT,
			PAGE_DOWN,
			PAGE_UP,
			MINUS,
			PLUS,
			LEFT_BRACKET,
			RIGHT_BRACKET,
			SEMICOLON,
			QOUTE,
			COMMA,
			DOT,
			SLASH,
			BACKSLASH,
			NUM_0,
			NUM_1,
			NUM_2,
			NUM_3,
			NUM_4,
			NUM_5,
			NUM_6,
			NUM_7,
			NUM_8,
			NUM_9,
			NUMPAD_0,
			NUMPAD_1,
			NUMPAD_2,
			NUMPAD_3,
			NUMPAD_4,
			NUMPAD_5,
			NUMPAD_6,
			NUMPAD_7,
			NUMPAD_8,
			NUMPAD_9,
			NUMPAD_SLASH,
			NUMPAD_STAR,
			NUMPAD_MINUS,
			NUMPAD_PLUS,
			NUMPAD_ENTER,
			NUMPAD_DOT,
			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			R1,
			R2,
			L1,
			L2,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
		} Type;
	};
	struct ActionId {
		typedef enum {
			PRESS,
			RELEASE,
			DOUBLE,
		} Type;
	};
	struct Data {
		ActionId action;
		DeviceId device;
		KeyId key;
		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;
	};
}
#endif