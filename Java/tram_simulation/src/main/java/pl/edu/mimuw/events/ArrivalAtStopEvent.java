package pl.edu.mimuw.events;

import pl.edu.mimuw.datastructures.specific.Time;
import pl.edu.mimuw.simulationobjects.Stop;
import pl.edu.mimuw.simulationobjects.Tram;

public class ArrivalAtStopEvent extends Event {
    private final Stop stop;
    private final boolean endOfRoute;
    private final Tram tram;

    public ArrivalAtStopEvent(Time time, Stop stop, boolean endOfRoute, Tram tram) {
        super(time);
        this.stop = stop;
        this.endOfRoute = endOfRoute;
        this.tram = tram;
    }

    @Override
    public void executeEvent() {
        tram.arriveAtStop(stop, endOfRoute, time);
    }
}
