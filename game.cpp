#include "stdafx.h"
#include "collapsible.h"
#include "game.h"
#include "block.h"

Game::Game()
	: m_pCurrentBlock(NULL), m_pNextBlock(NULL), m_dwLastMoved(0), 
	  m_dwLastRotated(0), m_dwPos(0), m_dwHeight(0), m_dwSpeed(1000), 
	  m_dwLastAdvanced(0), m_dwLines(0), m_dwScore(0), m_dwLevel(1),
	  m_bPause(FALSE)
{
	m_pBrickMatrix = new int[MATRIX_HEIGHT*MATRIX_WIDTH];
	memset(m_pBrickMatrix, 0, MATRIX_HEIGHT*MATRIX_WIDTH*sizeof(int));

	m_pBlocks = new std::vector<Block*>();

	{
		// Declare square block
		int blockDef[4] = {
			1,1,
			1,1
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 2));
	}

	{
		// Declare straight block
		int blockDef[4] = {
			1,1,1,1
		};

		m_pBlocks->push_back(new Block(blockDef, 4, 1));
	}

	{
		// Declare L block
		int blockDef[6] = {
			1,0,
			1,0,
			1,1
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 3));
	}

	{
		// Declare mirrored L block
		int blockDef[6] = {
			0,1,
			0,1,
			1,1
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 3));
	}

	{
		// Declare S block
		int blockDef[6] = {
			1,0,
			1,1,
			0,1
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 3));
	}

	{
		// Declare mirrored S block
		int blockDef[6] = {
			0,1,
			1,1,
			1,0
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 3));
	}

	{
		// Declare T block
		int blockDef[6] = {
			1,0,
			1,1,
			1,0
		};

		m_pBlocks->push_back(new Block(blockDef, 2, 3));
	}
}

Game::~Game()
{
	delete m_pBlocks;
}

void Game::togglePause()
{
	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastMoved) < 100) {
		return;
	}

	m_bPause = !m_bPause;

	m_dwLastMoved = dwCurrentTick;
}

BOOL Game::hasAdjacentBlocks(int direction)
{
	int blockWidth = m_pCurrentBlock->getWidth();
	int blockHeight = m_pCurrentBlock->getHeight();
	int *blockStructure = m_pCurrentBlock->getStructure();

	BOOL adjacent = FALSE;
	for (int x = 0; x < blockWidth; x++) {
		for (int y = 0; y < blockHeight; y++) {
			if (!blockStructure[blockWidth * y + x]) {
				continue;
			}

			int matrixX = m_dwPos + x + direction;
			int matrixY = m_dwHeight + y;
			if (m_pBrickMatrix[matrixY * MATRIX_WIDTH + matrixX]) {
				adjacent = TRUE;
				break;
			}
		}

		if (adjacent) {
			break;
		}
	}

	return adjacent;
}

void Game::moveLeft()
{
	if (m_bPause) {
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastMoved) < 100) {
		return;
	}

	// Make sure we're not always as far to the left as possible
	if (m_dwPos == 0) {
		return;
	}

	// Abort if no block is loaded
	if (!m_pCurrentBlock) {
		return;
	}

	// Check if there's a block to left blocking our move
	if (hasAdjacentBlocks(-1)) {
		return;
	}
	
	// Perform the move
	m_dwPos--;

	m_dwLastMoved = dwCurrentTick;
}

void Game::moveRight()
{
	if (m_bPause) {
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastMoved) < 100) {
		return;
	}

	if (!m_pCurrentBlock) {
		return;
	}

	if (m_dwPos + m_pCurrentBlock->getWidth() >= 10) {
		return;
	}

	if (hasAdjacentBlocks(1)) {
		return;
	}

	m_dwPos++;

	m_dwLastMoved = dwCurrentTick;
}

void Game::rotate()
{
	if (m_bPause) {
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastRotated) < 200) {
		return;
	}

	if (!m_pCurrentBlock) {
		return;
	}

	/*if (m_dwHeight + m_pCurrentBlock->getHeight() > MATRIX_HEIGHT) {
		return;
	}*/

	if (hasAdjacentBlocks(-1) || hasAdjacentBlocks(1)) {
		return;
	}

	if (m_pCurrentBlock) {
		m_pCurrentBlock->rotate();
	}

	while (m_dwPos + m_pCurrentBlock->getWidth() > MATRIX_WIDTH) {
		m_dwPos--;
	}

	m_dwLastRotated = dwCurrentTick;
}

void Game::drop()
{
	if (m_bPause) {
		return;
	}

	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastMoved) < 200) {
		return;
	}

	// advance the block until we hit an obstacle
	while (!advanceBlock(TRUE));

	m_dwLastAdvanced = dwCurrentTick - m_dwSpeed / 2;
	m_dwLastMoved = dwCurrentTick;
}

