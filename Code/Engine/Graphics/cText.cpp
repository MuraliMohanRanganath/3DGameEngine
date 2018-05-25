#include "cText.h"

eae6320::Graphics::cText::sFont* eae6320::Graphics::cText::m_Font = new cText::sFont[95];

eae6320::Graphics::cText::cText(char * text, int x, int y)
{
	m_text = text;
	m_x = x;
	m_y = y;
}

bool eae6320::Graphics::cText::LoadFontData(char* filename)
{
	std::ifstream inputStream;

	if (!m_Font)
	{
		return false;
	}

	inputStream.open(filename);
	if (inputStream.fail())
	{
		return false;
	}

	char temp;
	for (int i = 0; i < 95; i++)
	{
		inputStream.get(temp);
		while (temp != ' ')
		{
			inputStream.get(temp);
		}
		inputStream.get(temp);
		while (temp != ' ')
		{
			inputStream.get(temp);
		}

		inputStream >> m_Font[i].left;
		inputStream >> m_Font[i].right;
		inputStream >> m_Font[i].size;
	}
	inputStream.close();
	return true;
}