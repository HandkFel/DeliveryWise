#include "event.h"

// Constructor: x coordinate, y coordinate and index of event
Event::Event(double x, double y, int index):
    x(x),
    y(y),
    index(index)
{

}

// Computes the euclidean distance between this event and the event "eventPoint"
double Event::GetDistance(Event eventPoint){
    return sqrt((x-eventPoint.GetX())*(x-eventPoint.GetX()) +
                (y-eventPoint.GetY())*(y-eventPoint.GetY()));
}

// Get x coordinate of event
double Event::GetX(){
    return x;
}

// Get y coordinate of event
double Event::GetY(){
    return y;
}

// Get index of event
int Event::GetIndex(){
    return index;
}
