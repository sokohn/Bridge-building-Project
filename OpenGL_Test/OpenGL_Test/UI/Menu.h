#include "UI_Base.h"
#include <vector>

class MENU
{
public:
	MENU();
	~MENU();

	void Draw();
	void Update();
	void AddUIElement(UI_BASE* Element);
	void Clear();
protected:
	std::vector<UI_BASE*> Elements;
};