Block *Game::getRandomBlock()
{
	srand(GetTickCount());
	int blockNum = rand() % m_pBlocks->size();
	return (*m_pBlocks)[blockNum]->clone();
}

BOOL Game::tick()
{
	if (m_bPause) {
		return FALSE;
	}

	if (m_pNextBlock == NULL) {
		m_pNextBlock = getRandomBlock();
	}

	if (m_pCurrentBlock == NULL) {
		m_pCurrentBlock = m_pNextBlock;
		m_pNextBlock = getRandomBlock();

		m_dwPos = (10 - m_pCurrentBlock->getWidth()) / 2;

		return TRUE;
	}

	DWORD dwCurrentTick = GetTickCount();
	if ((dwCurrentTick - m_dwLastAdvanced) > m_dwSpeed) {
		advanceBlock(FALSE);
		m_dwLastAdvanced = dwCurrentTick;
		return TRUE;
	}

	return FALSE;
}

BOOL Game::advanceBlock(BOOL bDelayTransfer)
{
	if (!m_pCurrentBlock) {
		return FALSE;
	}

	int blockWidth = m_pCurrentBlock->getWidth();
	int blockHeight = m_pCurrentBlock->getHeight();
	int *blockStructure = m_pCurrentBlock->getStructure();

	// Hit the bottom
	if (m_dwHeight + blockHeight == MATRIX_HEIGHT) {
		if (!bDelayTransfer) {
			transferBlock();
		}
		return TRUE;
	}

	// Check for other bricks on the next row
	for (int x = 0; x < blockWidth; x++) {
		for (int y = 0; y < blockHeight; y++) {
			if (!blockStructure[blockWidth * y + x]) {
				continue;
			}

			int matrixX = m_dwPos + x;
			int matrixY = m_dwHeight + y + 1;
			if (m_pBrickMatrix[matrixY * MATRIX_WIDTH + matrixX]) {
				if (!bDelayTransfer) {
					transferBlock();
				}
				return TRUE;
			}
		}
	}

	m_dwHeight++;
	return FALSE;
}

void Game::transferBlock()
{
	if (m_dwHeight == 0) {
		MessageBox(NULL, L"Game over!", L"Collapsible", 0);
		exit(0);
	}

	int blockWidth = m_pCurrentBlock->getWidth();
	int blockHeight = m_pCurrentBlock->getHeight();
	int *blockStructure = m_pCurrentBlock->getStructure();

	for (int x = 0; x < blockWidth; x++) {
		for (int y = 0; y < blockHeight; y++) {
			if (!blockStructure[blockWidth * y + x]) {
				continue;
			}

			int matrixX = m_dwPos + x;
			int matrixY = m_dwHeight + y;

			m_pBrickMatrix[matrixY * MATRIX_WIDTH + matrixX] = 1;
		}
	}

	int *pNewMatrix = new int[MATRIX_HEIGHT * MATRIX_WIDTH];
	memset(pNewMatrix, 0, MATRIX_HEIGHT*MATRIX_WIDTH*sizeof(int));

	int targetRow = MATRIX_HEIGHT - 1;
	int eliminatedLines = 0;
	for (int y = MATRIX_HEIGHT - 1; y >= 0; y--) {
		BOOL bFullRow = TRUE, bEmptyRow = TRUE;
		for (int x = 0; x < MATRIX_WIDTH; x++) {
			if (!m_pBrickMatrix[y * MATRIX_WIDTH + x]) {
				bFullRow = FALSE;
			} else {
				bEmptyRow = FALSE;
			}

			pNewMatrix[targetRow * MATRIX_WIDTH + x] = m_pBrickMatrix[y * MATRIX_WIDTH + x];
		}

		if (bFullRow) {
			eliminatedLines++;
			continue;
		}
		else if (bEmptyRow) {
			break;
		}

		targetRow--;
	}

	if (eliminatedLines > 0) {
		m_dwLines += eliminatedLines;
		int newScore = 1000 * static_cast<int>(pow(2.0, eliminatedLines));
		int breakOffPoint = 25000;
		if ((m_dwScore + newScore) % breakOffPoint < m_dwScore % breakOffPoint) {
			m_dwLevel++;
			m_dwSpeed = 1000 / m_dwLevel;
		}
		m_dwScore += newScore;
	}

	delete[] m_pBrickMatrix;

	m_pBrickMatrix = pNewMatrix;

	m_dwHeight = 0;
	m_pCurrentBlock = NULL;
}

