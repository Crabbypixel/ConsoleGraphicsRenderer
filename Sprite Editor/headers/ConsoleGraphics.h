/*
*	A very simple graphics library using the Windows API.
*	To be developed further.
*	Version: 1.0.2
* 
*	By: Crabbyfeet
*	Dated: 03-06-2024
* 
*	Updates:
*	* Optimizations (1.0.1)
*	* Console closes only when window is active (1.0.2)
*	* Added sprites
* 
*	TODO:
*	* Add scroll wheel events
*	* More draw functions
*	* Sounds (later)
* 
*	History
*	* 31-05-2024: 1.0.1
*	* 03-06-2024: 1.0.2
*   * 08-06-2024: 1.0.4
*/

#pragma once
#include "vec_2d.h"
#include "random.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>
#include <condition_variable>
#include <windows.h>

#ifndef CF_GRAPHICS
#define CF_GRAPHICS
#endif

enum COLOR
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007,
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

enum PIXEL_TYPE
{
	PIXEL_SOLID = 0x2588,
	PIXEL_THREEQUARTERS = 0x2593,
	PIXEL_HALF = 0x2592,
	PIXEL_QUARTER = 0x2591,
};


class Sprite
{
private:
	short* spriteData;
	int nSpriteDimX, nSpriteDimY;

public:
	cf::vec_2d<float> vPos;

	Sprite()
	{
		nSpriteDimX = nSpriteDimY = 0;
		spriteData = nullptr;
	}

	bool Load(std::string sFile)
	{
		FILE* f = nullptr;
		fopen_s(&f, sFile.c_str(), "r");

		if (!f)
			return false;

		fread(&nSpriteDimX, sizeof(int), 1, f);
		fread(&nSpriteDimY, sizeof(int), 1, f);

		spriteData = new short[nSpriteDimX * nSpriteDimY];

		fread(spriteData, sizeof(short), nSpriteDimX * nSpriteDimY, f);

		return true;
	}

	cf::vec_2d<int> GetSpriteDim() const
	{
		return cf::vec_2d{ nSpriteDimX , nSpriteDimY };
	}

	short operator[](int x) const
	{
		return spriteData[x];
	}

	~Sprite()
	{
		delete[] spriteData;
	}
};

class ConsoleGraphics
{
private:
	int m_screenWidth;
	int m_screenHeight;
	HANDLE m_hConsole;
	HANDLE m_hConsoleInput;
	HANDLE m_hOriginalConsole;
	SMALL_RECT m_rectWindow;
	bool m_bIsConsoleInFocus = true;

	short m_keyOldState[256] = { 0 };
	short m_keyNewState[256] = { 0 };
	bool m_mouseOldState[5] = { 0 };
	bool m_mouseNewState[5] = { 0 };
	int m_mousePosX;
	int m_mousePosY;

	// static thread variables - to handle window closing event
	static std::mutex m_muxGame;
	static std::condition_variable m_cvConditionVariable;
	static std::atomic<bool> m_bIsRunning;

