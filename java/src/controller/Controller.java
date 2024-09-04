package controller;

import javax.swing.SwingUtilities;
import javax.swing.SwingWorker;
import java.util.concurrent.ExecutionException;
import jni.Bridge;
import view.MainFrame;

public class Controller {

    private static MainFrame mainFrame;

    public void setMainFrame(MainFrame mainFrame) {
        Controller.mainFrame = mainFrame;
    }

    public void swithPanel(String panelName) {
        mainFrame.swithPanel(panelName);
    }

    public void startChatRoom(String username) {
        new SwingWorker<Integer, Void>() {
            @Override
            protected Integer doInBackground() throws Exception {
                return Bridge.startChatRoom(username);
            }

            @Override
            protected void done() {
                try {
                    if (get() == 0) {
                        swithPanel("MAIN");
                    }
                } catch (InterruptedException | ExecutionException e) {
                    e.printStackTrace();
                }
            }
        }.execute();
    }

    public void joinChatRoom(String ipAddress, String port, String secretKey, String username) {
        SwingUtilities.invokeLater(() -> mainFrame.getJoinChatRoomPanel().clearErrors());
        new SwingWorker<Void, Void>() {
            @Override
            protected Void doInBackground() throws Exception {
                Bridge.joinChatRoom(ipAddress, port, secretKey, username);
                return null;
            }
        }.execute();
    }

    public void sendMessage(String message) {
        Bridge.sendMessage(message);
    }

    public static void displayMessage(final String username, final String message) {
        SwingUtilities.invokeLater(() -> {
            mainFrame.getMainChatRoomPanel().appendMessage(username + ": " + message + "\n");
        });
    }

    public static void showCriticalError(String errorMessage) {
        SwingUtilities.invokeLater(() -> {
            mainFrame.getMainChatRoomPanel().showCriticalError(errorMessage);
        });
    }

    public static void logNonCriticalError(String errorMessage) {
        SwingUtilities.invokeLater(() -> {
            mainFrame.getMainChatRoomPanel().appendError("ERROR: " + errorMessage + "\n");
        });
    }

    public static void showUsernameError(String errorMessage) {
        SwingUtilities.invokeLater(() -> {
            mainFrame.getJoinChatRoomPanel().showUsernameError(errorMessage);
        });
    }

    public static void showSecretKeyError(String errorMessage) {
        SwingUtilities.invokeLater(() -> {
            mainFrame.getJoinChatRoomPanel().showSecretKeyError(errorMessage);
        });
    }

    public static void switchToMainPanel() {
        SwingUtilities.invokeLater(() -> mainFrame.swithPanel("MAIN"));
    }
}