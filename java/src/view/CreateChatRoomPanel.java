package view;

import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTextField;
import java.awt.BorderLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.FlowLayout;
import java.awt.Dimension;
import java.awt.Insets;

import controller.Controller;

public class CreateChatRoomPanel extends JPanel {

    private static final String BACK_BUTTON_TEXT = "Back";
    private static final String USERNAME_LABEL_TEXT = "Username";
    private static final String CREATE_BUTTON_TEXT = "Create Chat Room";
    private static final Dimension BACK_BUTTON_DIMENSIONS = new Dimension(75, 30);
    private static final Dimension OTHER_COMPONENT_DIMENSIONS = new Dimension(150, 35);
    private static final Dimension SPACER_DIMENSIONS = new Dimension(75, 40);
    private static final Dimension TEXTFIELD_MIN_SIZE = new Dimension(50, 35);

    private JTextField usernameField;

    public CreateChatRoomPanel(Controller controller) {
        setLayout(new BorderLayout());

        JPanel headerPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JButton backButton = new JButton(BACK_BUTTON_TEXT);
        backButton.setPreferredSize(BACK_BUTTON_DIMENSIONS);
        backButton.addActionListener(e -> controller.swithPanel("START"));
        headerPanel.add(backButton);

        JPanel centralPanel = new JPanel(new GridBagLayout());
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.gridy = 0; // row number for username label
        constraints.insets = new Insets(0, 0, 5, 0); // insets for username label
        constraints.anchor = GridBagConstraints.CENTER;

        JLabel usernameLabel = new JLabel(USERNAME_LABEL_TEXT);
        centralPanel.add(usernameLabel, constraints);

        constraints.gridy = 1; // row number for username field
        constraints.insets = new Insets(0, 0, 50, 0);

        usernameField = new JTextField();
        usernameField.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        usernameField.setMinimumSize(TEXTFIELD_MIN_SIZE);
        centralPanel.add(usernameField, constraints);

        constraints.gridy = 2; // row number for create chat room button
        JButton createChatRoomButton = new JButton(CREATE_BUTTON_TEXT);
        createChatRoomButton.setPreferredSize(OTHER_COMPONENT_DIMENSIONS);
        createChatRoomButton.addActionListener(e -> controller.startChatRoom(usernameField.getText()));
        centralPanel.add(createChatRoomButton, constraints);

        add(headerPanel, BorderLayout.NORTH);
        add(centralPanel, BorderLayout.CENTER);

        JPanel bottomSpacer = new JPanel();
        bottomSpacer.setPreferredSize(SPACER_DIMENSIONS);
        add(bottomSpacer, BorderLayout.SOUTH);
    }
}
