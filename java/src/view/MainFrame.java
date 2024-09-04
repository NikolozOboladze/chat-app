package view;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.WindowConstants;
import java.awt.CardLayout;
import java.awt.Dimension;

import controller.Controller;

public class MainFrame extends JFrame {

   private static final String TITLE = "Chat App";
   private static final Dimension STARTING_WINDOW_SIZE = new Dimension(400, 600);

   private CardLayout cardLayout;
   private JPanel cards;

   private MainChatRoomPanel mainChatRoomPanel;
   private JoinChatRoomPanel joinChatRoomPanel;

   public MainFrame(Controller controller) {
      initializeMainFrame();
      controller.setMainFrame(this);

      cardLayout = new CardLayout();
      cards = new JPanel(cardLayout);

      mainChatRoomPanel = new MainChatRoomPanel(controller);
      joinChatRoomPanel = new JoinChatRoomPanel(controller);

      cards.add(new StartingPanel(controller), "START");
      cards.add(joinChatRoomPanel, "JOIN");
      cards.add(new CreateChatRoomPanel(controller), "CREATE");
      cards.add(mainChatRoomPanel, "MAIN");

      add(cards);
      cardLayout.show(cards, "START");
   }

   private void initializeMainFrame() {
      setSize(STARTING_WINDOW_SIZE);
      setTitle(TITLE);
      setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
      setLocationRelativeTo(null);
   }

   public void swithPanel(String panelName) {
      cardLayout.show(cards, panelName);
   }

   public MainChatRoomPanel getMainChatRoomPanel() {
      return mainChatRoomPanel;
   }

   public JoinChatRoomPanel getJoinChatRoomPanel() {
      return joinChatRoomPanel;
   }

}