void Game::draw(ID2D1HwndRenderTarget *pRenderTarget, int logoHeight, ID2D1Bitmap *pBrickImage, IDWriteTextFormat *pTextFormat)
{
	HRESULT hr;

	ID2D1SolidColorBrush *pDefBrush;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &pDefBrush);
	if (FAILED(hr)) {
		return;
	}

	ID2D1SolidColorBrush *pGameAreaBrush;
    hr = pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black, 0.25),
        &pGameAreaBrush);
	if (FAILED(hr)) {
		return;
	}

    D2D1_SIZE_F rtSize = pRenderTarget->GetSize();

    // Draw a background
    int width = static_cast<int>(rtSize.width);
    int height = static_cast<int>(rtSize.height);

	int brickSize = height / 20;
	int gameAreaWidth = brickSize  * 10;

	D2D1_RECT_F gameAreaRect = D2D1::RectF(
		static_cast<float>((width - gameAreaWidth) / 2), 
		static_cast<float>(0),
		static_cast<float>((width + gameAreaWidth) / 2), 
		static_cast<float>(height));

	pRenderTarget->FillRectangle(gameAreaRect, pGameAreaBrush);

	// Statistics
	D2D1_RECT_F textRect = D2D1::RectF(
		static_cast<float>(10), 
		static_cast<float>(logoHeight + 10),
		static_cast<float>((width - gameAreaWidth) / 2 - 10), 
		static_cast<float>(height - 10));

	std::wstringstream statsStream;
	statsStream
		<< L"Score: " << m_dwScore << L"\r\n"
		<< L"Lines: " << m_dwLines << L"\r\n"
		<< L"Level: " << m_dwLevel << L"\r\n";

	if (m_bPause) {
		statsStream << "\r\n" << "\r\n" << "Paused";
	}

	std::wstring statsStr = statsStream.str();

	pRenderTarget->DrawTextW(statsStr.c_str(), statsStr.length(), pTextFormat, textRect, pDefBrush);

	// Draw the current block
	if (m_pCurrentBlock) {
		int blockWidth = m_pCurrentBlock->getWidth();
		int blockHeight = m_pCurrentBlock->getHeight();
		int *blockStructure = m_pCurrentBlock->getStructure();
		int baseX = (width - gameAreaWidth) / 2 + m_dwPos * brickSize;
		int baseY = m_dwHeight * brickSize;

		for (int x = 0; x < blockWidth; x++) {
			for (int y = 0; y < blockHeight; y++) {
				if (!blockStructure[blockWidth * y + x]) {
					continue;
				}

				int brickX = baseX + x * brickSize;
				int brickY = baseY + y * brickSize;

				D2D1_RECT_F brickRect = D2D1::RectF(
					static_cast<float>(brickX), 
					static_cast<float>(brickY),
					static_cast<float>(brickX + brickSize), 
					static_cast<float>(brickY + brickSize));

				pRenderTarget->DrawBitmap(pBrickImage, &brickRect);
			}
		}
	}

	// Draw next block
	if (m_pNextBlock) {
		int blockWidth = m_pNextBlock->getWidth();
		int blockHeight = m_pNextBlock->getHeight();
		int *blockStructure = m_pNextBlock->getStructure();
		int baseX = (width + gameAreaWidth) / 2 + 20;
		int baseY = 20;

		for (int x = 0; x < blockWidth; x++) {
			for (int y = 0; y < blockHeight; y++) {
				if (!blockStructure[blockWidth * y + x]) {
					continue;
				}

				int brickX = baseX + x * brickSize;
				int brickY = baseY + y * brickSize;

				D2D1_RECT_F brickRect = D2D1::RectF(
					static_cast<float>(brickX), 
					static_cast<float>(brickY),
					static_cast<float>(brickX + brickSize), 
					static_cast<float>(brickY + brickSize));

				//pRenderTarget->DrawRectangle(&brickRect, pDefBrush);
				pRenderTarget->DrawBitmap(pBrickImage, &brickRect);
			}
		}
	}

	// Draw brick matrix
	int baseX = (width - gameAreaWidth) / 2;
	int baseY = 0;
	for (int x = 0; x < MATRIX_WIDTH; x++) {
		for (int y = 0; y < MATRIX_HEIGHT; y++) {
			if (!m_pBrickMatrix[y * MATRIX_WIDTH + x]) {
				continue;
			}

			int brickX = baseX + x * brickSize;
			int brickY = baseY + y * brickSize;

			D2D1_RECT_F brickRect = D2D1::RectF(
				static_cast<float>(brickX), 
				static_cast<float>(brickY),
				static_cast<float>(brickX + brickSize), 
				static_cast<float>(brickY + brickSize));

			//pRenderTarget->FillRectangle(&brickRect, pDefBrush);
			pRenderTarget->DrawBitmap(pBrickImage, &brickRect, 0.9);
		}
	}

	SafeRelease(&pDefBrush);
	SafeRelease(&pGameAreaBrush);
}
