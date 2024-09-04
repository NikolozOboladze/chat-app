import controller.Controller;
import view.MainFrame;

public class App {

    static {
        System.loadLibrary("main");
    }

    public static void main(String[] args) {
        Controller controller = new Controller();
        MainFrame mainFrame = new MainFrame(controller);
        mainFrame.setVisible(true);
    }
}