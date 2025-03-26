package cp2024.solution;

import cp2024.circuit.CircuitNode;
import cp2024.circuit.CircuitValue;
import cp2024.circuit.LeafNode;
import cp2024.circuit.ThresholdNode;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.Semaphore;

public class ParallelCircuitValue implements CircuitValue {
    protected final CircuitNode circuitNode;
    protected final int subTaskParentIndex;
    protected final ExecutorService workers;
    protected final Semaphore parentNotProcessedSubTasksSemaphore;
    protected final ArrayBlockingQueue<Integer> parentDoneTasksQueue;
    protected final Semaphore computedSemaphore;
    protected final Semaphore notProcessedTasksSempahore;  // Semaphore indicating how many tasks were done but still weren't processed by this object.
    protected volatile boolean circuitBooleanValue;
    protected boolean interrupted;
    protected volatile boolean alreadyComputed;
    protected ArrayBlockingQueue<Integer> doneTasks; // Queue holding indexes of done tasks.
    protected ArrayList<ParallelCircuitValue> tasks;
    protected CircuitNode[] childrenNodes;
    protected int numOfChildren;

    protected Future<?> taskManager;


    public ParallelCircuitValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                Semaphore parentNotProcessedSubTasksSemaphore, ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        this.circuitNode = circuitNode;
        this.subTaskParentIndex = subTaskParentIndex;
        this.workers = workers;
        this.parentNotProcessedSubTasksSemaphore = parentNotProcessedSubTasksSemaphore;
        this.parentDoneTasksQueue = parentDoneTasksQueue;
        this.interrupted = false;
        this.computedSemaphore = new Semaphore(0);
        this.notProcessedTasksSempahore = new Semaphore(0);
        this.alreadyComputed = false;
    }

    public static ParallelCircuitValue factory(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                               Semaphore parentNotProcessedSubTasksSemaphore, ArrayBlockingQueue<Integer> parentDoneTasksQueue) {
        return switch (circuitNode.getType()) {
            case LEAF ->
                    new ParallelCircuitLeafValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue, (LeafNode) circuitNode);
            case GT ->
                    new ParallelCircuitGTValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue, (ThresholdNode) circuitNode);
            case LT ->
                    new ParallelCircuitLTValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue, (ThresholdNode) circuitNode);
            case AND ->
                    new ParallelCircuitAndValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
            case OR ->
                    new ParallelCircuitOrValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
            case NOT ->
                    new ParallelCircuitNotValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
            case IF ->
                    new ParallelCircuitIfValue(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
            default -> throw new RuntimeException("Illegal type");
        };
    }

    private void initChildrenTasks() {
        for (int i = 0; i < numOfChildren; i++) {
            ParallelCircuitValue childTask = ParallelCircuitValue.factory(childrenNodes[i], i,
                    workers, notProcessedTasksSempahore, doneTasks);
            tasks.add(childTask);
        }
    }

    private void setChildrenFields() throws InterruptedException {
        this.childrenNodes = circuitNode.getArgs();
        this.numOfChildren = childrenNodes.length;
        if (numOfChildren > 0) {
            this.doneTasks = new ArrayBlockingQueue<>(numOfChildren);
        }
        this.tasks = new ArrayList<>();
    }

    protected void initComputations() {
        try {
            this.setChildrenFields();
            this.initChildrenTasks();

        } catch (InterruptedException e) {
            this.handleInterruptedExceptionDuringInit();
        }
    }

    protected void cancelChildrenTasks() {
        for (ParallelCircuitValue childTask : tasks) {
            childTask.taskManager.cancel(true);
        }
    }

    protected void endTask() {
        cancelChildrenTasks();
        computedSemaphore.release();
        parentDoneTasksQueue.add(subTaskParentIndex);

        parentNotProcessedSubTasksSemaphore.release();
    }

    protected void handleInterruptedExceptionDuringInit() {
        interrupted = true;
        computedSemaphore.release();
        endTask();
    }

    @Override
    public boolean getValue() throws InterruptedException {
        if (!alreadyComputed) {
            computedSemaphore.acquire();
        }
        alreadyComputed = true;
        if (interrupted) {
            throw new InterruptedException();
        }
        return circuitBooleanValue;
    }
}
