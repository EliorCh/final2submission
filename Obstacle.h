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

     bool isObBody(const Point& p) const;
     bool canBePushed(int force) const;

     void move(Direction dir);
     std::vector<Point> getNextBody(Direction dir) const;


};

