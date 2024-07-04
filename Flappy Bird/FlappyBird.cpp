#include <iostream>
#include <fstream>
#include <list>
#include <windows.h>
#include "CrabbyGraphics.h"

class Console : public CrabbyGraphics
{
private:
	float fBirdPosition = 0.0f;
	float fBirdVelocity = 0.0f;
	const float fBirdAcceleration = 100.0f;

	float fSectionWidth;				// width of each section
	std::list<int> listSection;			// section heights
	float fLevelPosition = 0.0f;
	bool bHasCollided = false;
	bool bResetGame = false;
	int nFlapCount;
	int nAttemptCount;
	int nMaxFlapCount;

public:
	Console()
	{
	}

	bool Setup() override
	{
		listSection = { 0, 0, 0, 0 };
		fSectionWidth = (float)ScreenWidth() / (float)(listSection.size() - 1);		// -1 because one element from the list is not displayed on the screen
		nAttemptCount = 0;
		nMaxFlapCount = 0;

		bHasCollided = false;
		bResetGame = true;

		return true;
	}

	bool Update(float fElapsedTime) override
	{
		if (bResetGame)
		{
			bResetGame = false;
			bHasCollided = false;
			listSection = { 0, 0, 0, 0 };
			fBirdPosition = ScreenHeight() / 2.0f;
			fBirdVelocity = 0.0f;
			nFlapCount = 0;
			nAttemptCount++;
		}

		if (bHasCollided)
		{
			DrawString(0, 1, L"Game over. Press SPACE to start again.");
			if (m_keys[VK_SPACE].bReleased)
			{
				bResetGame = true;
			}
		}
		else
		{

			ClearScreen();

			// Physics
			if (m_keys[VK_SPACE].bPressed && fBirdVelocity >= 2.0f)
			{
				fBirdVelocity = -50.0f;
				nFlapCount++;
				if (nFlapCount > nMaxFlapCount)
					nMaxFlapCount = nFlapCount;
			}

			fBirdVelocity += fBirdAcceleration * fElapsedTime;
			fBirdPosition += fBirdVelocity * fElapsedTime;

			fLevelPosition += 14.0f * fElapsedTime;

			if (fLevelPosition > fSectionWidth)
			{
				fLevelPosition -= fSectionWidth;

				listSection.pop_front();
				int i = Random(1, 20);
				if (i <= 5) i = 0;
				listSection.push_back(i);
			}

			// Draw levels

			int nSection = 0;
			for (auto s : listSection)
			{
				if (s != 0)
				{
					Fill({ (int)(nSection * fSectionWidth + 10 - fLevelPosition), ScreenHeight() - s }, { (int)(nSection * fSectionWidth + 15 - fLevelPosition), ScreenHeight() }, FG_GREEN);
					Fill({ (int)(nSection * fSectionWidth + 10 - fLevelPosition), 0 }, { (int)(nSection * fSectionWidth + 15 - fLevelPosition), ScreenHeight() - s - 15 }, FG_GREEN);
				}
				nSection++;
			}

			int nBirdX = (int)(ScreenWidth() / 3.0f);

			// Collision detection
			bHasCollided = fBirdPosition < -2 || fBirdPosition > ScreenHeight() + 2 ||
				m_bufScreenData[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX].Attributes == FG_GREEN ||
				m_bufScreenData[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX].Attributes == FG_GREEN ||
				m_bufScreenData[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX + 6].Attributes == FG_GREEN ||
				m_bufScreenData[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX + 6].Attributes == FG_GREEN;

			// Draw bird

			if (fBirdVelocity > 0.0f)
			{
				DrawString(nBirdX, fBirdPosition + 0, L"\\\\\\");
				DrawString(nBirdX, fBirdPosition + 1, L"<\\\\\\=Q");
			}
			else
			{
				DrawString(nBirdX, fBirdPosition + 0, L"<///=Q");
				DrawString(nBirdX, fBirdPosition + 1, L"///");
			}

			DrawString(0, 0, L"Attempt: " + std::to_wstring(nAttemptCount) + L" Score: " + std::to_wstring(nFlapCount) + L" High Score: " + std::to_wstring(nMaxFlapCount));
			if (nAttemptCount > 10)
				DrawString(0, 1, L"What are you even doing? Go get a life.");
			else if(nAttemptCount > 5)
				DrawString(0, 1, L"Enough for today.");
		}

		return true;
	}
};

int main()
{
	Console console;

	if (console.ConstructConsole(80, 40, 16, 16))
		console.Start();
	else
		std::wcout << L"Select a different screen resolution/font dimension." << std::endl;

	return 0;
}