	// Thread which runs the game engine
	void GameThread()
	{
		if (!Setup())
			m_bIsRunning = false;

		auto dt1 = std::chrono::system_clock::now();
		auto dt2 = std::chrono::system_clock::now();

		while (m_bIsRunning)
		{
			while (m_bIsRunning)
			{
				dt2 = std::chrono::system_clock::now();
				std::chrono::duration<float> elapsedTime = dt2 - dt1;
				dt1 = dt2;
				float fElapsedTime = elapsedTime.count();

				// Handle Keyboard Inputs
				for (int i = 0; i < 256; i++)
				{
					m_keyNewState[i] = GetAsyncKeyState(i);

					m_keys[i].bPressed = false;
					m_keys[i].bReleased = false;

					if (m_keyNewState[i] != m_keyOldState[i])
					{
						if (m_keyNewState[i] & 0x8000)
						{
							m_keys[i].bPressed = !m_keys[i].bHeld;
							m_keys[i].bHeld = true;
						}
						else
						{
							m_keys[i].bReleased = true;
							m_keys[i].bHeld = false;
						}
					}

					m_keyOldState[i] = m_keyNewState[i];
				}

				// Handle Mouse Inputs
				INPUT_RECORD inBuf[32];
				DWORD events = 0;
				GetNumberOfConsoleInputEvents(m_hConsoleInput, &events);
				if (events > 0)
					ReadConsoleInput(m_hConsoleInput, inBuf, events, &events);

				// Handle mouse events
				for (DWORD i = 0; i < events; i++)
				{
					switch (inBuf[i].EventType)
					{
					case FOCUS_EVENT:
					{
						m_bIsConsoleInFocus = inBuf[i].Event.FocusEvent.bSetFocus;
					}
					break;

					case MOUSE_EVENT:
					{
						switch (inBuf[i].Event.MouseEvent.dwEventFlags)
						{
						case MOUSE_MOVED:
						{
							m_mousePosX = inBuf[i].Event.MouseEvent.dwMousePosition.X;
							m_mousePosY = inBuf[i].Event.MouseEvent.dwMousePosition.Y;
						}
						break;

						case 0:
						{
							for (int m = 0; m < 5; m++)
								m_mouseNewState[m] = (inBuf[i].Event.MouseEvent.dwButtonState & (1 << m)) > 0;

						}
						break;

						default:
							break;
						}
					}
					break;

					default:
						break;
						// We don't care just at the moment
					}
				}

				for (int m = 0; m < 5; m++)
				{
					m_mouse[m].bPressed = false;
					m_mouse[m].bReleased = false;

					if (m_mouseNewState[m] != m_mouseOldState[m])
					{
						if (m_mouseNewState[m])
						{
							m_mouse[m].bPressed = true;
							m_mouse[m].bHeld = true;
						}
						else
						{
							m_mouse[m].bReleased = true;
							m_mouse[m].bHeld = false;
						}
					}

					m_mouseOldState[m] = m_mouseNewState[m];
				}

				// Check for Ctrl-C event (while window is active)
				if (m_bIsConsoleInFocus)
				{
					if (m_keys[VK_CONTROL].bHeld && m_keys['C'].bReleased)
					{
						GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
					}
				}

				if (!Update(fElapsedTime))
					m_bIsRunning = false;

				// Draw onto screen
				wchar_t s[256];
				swprintf_s(s, 256, L"%s : %d FPS", m_sConsoleName.c_str(), (int)(1.0f / fElapsedTime));
				SetConsoleTitle(s);
				WriteConsoleOutput(m_hConsole, m_bufScreenData, { (short)m_screenWidth, (short)m_screenHeight }, { 0,0 }, &m_rectWindow);
			}

			// Contol reaches here if window close event occurs
			if (Destroy()) {
				delete[] m_bufScreenData;
				SetConsoleActiveScreenBuffer(m_hOriginalConsole);
				m_cvConditionVariable.notify_one();
			}
			else
			{
				m_bIsRunning = true;
			}
		}
	}

	// This function is invoked when any closing event triggers
	static BOOL ControlCloseHandler(DWORD evt)
	{
		if (evt == CTRL_C_EVENT || evt == CTRL_CLOSE_EVENT)
		{
			m_bIsRunning = false;

			std::unique_lock<std::mutex> ul(m_muxGame);
			m_cvConditionVariable.wait(ul);
		}

		return true;
	}

