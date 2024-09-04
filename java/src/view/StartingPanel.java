package view;

import javax.swing.JButton;
import javax.swing.JPanel;
import java.awt.GridBagLayout;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.Insets;

import controller.Controller;

public class StartingPanel extends JPanel {

    private static final String JOIN_BUTTON_TEXT = "Join Chat Room";
    private static final String CREATE_BUTTON_TEXT = "Create Chat Room";
    private static final Dimension BUTTON_DIMENSIONS = new Dimension(150, 35);

    public StartingPanel(Controller controller) {
        setLayout(new GridBagLayout());

        GridBagConstraints buttonConstraints = new GridBagConstraints();
        buttonConstraints.gridy = 0; // row number for join chat room button
        buttonConstraints.insets = new Insets(0, 0, 50, 0);
        buttonConstraints.anchor = GridBagConstraints.CENTER;
        buttonConstraints.weightx = 1.0;

        JButton joinChatRoomButton = new JButton(JOIN_BUTTON_TEXT);
        joinChatRoomButton.setPreferredSize(BUTTON_DIMENSIONS);
        joinChatRoomButton.addActionListener(e -> controller.swithPanel("JOIN"));
        
        add(joinChatRoomButton, buttonConstraints);

        buttonConstraints.gridy = 1; // row number for create chat room button
        JButton createChatRoomButton = new JButton(CREATE_BUTTON_TEXT);
        createChatRoomButton.setPreferredSize(BUTTON_DIMENSIONS);
        createChatRoomButton.addActionListener(e -> controller.swithPanel("CREATE"));

        add(createChatRoomButton, buttonConstraints);
    }
}
