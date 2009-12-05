#include "GtkPainter.h"
#include "MapApp.h"

CMapApp* app;

#include <gtkmm.h>
#include <hildonmm.h>
#include <hildon-fmmm.h>

int main(int argc, char ** argv)
{
	Gtk::Main    toolkit (argc, argv);
    Hildon::init();
	Hildon::fm_init();
	Hildon::Window window;

    app = new CMapApp(window);
    app->Create(0);
    for (int i = 1; i < argc; ++i)
        app->ProcessCmdLineElement(argv[i]);
	window.add(app->m_painter);
	window.set_main_menu(app->m_Menu);
	window.show_all_children();
    Hildon::Program::get_instance()->add_window(window);

    // window.add(dp);
    // dp.show();


	// while (*++argv)
	//	dp.AddMap(*argv);
	// Glib::Thread::create(sigc::mem_fun(&dp, &DumpPainter::ThreadRoutine), true);
 	// Gtk::Main::run (window);
 	toolkit.run(window);

	return 0;
}

