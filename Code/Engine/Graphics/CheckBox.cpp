#include "Checkbox.h"

eae6320::Graphics::Checkbox::Checkbox()
{

}

eae6320::Graphics::Checkbox::Checkbox(char * i_text, int16_t i_x, int16_t i_y)
{
	Text.text = new Graphics::cText(i_text, i_x, i_y);
	Text.material.Load("data/materials/sprite.material");
}


eae6320::Graphics::Checkbox::~Checkbox()
{
	Text.text->CleanUp();
	Text.material.CleanUp();
}