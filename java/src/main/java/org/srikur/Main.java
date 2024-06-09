package org.srikur;

// Java 2D imports
import java.awt.*;
import java.awt.image.BufferedImage;
import javax.swing.*;

public class Main {

    private static CPU cpu;
    private static boolean quit = false;

    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Please provide a ROM path");
            return;
        }
        String romPath = args[0];
        if (romPath == null) {
            System.out.println("Please provide a ROM path");
            return;
        }
        boolean shiftQuirk = romPath.contains("INVADERS");
        cpu = new CPU(romPath, 540, shiftQuirk);

        // Create a window
        JFrame frame = new JFrame("Chip8 Emulator");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(640, 348);
        frame.setVisible(true);
        frame.setResizable(false);

        while (!quit) {
            // Execute a cycle
            cpu.emulateCycle();

            // If the draw flag is set, update the screen
            if (cpu.drawFlag) {
                // Create a new image
                BufferedImage image = new BufferedImage(64, 32, BufferedImage.TYPE_INT_RGB);
                Graphics2D g = image.createGraphics();
                g.setColor(Color.BLACK);
                g.fillRect(0, 0, 64, 32);
                g.setColor(Color.WHITE);
                int[] display = cpu.getScreen();
                for (int y = 0; y < 32; y++) {
                    for (int x = 0; x < 64; x++) {
                        if (display[y * 64 + x] == 1) {
                            g.fillRect(x, y, 1, 1);
                        }
                    }
                }
                g.dispose();

                // Update the window
                frame.getContentPane().getGraphics().drawImage(image, 0, 0, 640, 320, null);
                cpu.drawFlag = false;
            }
        }
    }
}