#pragma once

#define MATRIX_WIDTH	10
#define MATRIX_HEIGHT	20

class Block;

class Game
{
public:
	Game();
	~Game();

	void togglePause();
	void moveLeft();
	void moveRight();
	void rotate();
	void drop();

	BOOL tick();
	void draw(ID2D1HwndRenderTarget *pRenderTarget, int logoHeight, ID2D1Bitmap *pBrickImage, IDWriteTextFormat *pTextFormat);

private:
	BOOL advanceBlock(BOOL bDelayTransfer);
	void transferBlock();
	BOOL hasAdjacentBlocks(int direction);
	Block *getRandomBlock();

private:
	BOOL m_bPause;
	DWORD m_dwLastMoved, m_dwLastRotated, m_dwLastAdvanced;
	DWORD m_dwSpeed;
	DWORD m_dwPos, m_dwHeight;
	DWORD m_dwLines, m_dwScore, m_dwLevel;
	int *m_pBrickMatrix;

	std::vector<Block*> *m_pBlocks;
	Block *m_pCurrentBlock, *m_pNextBlock;
};
