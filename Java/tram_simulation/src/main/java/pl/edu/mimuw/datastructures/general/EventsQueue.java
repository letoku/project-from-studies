package pl.edu.mimuw.datastructures.general;

import pl.edu.mimuw.events.Event;

public interface EventsQueue {
    void add(Event event);

    Event top();

    Event pop();

    void clear();

    boolean isEmpty();

}
