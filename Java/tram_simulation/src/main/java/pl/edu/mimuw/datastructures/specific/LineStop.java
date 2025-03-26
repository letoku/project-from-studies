package pl.edu.mimuw.datastructures.specific;

import pl.edu.mimuw.simulationobjects.Stop;

import java.util.Scanner;

public record LineStop(Stop stop, int travelTimeToNext) {
    public LineStop(Stop stop, int travelTimeToNext) {
        this.stop = stop;
        this.travelTimeToNext = travelTimeToNext;
    }

    // Static factory method
    public static LineStop lineStopPointFromName(String stopName, int travelTimeToNext, Stop[] stops) {
        for (Stop stop : stops) {
            if (stopName.equals(stop.getName())) {
                return new LineStop(stop, travelTimeToNext);
            }
        }
        throw new IllegalArgumentException("No stop with given name exists");
    }

    public static LineStop[] lineStopsFromInput(Scanner sc, int lineLength, Stop[] stops) {
        LineStop[] lineStops = new LineStop[lineLength];
        for (int j = 0; j < lineLength; j++) {
            String stopName = sc.next();
            int travelTime = sc.nextInt();
            lineStops[j] = LineStop.lineStopPointFromName(stopName, travelTime, stops);
        }
        return lineStops;
    }
}
