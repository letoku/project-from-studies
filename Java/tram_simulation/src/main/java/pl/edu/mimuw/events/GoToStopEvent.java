package pl.edu.mimuw.events;

import pl.edu.mimuw.datastructures.specific.Time;
import pl.edu.mimuw.simulationobjects.Passenger;

public class GoToStopEvent extends Event {
    private Passenger passenger;

    public GoToStopEvent(Time time, Passenger passenger) {
        super(time);
        this.passenger = passenger;
    }

    @Override
    public void executeEvent() {
        passenger.goToStop(time);
    }
}
