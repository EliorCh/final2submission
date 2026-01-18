#pragma once
#include <vector>
#include "Point.h"
#include "GameDefs.h"

class Obstacle{
private: 

	std::vector<Point> body;
	char figure = '*';

 public:
     Obstacle() : body() {}      // default ctor
     explicit Obstacle(const std::vector<Point>& _body)  // custom ctor
         : body(_body) {}
	 
	// Get Functions
	std::vector<Point>& getBody() { return body; }    // Non-const & const access to obstacle body
	const std::vector<Point>& getBody() const { return body; }
	int getSize() const { return static_cast<int>(body.size()); }  // casting
	char getFigure() const { return figure; }

	bool isObBody(const Point& p) const {
		return std::find(body.begin(), body.end(), p) != body.end();
	}

	bool canBePushed(int force) const {
		return force >= getSize();
	}

	void move(Direction dir) {
		if (dir == Direction::STAY || dir == Direction::DISPOSE)
			return;

		for (Point &cell: body)
			cell = cell.next(dir);
	}

	std::vector<Point> getNextBody(Direction dir) const {
		std::vector<Point> result;
		result.reserve(body.size());

		for (const Point &cell: body)
			result.push_back(cell.next(dir));

		return result;
	}
};

