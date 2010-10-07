#pragma once

class Block
{
public:
	Block(int *structure, int w, int h);
	~Block();
	Block *clone();
	void rotate();

	int getWidth()
	{
		return m_width;
	}

	int getHeight()
	{
		return m_height;
	}

	int *getStructure()
	{
		return m_structure;
	}

private:
	int m_width, m_height;
	int *m_structure;
};
