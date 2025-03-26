package pl.edu.mimuw.datastructures.general;

import pl.edu.mimuw.datastructures.specific.Time;
import pl.edu.mimuw.events.Event;
import pl.edu.mimuw.events.MockEvent;

public class HeapEventsQueue implements EventsQueue {
    private Vector<Event> events;

    public HeapEventsQueue() {
        events = new Vector<>();
        addMockEvent();
    }


    /**
     * Adds mock event at index 0, so that real elements will be numerated from 1(this simplifies implementation of pq)
     */
    private void addMockEvent() {
        events.add(new MockEvent(new Time(0, 0)));
    }

    @Override
    public void add(Event event) {
        events.add(event);
        bubbleUp(events.indexOfLastElement());
    }

    /**
     * Bubbles up element of given index preserving order. If element is smaller than its parent then it is bubbled up
     * and recursively bubbled up then. Otherwise, it means element has found its place and bubbling up ends here
     *
     * @param index of element to bubble up
     */
    private void bubbleUp(int index) {
        if (index == 1) return;
        int parentIndex = index / 2;
        if (events.getAtIndex(index).compareTo(events.getAtIndex(parentIndex)) < 0) {
            events.swap(index, parentIndex);
            bubbleUp(parentIndex);
        }
    }

    /**
     * Similar function for bubbling down. Always bubbles down to the smaller child up to the moment when element being
     * bubbled down is not greater than its children.
     *
     * @param index of element to bubble down
     */
    private void bubbleDown(int index) {
        int leftChildIndex = 2 * index;
        int rightChildIndex = 2 * index + 1;
        if (leftChildIndex > events.indexOfLastElement()) return;

        if (leftChildIndex == events.indexOfLastElement()) {
            compareAndBubbleIfGreater(index, leftChildIndex);
            return;
        }

        int smallerChildIndex = smallerElementIndex(leftChildIndex, rightChildIndex);
        compareAndBubbleIfGreater(index, smallerChildIndex);
    }

    private void compareAndBubbleIfGreater(int indexOfBubblingElement, int indexOfChild) {
        if (events.getAtIndex(indexOfBubblingElement).compareTo(events.getAtIndex(indexOfChild)) > 0) {
            events.swap(indexOfBubblingElement, indexOfChild);
            bubbleDown(indexOfChild);
        }
    }

    private int smallerElementIndex(int index1, int index2) {
        if (events.getAtIndex(index1).compareTo(events.getAtIndex(index2)) < 0) {
            return index1;
        } else {
            return index2;
        }
    }

    @Override
    public Event top() {
        return events.getAtIndex(1);
    }

    @Override
    public Event pop() {
        Event topEvent = top();
        events.setAtIndex(1, events.getLastElement());
        events.removeLast();
        bubbleDown(1);

        return topEvent;
    }

    @Override
    public void clear() {
        events.clear();
        addMockEvent();
    }

    @Override
    public boolean isEmpty() {
        return events.indexOfLastElement() < 1;
    }
}