	int GraphicError(const wchar_t* msg) const
	{
		wchar_t buf[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
		wprintf(L"Error: %s\n\t%s\n", msg, buf);
		return 0;
	}

protected:
	std::wstring m_sConsoleName;
	CHAR_INFO* m_bufScreenData;

	struct sKeyState {
		bool bPressed;
		bool bReleased;
		bool bHeld;
	}m_keys[256], m_mouse[5];

	sKeyState GetKey(int nKeyID) const { return m_keys[nKeyID]; }
	sKeyState GetMouse(int nMouseButtonID) const { return m_mouse[nMouseButtonID]; }

public:
	//template <typename T = int>
	/*class vec_2d
	{
	static_assert(std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>, "vec_2d only supports int, float, and double");
	public:
		T x, y;

		vec_2d() : x{ 0 }, y{ 0 }
		{}

		vec_2d(T x, T y) : x{ x }, y{ y }
		{}

		// conversion constructor
		template<typename U>
		vec_2d(const vec_2d<U>& v) : x{ static_cast<T>(v.x) }, y{ static_cast<T>(v.y) }
		{}

		vec_2d operator+(const vec_2d& v) const { return vec_2d(x + v.x, y + v.y); }

		vec_2d operator-(const vec_2d& v) const { return vec_2d(x - v.x, y - v.y); }

		// Scalar multiplication
		vec_2d operator*(int w) const { return vec_2d(w * x, w * y); }

		// Dot product
		T operator*(const vec_2d& v) const { return x * v.x + y * v.y; }

		float Mag() const { return sqrtf(x * x + y * y); }

		// returns square of magnitude, for faster computations
		T Mag2() const { return x * x + y * y; }

		vec_2d operator=(const vec_2d& p)
		{
			x = p.x;
			y = p.y;

			return *this;
		}

		vec_2d& operator+=(const vec_2d& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		vec_2d& operator-=(const vec_2d& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}
	};*/
	using point_2d = cf::vec_2d<int>;

	class triangle
	{
	public:
		cf::vec_2d<float> p[3];
		COLOR edgeColor;
		COLOR fillColor;

		triangle() : p{ {0, 0}, {0, 0}, {0, 0} }, edgeColor{ FG_WHITE }, fillColor{ FG_BLACK }
		{}

		triangle(point_2d p1, point_2d p2, point_2d p3, COLOR fillColor = FG_BLACK, COLOR edgeColor = FG_WHITE)
			: p{ p1, p2, p3 }, edgeColor{ edgeColor }, fillColor{ fillColor }
		{}

		triangle(const triangle& t)
		{
			p[0] = t.p[0];
			p[1] = t.p[1];
			p[2] = t.p[2];

			edgeColor = t.edgeColor;
			fillColor = t.fillColor;
		}

		triangle& operator=(const triangle& t)
		{
			p[0] = t.p[0];
			p[1] = t.p[1];
			p[2] = t.p[2];

			edgeColor = t.edgeColor;
			fillColor = t.fillColor;

			return *this;
		}

		float getArea() const
		{
			return (float)std::abs((p[0].x * (p[1].y - p[2].y) + p[1].x * (p[2].y - p[0].y) + p[2].x * (p[0].y - p[1].y)) / 2.0);
		}

		point_2d midpoint() const
		{
			return point_2d((int)((p[0].x + p[1].x + p[2].x) / 3.0), (int)((p[0].y + p[1].y + p[2].y) / 3.0));
		}
	};

	struct mat3x3 {
		float m[3][3] = { 0 };
	};

	int ScreenWidth() const { return m_screenWidth; }
	int ScreenHeight() const { return m_screenHeight; }
	int GetMousePosX() const { return m_mousePosX; }
	int GetMousePosY() const { return m_mousePosY; }

	// Matrix functions
	void MultiplyMatrix3x3(const cf::vec_2d<float>& i, cf::vec_2d<float>& o, const mat3x3& m)
	{
		o.x = m.m[0][0] * i.x + m.m[0][1] * i.y + m.m[0][2] * 1.0f;
		o.y = m.m[1][0] * i.x + m.m[1][1] * i.y + m.m[1][2] * 1.0f;
		float w = m.m[2][0] * i.x + m.m[2][1] * i.y + m.m[2][2] * 1.0f;
	}

	// Other functions
	float Random()
	{
		return Random::get(1, 1000000) / 1000000.0f;
	}

	int Random(int min, int max)	
	{
		return Random::get(min, max);
	}

	bool InFocus() const { return m_bIsConsoleInFocus; }

	// Draw functions
	void ClearScreen()
	{
		Fill({ 0, 0 }, { m_screenWidth, m_screenHeight }, FG_BLACK, PIXEL_SOLID);
	}

	void Pixelate(const point_2d& p, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		if (p.x >= 0 && p.x < m_screenWidth && p.y >= 0 && p.y < m_screenHeight)
		{
			m_bufScreenData[p.y * m_screenWidth + p.x].Char.UnicodeChar = pixelType;
			m_bufScreenData[p.y * m_screenWidth + p.x].Attributes = color;
		}
	}

	void Fill(const point_2d& p1, const point_2d& p2, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		for (int x = p1.x; x < p2.x; x++)
			for (int y = p1.y; y < p2.y; y++)
				Pixelate({ x, y }, color, pixelType);
	}

	void DrawTriangle(const point_2d& p1, const point_2d& p2, const point_2d& p3, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		DrawLine(p1, p2);
		DrawLine(p2, p3);
		DrawLine(p3, p1);
	}

	void DrawTriangle(const triangle& t, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		DrawTriangle(t.p[0], t.p[1], t.p[2]);
	}

	void DrawLine(const point_2d& p1, const point_2d& p2, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		//if(p1.x >= 0 && p2.x >= 0 && p1.x < m_screenWidth && p2.x < m_screenWidth && p1.y >= 0 && p2.y >= 0 && p1.y < m_screenHeight && p2.y < m_screenHeight)
		int dx = p2.x - p1.x;
		int dy = p2.y - p1.y;

		int mod_dx = std::abs(dx);
		int mod_dy = std::abs(dy);

		int px = 2 * mod_dy - mod_dx;
		int py = 2 * mod_dx - mod_dy;

		int x, y, large_x, large_y;

		if (mod_dy <= mod_dx)
		{
			if (dx >= 0)
			{
				x = p1.x; y = p1.y; large_x = p2.x;
			}
			else
			{
				x = p2.x; y = p2.y; large_x = p1.x;
			}

			Pixelate({ x, y }, color, pixelType);

			for (int i = 0; x < large_x; i++)
			{
				x++;
				if (px < 0)
					px = px + 2 * mod_dy;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
						y++;
					else
						y--;

					px = px + 2 * (mod_dy - mod_dx);
				}

				Pixelate({ x, y }, color, pixelType);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = p1.x; y = p1.y; large_y = p2.y;
			}
			else
			{
				x = p2.x; y = p2.y; large_y = p1.y;
			}

			Pixelate({ x, y }, color, pixelType);

			for (int i = 0; y < large_y; i++)
			{
				y = y + 1;
				if (py <= 0)
					py = py + 2 * mod_dx;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
					py = py + 2 * (mod_dx - mod_dy);
				}

				Pixelate({ x, y }, color, pixelType);
			}
		}
	}

	void DrawCircle(const point_2d& center, int radius, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		// Using Midpoint circle algorithm
		int x = 0;
		int y = radius;
		int p = 3 - 2 * radius;
		if (!radius) return;

		while (y >= x)
		{
			Pixelate({ center.x - x, center.y - y }, color, pixelType);		//upper left left
			Pixelate({ center.x - y, center.y - x }, color, pixelType);		//upper upper left
			Pixelate({ center.x + y, center.y - x }, color, pixelType);		//upper upper right
			Pixelate({ center.x + x, center.y - y }, color, pixelType);		//upper right right
			Pixelate({ center.x - x, center.y + y }, color, pixelType);		//lower left left
			Pixelate({ center.x - y, center.y + x }, color, pixelType);		//lower lower left
			Pixelate({ center.x + y, center.y + x }, color, pixelType);		//lower lower right
			Pixelate({ center.x + x, center.y + y }, color, pixelType);		//lower right right

			if (p < 0) p += 4 * x++ + 6;
			else p += 4 * (x++ - y--) + 10;
		}

		DrawLine({ center.x, center.y }, { center.x + radius - 1, center.y }, FG_RED);
		//Pixelate({ center.x, center.y }, FG_BLUE);
	}

	void FillCircle(const point_2d& center, int radius, short color = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		// Taken from wikipedia
		int x = 0;
		int y = radius;
		int p = 3 - 2 * radius;
		if (!radius) return;

		while (y >= x)
		{
			// Modified to draw scan-lines instead of edges
			DrawLine({ center.x - x, center.y - y }, { center.x + x, center.y - y });
			DrawLine({ center.x - y, center.y - x }, { center.x + y, center.y - x });
			DrawLine({ center.x - x, center.y + y }, { center.x + x, center.y + y });
			DrawLine({ center.x - y, center.y + x }, { center.x + y, center.y + x });
			if (p < 0) p += 4 * x++ + 6;
			else p += 4 * (x++ - y--) + 10;
		}
	};

	void DrawString(int x, int y, std::wstring str, short color = FG_WHITE)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			m_bufScreenData[y * m_screenWidth + x + i].Char.UnicodeChar = str[i];
			m_bufScreenData[y * m_screenWidth + x + i].Attributes = color;
		}
	}
	
	void DrawSprite(Sprite& sprite)
	{
		int x, y;
		int nDimX, nDimY;
		nDimX = sprite.GetSpriteDim().x;
		nDimY = sprite.GetSpriteDim().y;

		for (y = 0; y < nDimY; y++)
		{
			for (x = 0; x < nDimX; x++)
			{
				Pixelate({ x + (int)sprite.vPos.x , y + (int)sprite.vPos.y }, sprite[y * nDimX + x]);
			}
		}
	}

	void Clip(int& x, int& y) const
	{
		if (x < 0) x = 0;
		if (x >= m_screenWidth) x = m_screenWidth;
		if (y < 0) y = 0;
		if (y >= m_screenHeight) y = m_screenHeight;
	}

	void Clip(float& x, float& y) const
	{
		if (x < 0.0f) x = 0.0f;
		if (x >= (float)m_screenWidth) x = (float)m_screenWidth;
		if (y < 0.0f) y = 0.0f;
		if (y >= (float)m_screenHeight) y = (float)m_screenHeight;
	}

	void FillTriangle(const point_2d& p1, const point_2d& p2, const point_2d& p3, short fillColor = FG_WHITE, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		// TODO: Use threads maybe?
		// 
		// Calculate bounding box
		int x_min = min(min(p1.x, p2.x), p3.x);
		int y_min = min(min(p1.y, p2.y), p3.y);
		int x_max = max(max(p1.x, p3.x), p3.x);
		int y_max = max(max(p1.y, p2.y), p3.y);

		for (int y = y_min; y <= y_max; y++)
		{
			for (int x = x_min; x <= x_max; x++)
			{
				if (IsPointInsideTriangle({ x, y }, p1, p2, p3) && !(m_bufScreenData[y * m_screenWidth + x].Attributes == FG_WHITE))
				{
					Pixelate({ x, y }, fillColor, PIXEL_SOLID);
				}
			}
		}
	}

	void FillTriangle(const triangle& t, PIXEL_TYPE pixelType = PIXEL_SOLID)
	{
		// Calculate bounding box
		int x_min = min(min((int)(t.p[0].x), (int)(t.p[1].x)), (int)(t.p[2].x));
		int y_min = min(min((int)(t.p[0].y), (int)(t.p[1].y)), (int)(t.p[2].y));
		int x_max = max(max((int)(t.p[0].x), (int)(t.p[1].x)), (int)(t.p[2].x));
		int y_max = max(max((int)(t.p[0].y), (int)(t.p[1].y)), (int)(t.p[2].y));

		for (int y = y_min; y <= y_max; y++)
		{
			for (int x = x_min; x <= x_max; x++)
			{
				if (x >= 0 && x < m_screenWidth && y >= 0 && y < m_screenHeight)
				{
					if (IsPointInsideTriangle({ x, y }, t.p[0], t.p[1], t.p[2]) && !(m_bufScreenData[y * m_screenWidth + x].Attributes == t.edgeColor))
					{
						Pixelate({ x, y }, t.fillColor, pixelType);
					}
				}
			}
		}
	}

	void RotateTriangle(const point_2d& p, float fAngle, triangle& rotatedTriangle, const triangle& tri)
	{
		mat3x3 mat_transrotate;

		mat_transrotate.m[0][0] = cos(fAngle);
		mat_transrotate.m[0][1] = -sin(fAngle);
		mat_transrotate.m[0][2] = p.x * (1.0f - cos(fAngle)) + p.y * sin(fAngle);
		mat_transrotate.m[1][0] = sin(fAngle);
		mat_transrotate.m[1][1] = cos(fAngle);
		mat_transrotate.m[1][2] = p.y * (1.0f - cos(fAngle)) - p.x * sin(fAngle);
		mat_transrotate.m[2][2] = 1;

		MultiplyMatrix3x3(tri.p[0], rotatedTriangle.p[0], mat_transrotate);
		MultiplyMatrix3x3(tri.p[1], rotatedTriangle.p[1], mat_transrotate);
		MultiplyMatrix3x3(tri.p[2], rotatedTriangle.p[2], mat_transrotate);
	}

	void TranslateTriangle(const cf::vec_2d<>& t, triangle& translatedTriangle, const triangle& tri)
	{
		mat3x3 mat_translate;
		mat_translate.m[0][0] = 1;
		mat_translate.m[1][1] = 1;
		mat_translate.m[2][2] = 1;
		mat_translate.m[0][2] = static_cast<float>(t.x);
		mat_translate.m[1][2] = static_cast<float>(t.y);

		MultiplyMatrix3x3(tri.p[0], translatedTriangle.p[0], mat_translate);
		MultiplyMatrix3x3(tri.p[1], translatedTriangle.p[1], mat_translate);
		MultiplyMatrix3x3(tri.p[2], translatedTriangle.p[2], mat_translate);
	}

	bool IsPointInsideTriangle(const point_2d& p, const point_2d& p1, const point_2d& p2, const point_2d& p3)
	{
		auto area = [](point_2d _p1, point_2d _p2, point_2d _p3) {
			return (float)std::abs((_p1.x * (_p2.y - _p3.y) + _p2.x * (_p3.y - _p1.y) + _p3.x * (_p1.y - _p2.y)) / 2.0);
			};

		float fAreaTriangle = area(p1, p2, p3);
		float fA1 = area(p, p2, p3);
		float fA2 = area(p1, p, p3);
		float fA3 = area(p1, p2, p);

		return (fAreaTriangle == fA1 + fA2 + fA3);
	}
	
