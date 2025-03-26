package pl.edu.mimuw.simulationobjects;

import pl.edu.mimuw.utils.Logger;
import pl.edu.mimuw.datastructures.specific.Direction;
import pl.edu.mimuw.datastructures.specific.LineStop;
import pl.edu.mimuw.datastructures.general.EventsQueue;
import pl.edu.mimuw.datastructures.specific.Time;

import java.util.Scanner;

public class Line {
    private final static int startingHour = 6;
    private final int lineNumber;
    private final LineStop[] lineStops;
    private final Tram[] trams;

    public Line(int lineNumber, LineStop[] lineStops, int tramsNumber, int tramCapacity, EventsQueue eventsQueue, Logger simulationLogger) {
        this.lineNumber = lineNumber;
        this.lineStops = lineStops;
        trams = new Tram[tramsNumber];
        for (int i = 0; i < tramsNumber; i++) {
            trams[i] = new Tram(this, tramCapacity, eventsQueue, simulationLogger);
        }
    }

    public static Line[] readLinesFromInput(Scanner sc, int linesNumber, int tramCapacity, Stop[] stops, EventsQueue eventsQueue, Logger simulationLogger) {
        Line[] lines = new Line[linesNumber];

        for (int i = 0; i < linesNumber; i++) {
            int tramsNumber = sc.nextInt();
            int lineLength = sc.nextInt();
            LineStop[] lineStops = LineStop.lineStopsFromInput(sc, lineLength, stops);

            lines[i] = new Line(i, lineStops, tramsNumber, tramCapacity, eventsQueue, simulationLogger);
        }

        return lines;
    }

    public void startTrams(int simulationDay) {
        Time startTime = new Time(simulationDay, startingHour * Time.MINUTES_IN_HOUR);
        startForwardTrams(startTime);
        startBackwardTrams(startTime);
    }

    private void startForwardTrams(Time startTime) {
        int minutesOfDelay = 0;
        int delayBetweenTrams = delayBetweenTrams();
        for (int i = 0; i < trams.length; i += 2) {
            Time currentTime = new Time(startTime.getDay(), startTime.getMinute() + minutesOfDelay);
            trams[i].start(Direction.FORWARD, currentTime);
            minutesOfDelay += delayBetweenTrams;
        }
    }

    private void startBackwardTrams(Time startTime) {
        int minutesOfDelay = 0;
        for (int i = 1; i < trams.length; i += 2) {
            Time currentTime = new Time(startTime.getDay(), startTime.getMinute() + minutesOfDelay);
            trams[i].start(Direction.BACKWARD, currentTime);
            minutesOfDelay += delayBetweenTrams();
        }
    }

    public int getLineLength() {
        return lineStops.length;
    }

    public LineStop getLineStopAt(int i) {
        return lineStops[i];
    }

    public int minutesOfStopAtTheEnd() {
        return lineStops[lineStops.length - 1].travelTimeToNext();
    }

    private int totalTimeOfRoute() {
        int totalTime = 0;
        for (LineStop lineStop : lineStops) {
            totalTime += lineStop.travelTimeToNext();
        }
        return 2 * totalTime;
    }

    private int delayBetweenTrams() {
        return (int) totalTimeOfRoute() / trams.length;
    }

    public LineStop[] getLineStops() {
        return lineStops;
    }

    public int getLineNumber() {
        return lineNumber;
    }

    public String lineStopsDescriptionLog() {
        String log = "";
        for (int i = 0; i < lineStops.length; i++) {
            log = log + lineStops[i].stop().getName() + " " + lineStops[i].travelTimeToNext() + " ";
        }
        return log;
    }
}
