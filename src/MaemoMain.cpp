#include "GtkPainter.h"
#include "MapApp.h"

CMapApp* app;

#include <gtkmm.h>
#ifdef HILDON
#	include <hildonmm.h>
#	include <hildon-fmmm.h>
#endif // HILDON

int main(int argc, char ** argv)
{
	Gtk::Main    toolkit (argc, argv);
#ifdef HILDON
    Hildon::init();
	Hildon::fm_init();
	Hildon::Window window;
#else // HILDON
	Gtk::Window window;
#endif // HILDON

    app = new CMapApp(window);
    app->Create(0);
    for (int i = 1; i < argc; ++i)
        app->ProcessCmdLineElement(argv[i]);
#ifdef HILDON
	window.add(app->m_painter);
	window.set_main_menu(app->m_Menu);
#else
	window.set_default_size(500, 300);
	Gtk::VBox vbox(false, 0);
	vbox.pack_start(app->m_menuBar, false, false);
	vbox.pack_start(app->m_painter, true, true);
	vbox.show_all_children();
	window.add(vbox);
#endif // HILDON
	window.show_all_children();
#ifdef HILDON
    Hildon::Program::get_instance()->add_window(window);
#endif // HILDON

    // window.add(dp);
    // dp.show();


	// while (*++argv)
	//	dp.AddMap(*argv);
	// Glib::Thread::create(sigc::mem_fun(&dp, &DumpPainter::ThreadRoutine), true);
 	// Gtk::Main::run (window);
 	toolkit.run(window);

	return 0;
}

