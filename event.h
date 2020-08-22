#ifndef EVENT_H
#define EVENT_H

#include "math.h"

class Event
{
public:
    Event(double x, double y, int index);
    double GetDistance(Event eventPoint); // Computes the euclidean distance between this event and the event "eventPoint"
    double GetX(); // Get x coordinate
    double GetY(); // Get y coordinate
    int GetIndex(); // Get index
private:
    double x; double y;
    int index;
};

#endif // EVENT_H
