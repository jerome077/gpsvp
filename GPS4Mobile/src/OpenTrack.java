/*
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.List;
import javax.microedition.lcdui.Choice;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Display;

class OpenTrack extends List implements CommandListener {
    private final static Command CMD_CANCEL = new Command("Cancel", Command.CANCEL, 1);
	private Displayable previous = null;
	private Display display;

	OpenTrack(Display d) {
		super("Tracks2", Choice.IMPLICIT, new String[0], null);
		display = d;
		addCommand(SELECT_COMMAND);
		addCommand(CMD_CANCEL);
		append("Something2", null);
		previous = display.getCurrent();
		display.setCurrent(this);
		setCommandListener(this);
	}

    public void commandAction(Command c, Displayable d) {
		display.setCurrent(previous);
	}
}
*/