public:
	ConsoleGraphics()
	{
		m_sConsoleName = L"Console";

		m_screenHeight = 80;
		m_screenWidth = 30;

		m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		m_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

		// Set all keystates to be zero initialized
		std::memset(m_keyOldState, 0, 256 * sizeof(short));
		std::memset(m_keyNewState, 0, 256 * sizeof(short));
		std::memset(m_keys, 0, 256 * sizeof(short));
	}

	~ConsoleGraphics()
	{
		SetConsoleActiveScreenBuffer(m_hOriginalConsole);
	}

	int ConstructConsole(int width, int height, int fontWidth, int fontHeight)
	{
		if (m_hConsole == INVALID_HANDLE_VALUE)
			GraphicError(L"Bad Output Handle Error");

		if (m_hConsoleInput == INVALID_HANDLE_VALUE)
			GraphicError(L"Bad Input Handle Error");

		m_screenWidth = width;
		m_screenHeight = height;

		m_rectWindow = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

		// Set size of the screen buffer
		COORD coord = { (short)m_screenWidth, (short)m_screenHeight };
		if (!SetConsoleScreenBufferSize(m_hConsole, coord))
			return GraphicError(L"Cannot set size of the screen buffer");

		// Assign screen buffer to the console
		if (!SetConsoleActiveScreenBuffer(m_hConsole))
			return GraphicError(L"Cannot assign screen buffer to the console");

		// Set the font size now that the screen buffer has been assigned to the console
		CONSOLE_FONT_INFOEX cfi{ sizeof(CONSOLE_FONT_INFOEX), 0, {(short)fontWidth, (short)fontHeight}, FF_DONTCARE, FW_NORMAL };

		wcscpy_s(cfi.FaceName, L"Consolas");
		if (!SetCurrentConsoleFontEx(m_hConsole, false, &cfi))
			return GraphicError(L"Cannot set font settings");

		// Get screen buffer info and check whether if the window sizes are allowed
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(m_hConsole, &csbi))
			return GraphicError(L"Cannot get console information");
		if (m_screenHeight > csbi.dwMaximumWindowSize.Y)
			return GraphicError(L"Screen Height / Font Height too large");
		if (m_screenWidth > csbi.dwMaximumWindowSize.X)
			return GraphicError(L"Screen Width / Font Width too large");

		// Set Physical Console Window Size
		m_rectWindow = { 0, 0, (short)(m_screenWidth - 1), (short)(m_screenHeight - 1) };
		if (!SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow))
			return GraphicError(L"Cannot create console window");

		// Allow keyboard and mouse inputs
		if (!SetConsoleMode(m_hConsoleInput, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
			return GraphicError(L"Cannot get keyboard/mouse inputs");

		// Allocate memory for screen buffer
		m_bufScreenData = new CHAR_INFO[m_screenWidth * m_screenHeight];

		// Initalize all values to zero
		memset(m_bufScreenData, 0, sizeof(CHAR_INFO) * m_screenWidth * m_screenHeight);

		if(!SetConsoleCtrlHandler(ControlCloseHandler, TRUE))
			return GraphicError(L"Cannot set close handler");

		// Make the window non-resizable
		HWND hwndConsole = GetConsoleWindow();
		if (hwndConsole != NULL) {
			LONG style = GetWindowLong(hwndConsole, GWL_STYLE);
			style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);  // Remove the resizable and maximize box styles
			SetWindowLong(hwndConsole, GWL_STYLE, style);

			// Apply the style changes by calling SetWindowPos
			SetWindowPos(hwndConsole, NULL, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}

		return 1;
	}

	void Start()
	{
		// Create a separate thread
		m_bIsRunning = true;
		std::thread gameThread = std::thread(&ConsoleGraphics::GameThread, this);

		// Wait until it exits
		gameThread.join();
	}

// Virtual functions
protected:
	// These functions has to be overriden
	virtual bool Setup() = 0;						// Called when the console engine starts
	virtual bool Update(float fElapsedTime) = 0;	// Called on every frame

	// Optional to override
	virtual bool Destroy() { return true; }
};

// Initialize static variables
std::atomic<bool> ConsoleGraphics::m_bIsRunning(false);
std::condition_variable ConsoleGraphics::m_cvConditionVariable;
std::mutex ConsoleGraphics::m_muxGame;