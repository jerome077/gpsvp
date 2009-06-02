import javax.microedition.lcdui.*;
import java.util.*;

class PointsList extends List implements CommandListener {
	private final static Command CMD_CLOSE = new Command("Close", Command.OK, 1);
	private Displayable prev;
	private Points points;
	private Display display;
	PointsList(Points ps, Display d, Displayable pr) {
		super("Waypoints", IMPLICIT);
		prev = pr;
		points = ps;
		display = d;
		Enumeration e = points.getPoints();
		while (e.hasMoreElements()) {
			WayPoint pt = (WayPoint)e.nextElement();
			append(pt.l, null);
		}
		addCommand(CMD_CLOSE);
		setCommandListener(this);
	}
    public void commandAction(Command c, Displayable d) {
		if (c == CMD_CLOSE)
			display.setCurrent(prev);
	}
}
