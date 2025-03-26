package pl.edu.mimuw.datastructures.specific;

import pl.edu.mimuw.utils.Losowanie;

public class Time {
    public static final int MINUTES_IN_HOUR = 60;
    private final int day;
    private final int minute;

    public Time(int day, int minute) {
        this.day = day;
        this.minute = minute;
    }

    public static int randomMinute(int lowerRangeHour, int upperRangeHour) {
        return Losowanie.losuj(lowerRangeHour * MINUTES_IN_HOUR, upperRangeHour * MINUTES_IN_HOUR);
    }

    public int getDay() {
        return day;
    }

    public int getMinute() {
        return minute;
    }

    public int minutesFrom(Time otherTime) {
        return minute - otherTime.getMinute();
    }

    private int getHour() {
        return minute / MINUTES_IN_HOUR;
    }

    private int getMinutesAfterWholeHour() {
        return minute % MINUTES_IN_HOUR;
    }

    public String logTimeInfoString() {
        return getDay() + ", " + standardizeTimeString(getHour()) + ":"
                + standardizeTimeString(getMinutesAfterWholeHour()) + ": ";
    }

    private String standardizeTimeString(int time) {
        if (time < 10) {
            return "0" + time;
        }
        return "" + time;
    }
}
