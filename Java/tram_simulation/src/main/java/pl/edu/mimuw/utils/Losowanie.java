package pl.edu.mimuw.utils;

import java.util.Random;

public class Losowanie {
    private static final Random random = new Random(1);

    public static int losuj(int dolna, int gorna) {
        return random.nextInt(gorna - dolna + 1) + dolna;
    }
}
