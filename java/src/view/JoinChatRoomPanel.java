package view;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.BorderFactory;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Insets;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import controller.Controller;

public class JoinChatRoomPanel extends JPanel {

    private static final String BACK_BUTTON_TEXT = "Back";
    private static final String USERNAME_LABEL_TEXT = "Username";
    private static final String CHAT_ROOM_ADDRESS_LABEL_TEXT = "Chat Room Address";
    private static final String CHAT_ROOM_KEY_LABEL_TEXT = "Chat Room Secret key";
    private static final String PORT_LABEL_TEXT = "Port";
    private static final String JOIN_BUTTON_TEXT = "Join Chat Room";
    private static final Dimension BACK_BUTTON_DIMENSIONS = new Dimension(75, 30);
    private static final Dimension OTHER_COMPONENT_DIMENSIONS = new Dimension(150, 35);
    private static final Dimension SPACER_DIMENSIONS = new Dimension(75, 40);
    private static final Dimension TEXTFIELD_MIN_SIZE = new Dimension(50, 35);
    private static final Insets LABEL_INSETS = new Insets(0, 0, 5, 0);
    private static final Insets OTHER_INSETS = new Insets(0, 0, 20, 0);

    private JTextField usernameField;
    private JTextField chatRoomAddressField;
    private JTextField chatRoomKeyField;
    private JTextField portField;
    private JLabel usernameErrorLabel;
    private JLabel chatRoomKeyErrorLabel;

    public JoinChatRoomPanel(Controller controller) {
        setLayout(new BorderLayout());

        JPanel headerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JButton backButton = new JButton(BACK_BUTTON_TEXT);
        backButton.setPreferredSize(BACK_BUTTON_DIMENSIONS);
        backButton.addActionListener(e -> controller.swithPanel("START"));
        headerPanel.add(backButton);

        JPanel centralPanel = new JPanel(new GridBagLayout());
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.gridy = 0;
        constraints.insets = LABEL_INSETS;
        constraints.anchor = GridBagConstraints.CENTER;

        JLabel usernameLabel = new JLabel(USERNAME_LABEL_TEXT);
        centralPanel.add(usernameLabel, constraints);

        constraints.gridy = 1;
        constraints.insets = OTHER_INSETS;

        usernameField = new JTextField();
        usernameField.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        usernameField.setMinimumSize(TEXTFIELD_MIN_SIZE);
        centralPanel.add(usernameField, constraints);

        usernameErrorLabel = new JLabel();
        usernameErrorLabel.setForeground(Color.RED);
        constraints.gridy = 2;
        centralPanel.add(usernameErrorLabel, constraints);

        constraints.gridy = 3;
        constraints.insets = LABEL_INSETS;

        JLabel chatRoomAddressLabel = new JLabel(CHAT_ROOM_ADDRESS_LABEL_TEXT);
        centralPanel.add(chatRoomAddressLabel, constraints);

        constraints.gridy = 4;
        constraints.insets = OTHER_INSETS;

        chatRoomAddressField = new JTextField();
        chatRoomAddressField.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        chatRoomAddressField.setMinimumSize(TEXTFIELD_MIN_SIZE);
        centralPanel.add(chatRoomAddressField, constraints);

        constraints.gridy = 5;
        constraints.insets = LABEL_INSETS;

        JLabel chatRoomKeyLabel = new JLabel(CHAT_ROOM_KEY_LABEL_TEXT);
        centralPanel.add(chatRoomKeyLabel, constraints);

        constraints.gridy = 6;
        constraints.insets = OTHER_INSETS;

        chatRoomKeyField = new JTextField();
        chatRoomKeyField.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        chatRoomKeyField.setMinimumSize(TEXTFIELD_MIN_SIZE);
        centralPanel.add(chatRoomKeyField, constraints);

        chatRoomKeyErrorLabel = new JLabel();
        chatRoomKeyErrorLabel.setForeground(Color.RED);
        constraints.gridy = 7;
        centralPanel.add(chatRoomKeyErrorLabel, constraints);

        constraints.gridy = 8;
        constraints.insets = LABEL_INSETS;

        JLabel portLabel = new JLabel(PORT_LABEL_TEXT);
        centralPanel.add(portLabel, constraints);

        constraints.gridy = 9;
        constraints.insets = OTHER_INSETS;

        portField = new JTextField();
        portField.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        portField.setMinimumSize(TEXTFIELD_MIN_SIZE);
        centralPanel.add(portField, constraints);

        constraints.gridy = 10;
        constraints.insets = OTHER_INSETS;

        JButton joinChatRoomButton = new JButton(JOIN_BUTTON_TEXT);
        joinChatRoomButton.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        joinChatRoomButton.addActionListener(e -> controller.joinChatRoom(
                chatRoomAddressField.getText(),
                portField.getText(),
                chatRoomKeyField.getText(),
                usernameField.getText()));
        centralPanel.add(joinChatRoomButton, constraints);

        add(headerPanel, BorderLayout.NORTH);
        add(centralPanel, BorderLayout.CENTER);

        JPanel bottomSpacer = new JPanel();
        bottomSpacer.setPreferredSize(SPACER_DIMENSIONS);
        add(bottomSpacer, BorderLayout.SOUTH);
    }

    public void showUsernameError(String errorMessage) {
        usernameField.setBorder(BorderFactory.createLineBorder(Color.RED));
        usernameErrorLabel.setText(errorMessage);
    }

    public void showSecretKeyError(String errorMessage) {
        chatRoomKeyField.setBorder(BorderFactory.createLineBorder(Color.RED));
        chatRoomKeyErrorLabel.setText(errorMessage);
    }

    public void clearErrors() {
        usernameField.setBorder(BorderFactory.createLineBorder(Color.GRAY));
        chatRoomKeyField.setBorder(BorderFactory.createLineBorder(Color.GRAY));
        usernameErrorLabel.setText("");
        chatRoomKeyErrorLabel.setText("");
    }
}
