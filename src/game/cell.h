#pragma once

enum class eCellType { 
	EMPTY = 0, 
	WALL_ALL, 
	WALL_HORIZONTAL, 
	WALL_VERTICAL, 
	WALL_TOP_LEFT, 
	WALL_TOP_RIGHT, 
	WALL_BOTTOM_LEFT, 
	WALL_BOTTOM_RIGHT, 
	FRUIT0, 
	FRUIT1, 
	FRUIT2 
};

class Cell
{
public:
	Cell();
	~Cell();
	eCellType cellType;

private:

};