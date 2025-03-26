package pl.edu.mimuw.events;

import pl.edu.mimuw.datastructures.specific.Time;

abstract public class Event implements Comparable<Event> {
    protected Time time;

    public Event(Time time) {
        this.time = time;
    }

    public int getTimeMinute() {
        return time.getMinute();
    }

    abstract public void executeEvent();

    @Override
    public int compareTo(Event other) {
        int diff = time.getMinute() - other.getTimeMinute();
        if (diff < 0) return -1;
        if (diff == 0) return 0;
        return 1;
    }
}
