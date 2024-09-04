package view;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JOptionPane;
import java.awt.BorderLayout;
import java.awt.Dimension;

import controller.Controller;

public class MainChatRoomPanel extends JPanel {

    private JTextArea messageDisplayArea;
    private JTextArea errorDisplayArea; // New area for non-critical errors
    private JList<String> userList;
    private JTextField messageInputField;
    private JButton sendButton;

    public MainChatRoomPanel(Controller controller) {
        setLayout(new BorderLayout());

        // Messages display area
        messageDisplayArea = new JTextArea();
        messageDisplayArea.setEditable(false);
        JScrollPane messageScrollPane = new JScrollPane(messageDisplayArea);

        // Error display area for non-critical errors
        errorDisplayArea = new JTextArea();
        errorDisplayArea.setEditable(false);
        errorDisplayArea.setForeground(java.awt.Color.RED);
        JScrollPane errorScrollPane = new JScrollPane(errorDisplayArea);

        // Split pane to separate chat messages and errors
        JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, messageScrollPane, errorScrollPane);
        splitPane.setResizeWeight(0.8); // Adjust the weight according to your preference
        splitPane.setDividerSize(2); // Set a smaller divider

        add(splitPane, BorderLayout.CENTER);

        // User list panel
        userList = new JList<>();
        JScrollPane userScrollPane = new JScrollPane(userList);
        userScrollPane.setPreferredSize(new Dimension(150, 0)); // Width of 150 pixels, height flexible
        add(userScrollPane, BorderLayout.EAST);

        // Message input and send button
        JPanel inputPanel = new JPanel(new BorderLayout());
        messageInputField = new JTextField();
        sendButton = new JButton("Send");

        inputPanel.add(messageInputField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);

        add(inputPanel, BorderLayout.SOUTH);

        // Bind the send button to the controller's sendMessage method
        sendButton.addActionListener(e -> {
            String message = messageInputField.getText();
            if (!message.isEmpty()) {
                controller.sendMessage(message);
                messageInputField.setText(""); // Clear the input field after sending
            }
        });
    }

    public void appendMessage(String message) {
        messageDisplayArea.append(message);
    }

    public void appendError(String error) {
        errorDisplayArea.append(error);
    }

    public void showCriticalError(String errorMessage) {
        JOptionPane.showMessageDialog(this, errorMessage, "Critical Error", JOptionPane.ERROR_MESSAGE);
    }

    public JTextArea getMessageDisplayArea() {
        return messageDisplayArea;
    }

    public JTextArea getErrorDisplayArea() {
        return errorDisplayArea;
    }

    public JList<String> getUserList() {
        return userList;
    }

    public JTextField getMessageInputField() {
        return messageInputField;
    }

    public JButton getSendButton() {
        return sendButton;
    }
}
