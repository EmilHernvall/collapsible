#include "stdafx.h"
#include "collapsible.h"
#include "block.h"

Block::Block(int *structure, int w, int h)
: m_width(w), m_height(h), m_structure(NULL)
{
	m_structure = new int[w*h];
	memcpy(m_structure, structure, w*h*sizeof(int));
}

Block::~Block()
{
	delete[] m_structure;
}

Block *Block::clone()
{
	return new Block(m_structure, m_width, m_height);
}

void Block::rotate()
{
	int newWidth, newHeight;
	int *newStructure;

	newWidth = m_height;
	newHeight = m_width;

	newStructure = new int[newWidth * newHeight];

	/*for (int x = 0; x < m_width; x++) {
		for (int y = 0; y < m_height; y++) {
			int newX, newY;
			newX = newWidth - y - 1;
			newY = x;

			int src = y * m_width + x;
			int dst = newY * newWidth + newX;
			newStructure[dst] = m_structure[src];
		}
	}*/

	for (int x = 0; x < m_width; x++) {
		for (int y = 0; y < m_height; y++) {
			int newX, newY;
			newX = y;
			newY = newHeight - x - 1;

			int src = y * m_width + x;
			int dst = newY * newWidth + newX;
			newStructure[dst] = m_structure[src];
		}
	}

	delete[] m_structure;

	m_structure = newStructure;
	m_width = newWidth;
	m_height = newHeight;
}
