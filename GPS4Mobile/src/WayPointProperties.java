<import javax.microedition.lcdui.*;

class WayPointProperties extends Form implements CommandListener {
    private final static Command CMD_CANCEL = new Command("Cancel", Command.CANCEL, 1);
	private final static Command CMD_SAVE = new Command("Save", Command.OK, 1);
	private TextField editName;
	private WayPoint editWp;
	private Displayable prev;
	private Points points;
	private Display display;

	WayPointProperties(WayPoint wp, Display d, Displayable pr, Points ps) {
		super("Waypoint");
		prev = pr;
		points = ps;
		display = d;
		editName = new TextField("Name:", wp.l, 100, TextField.ANY);
		editWp = wp;
		append(editName);
		addCommand(CMD_CANCEL);
		addCommand(CMD_SAVE);
		setCommandListener(this);
	}

    public void commandAction(Command c, Displayable d) {
		if (c == CMD_CANCEL)
			display.setCurrent(prev);
		if (c == CMD_SAVE) {
			editWp.l = editName.getString();
			points.write();
			display.setCurrent(prev);
		}
	}
}
