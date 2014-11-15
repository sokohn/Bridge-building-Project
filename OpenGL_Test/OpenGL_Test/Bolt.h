
struct Color;

class Bolt
{
public:
	float x;
	float y;
	bool Highlighted;

	static const int collisionRadius = 10;

	Bolt(float _x, float _y)
	{
		x = _x;
		y = _y;
		Highlighted = false;
	}

	float GetDrawX();
	float GetDrawY();
	virtual void GetDrawColor( Color* color );
};

class Anchor : public Bolt
{
public:
	Anchor(float _x, float _y) : Bolt(_x, _y)
	{
	}
	void GetDrawColor( Color* color );
};