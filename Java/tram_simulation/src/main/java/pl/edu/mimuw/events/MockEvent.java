package pl.edu.mimuw.events;

import pl.edu.mimuw.datastructures.specific.Time;

public class MockEvent extends Event {

    public MockEvent(Time time) {
        super(time);
    }

    @Override
    public void executeEvent() {
        System.out.println("execute mock event");
    }
}
