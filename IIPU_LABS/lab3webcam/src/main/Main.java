package main;

import main.controller.WebcamController;
import main.utils.Utils;

import java.awt.*;
import java.awt.event.*;
import java.net.URL;
import java.util.Objects;
import java.util.Scanner;
import javax.swing.*;

public class Main {
    private static final String APP_NAME = "Webcam app";
    private static final int FPS = 50;
    private static WebcamController webcamController;

    public static void main(String[] args) {
        webcamController = new WebcamController();
        /* Use an appropriate Look and Feel */
        try {
            UIManager.setLookAndFeel("com.sun.java.swing.plaf.gtk.GTKLookAndFeel");
        } catch (Exception exception) {
            exception.printStackTrace();
        }

        UIManager.put("swing.boldMetal", Boolean.FALSE);
        //Schedule a job for the event-dispatching thread:
        //adding TrayIcon.
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                createAndShowGUI();
            }
        });

        Thread thread = new Thread(){
            public void run(){
                Scanner scanner = new Scanner(System.in);
                String ch = "";
                System.out.println("Enter value:\nQUIT\nPHOTO\nVIDEO\nINFO\n");
                while(ch != "QUIT"){
                    ch = scanner.nextLine();
                    switch (ch) {
                        case "PHOTO":
                            try {
                                webcamController.takePhoto();
                            } catch (Exception exception) {
                                System.out.println("PHOTO EXCEPTION");
                            }
                            break;
                        case "VIDEO" :
                            System.out.println("enter duration video ");
                            int duration = scanner.nextInt();

                            try {
                                webcamController.recordVideo(duration, FPS);
                            } catch (Exception exception) {
                                System.out.println("VIDEO EXCEPTION");
                            }
                            break;

                        case "INFO":
                            System.out.println("Webcam: " + webcamController.getName());
                            break;
                        case "QUIT":
                            ch = "QUIT";
                            break;
                        default:
                           System.out.println("Wrong input, try again");
                            break;
                    }
                }
                System.out.println("Killing programm...");
                System.exit(0);
            }
        };
        thread.start();
    }

    private static void createAndShowGUI() {
        //Check the SystemTray support
        if (!SystemTray.isSupported()) {
            System.out.println("SystemTray is not supported");
            return;
        }
        final PopupMenu popup = new PopupMenu();
        final TrayIcon trayIcon =
                new TrayIcon(Objects.requireNonNull(createImage("icon.gif", "tray icon")));
        final SystemTray tray = SystemTray.getSystemTray();

        // Create a popup menu components
        MenuItem aboutItem = new MenuItem("About");
        MenuItem photoItem = new MenuItem("Take a photo");
        MenuItem videoItem = new MenuItem("Record a video");
        MenuItem exitItem = new MenuItem("Exit");


        //Add components to popup menu
        popup.add(aboutItem);
        popup.addSeparator();
        popup.add(photoItem);
        popup.add(videoItem);
        popup.add(exitItem);

        trayIcon.setPopupMenu(popup);
        trayIcon.setImageAutoSize(true);

        try {
            tray.add(trayIcon);
        } catch (AWTException e) {
            System.out.println("TrayIcon could not be added.");
            return;
        }

        trayIcon.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null,
                        "This dialog box is run from System Tray");
            }
        });

        aboutItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JOptionPane.showMessageDialog(null,
                        "Webcam: " + webcamController.getName());
            }
        });

        photoItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                MenuItem item = (MenuItem)e.getSource();

                System.out.println(item.getLabel());

                try {
                    webcamController.takePhoto();
                } catch (Exception exception) {
                    Utils.showExceptionDialog(exception);
                }


            }
        });

        videoItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                MenuItem item = (MenuItem)e.getSource();

                System.out.println(item.getLabel());

                int duration = showDurationInputDialog(trayIcon);

                try {
                    webcamController.recordVideo(duration, FPS);
                } catch (Exception exception) {
                    Utils.showExceptionDialog(exception);
                }


            }
        });

        exitItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                trayIcon.displayMessage(APP_NAME,
                        "Webcam app was shut down", TrayIcon.MessageType.INFO);

                tray.remove(trayIcon);
                System.exit(0);
            }
        });


    }

    private static int showDurationInputDialog(TrayIcon trayIcon)
    {
        int result;

        String inputValue = JOptionPane.showInputDialog("Please input required video duration");

        try {
            result = Integer.parseUnsignedInt(inputValue);
        } catch (Exception exception){
            trayIcon.displayMessage(APP_NAME,
                    "The value of param duration is invalid!", TrayIcon.MessageType.ERROR);
            result = showDurationInputDialog(trayIcon);
        }

        return result;
    }

    //Obtain the image URL
    private static Image createImage(String path, String description) {
        URL imageURL = Main.class.getClassLoader().getResource(path);

        if (imageURL == null) {
            System.err.println("Resource not found: " + path);
            return null;
        } else {
            return (new ImageIcon(imageURL, description)).getImage();
        }
    }
}
