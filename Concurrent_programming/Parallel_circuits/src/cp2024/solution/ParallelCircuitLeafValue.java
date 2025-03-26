package cp2024.solution;

import cp2024.circuit.CircuitNode;
import cp2024.circuit.LeafNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitLeafValue extends ParallelCircuitValue {
    private final LeafNode leafNode;

    public ParallelCircuitLeafValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                    Semaphore parentNotProcessedSubTasksSemaphore,
                                    ArrayBlockingQueue<Integer> parentDoneTasksQueue,
                                    LeafNode leafNode) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        this.leafNode = leafNode;
        this.taskManager = workers.submit(new LeafTaskManager(this));
    }

    private static class LeafTaskManager extends TaskManager<ParallelCircuitLeafValue> {

        public LeafTaskManager(ParallelCircuitLeafValue circuitValue) {
            super(circuitValue);
        }

        @Override
        public void run() {
            circuitValue.initComputations();
            try {
                circuitValue.circuitBooleanValue = circuitValue.leafNode.getValue();
                circuitValue.endTask();
            } catch (InterruptedException e) {
                handleInterruption();
            }
        }
    }
}
