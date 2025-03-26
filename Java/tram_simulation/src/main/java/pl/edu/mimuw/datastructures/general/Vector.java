package pl.edu.mimuw.datastructures.general;

public class Vector<T> {
    private T[] array;
    private int size;
    private static final int DEFAULT_CAPACITY = 10;

    public Vector() {
        array = (T[]) new Object[DEFAULT_CAPACITY];
        size = 0;
    }

    public void add(T element) {
        if (size == array.length) {
            resize();
        }
        array[size++] = element;
    }

    public T getAtIndex(int index) {
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Index " + index + " out of bounds for size " + size);
        }
        return array[index];
    }

    public int size() {
        return size;
    }

    private void resize() {
        int newSize = array.length * 2;
        T[] newArray = (T[]) new Object[newSize];
        System.arraycopy(array, 0, newArray, 0, size);
        array = newArray;
    }

    public void concatenate(Vector<T> other) {
        int newSize = this.size + other.size;
        if (newSize > array.length) {
            int newCapacity = 2 * newSize;
            T[] newArray = (T[]) new Object[newCapacity];
            System.arraycopy(array, 0, newArray, 0, this.size);
            array = newArray;
        }
        System.arraycopy(other.array, 0, array, this.size, other.size);
        size = newSize;
    }

    public void clear() {
        array = (T[]) new Object[DEFAULT_CAPACITY];
        size = 0;
    }

    public void removeLast() {
        size--;
    }

    public void setAtIndex(int index, T obj) {
        array[index] = obj;
    }

    public T getLastElement() {
        return array[size - 1];
    }

    public int indexOfLastElement() {
        return size - 1;
    }

    public void swap(int i, int j) {
        T temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

