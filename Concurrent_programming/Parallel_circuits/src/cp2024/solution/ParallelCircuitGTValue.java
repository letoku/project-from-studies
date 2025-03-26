package cp2024.solution;

import cp2024.circuit.CircuitNode;
import cp2024.circuit.ThresholdNode;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Semaphore;

public class ParallelCircuitGTValue extends ParallelCircuitValue {
    private final ThresholdNode thresholdNode;

    public ParallelCircuitGTValue(CircuitNode circuitNode, int subTaskParentIndex, ExecutorService workers,
                                  Semaphore parentNotProcessedSubTasksSemaphore,
                                  ArrayBlockingQueue<Integer> parentDoneTasksQueue, ThresholdNode thresholdNode) {
        super(circuitNode, subTaskParentIndex, workers, parentNotProcessedSubTasksSemaphore, parentDoneTasksQueue);
        this.thresholdNode = thresholdNode;
        this.taskManager = workers.submit(new GTTaskManager(this));
    }

    private static class GTTaskManager extends TaskManager<ParallelCircuitGTValue> {
        private int tasksDone;
        private int tasksValuedTrue;
        private boolean done;

        public GTTaskManager(ParallelCircuitGTValue circuitAndValue) {
            super(circuitAndValue);
            tasksDone = 0;
            tasksValuedTrue = 0;
            done = false;
        }

        @Override
        public void run() {
            circuitValue.initComputations();
            if (!initCheckIfPossible()) {
                circuitValue.circuitBooleanValue = false;
                circuitValue.endTask();
                return;
            }

            while (tasksDone < circuitValue.numOfChildren && !done) {
                if (Thread.currentThread().isInterrupted()) {
                    handleInterruption();
                    return;
                }
                try {
                    circuitValue.notProcessedTasksSempahore.acquire();
                    int indexOfDoneTask = circuitValue.doneTasks.take();
                    boolean computedValue = circuitValue.tasks.get(indexOfDoneTask).getValue();
                    handleDoneTask(computedValue);

                } catch (InterruptedException e) {
                    handleInterruption();
                    return;
                }
            }
        }

        private boolean initCheckIfPossible() {
            return circuitValue.numOfChildren > circuitValue.thresholdNode.getThreshold();
        }

        private void handleDoneTask(boolean computedValue) {
            tasksDone++;
            if (computedValue) {
                tasksValuedTrue++;
                if (tasksValuedTrue > circuitValue.thresholdNode.getThreshold()) {
                    circuitValue.circuitBooleanValue = true;
                    done = true;
                    circuitValue.endTask();
                }

            } else {
                if (tasksValuedTrue + (circuitValue.numOfChildren - tasksDone) <= circuitValue.thresholdNode.getThreshold()) {
                    circuitValue.circuitBooleanValue = false;
                    done = true;
                    circuitValue.endTask();
                }
            }
        }
    }
}