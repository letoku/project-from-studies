package pl.edu.mimuw.datastructures.general;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import pl.edu.mimuw.datastructures.general.HeapEventsQueue;
import pl.edu.mimuw.datastructures.specific.Time;
import pl.edu.mimuw.events.Event;
import pl.edu.mimuw.events.MockEvent;
import pl.edu.mimuw.utils.Losowanie;

import java.util.Arrays;


public class HeapEventsQueueTest {
    @Test
    void shouldSortEventsByTime() {
        //given
        int n = 1000;
        int[] times = new int[n];
        Arrays.setAll(times, i -> Losowanie.losuj(360, 720));
        Event[] events = new MockEvent[n];
        Event[] heapSortedEvents = new MockEvent[n];
        for (int i = 0; i < n; i++) {
            events[i] = new MockEvent(new Time(0, times[i]));
        }

        HeapEventsQueue queue = new HeapEventsQueue();
        // when
        for (Event event : events) {
            queue.add(event);
        }

        for (int i = 0; i < n; i++) {
            heapSortedEvents[i] = queue.pop();
        }

        //then
        Arrays.sort(times);
        for (int i = 0; i < n; i++) {
            Assertions.assertEquals(times[i], heapSortedEvents[i].getTimeMinute());
        }
    }
}
