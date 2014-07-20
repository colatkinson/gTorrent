#include "GtkAddMagnetLinkWindow.hpp"
#include <gtkmm/filechooserdialog.h>
#include "GtkMainWindow.hpp"
#include <Application.hpp>
#include <gtkmm/button.h>
#include <gtkmm/hvseparator.h>
#include <gtkmm/stock.h>
#include <glibmm.h>
#include <giomm.h>

GtkMainWindow::GtkMainWindow() :
	m_core(Application::getSingleton()->getCore())
{
	this->set_position(Gtk::WIN_POS_CENTER);
	this->set_default_size(800, 500);

	header = Gtk::manage(new Gtk::HeaderBar());
	header->set_title("gTorrent");
	header->set_show_close_button(true);

	// This needs to be refactored

	Gtk::Button *add_torrent_btn = Gtk::manage(new Gtk::Button());
	add_torrent_btn->set_image_from_icon_name("gtk-add");
	add_torrent_btn->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onAddBtnClicked));
	header->add(*add_torrent_btn);

	Gtk::Button *add_link_btn = Gtk::manage(new Gtk::Button());
	add_link_btn->set_image_from_icon_name("gtk-paste");
	add_link_btn->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onAddMagnetBtnClicked));
	header->add(*add_link_btn);

	Gtk::VSeparator *separator = Gtk::manage(new Gtk::VSeparator());
	header->add(*separator);

	Gtk::Button *pause_btn = Gtk::manage(new Gtk::Button());
	pause_btn->set_image_from_icon_name("gtk-media-pause");
	header->add(*pause_btn);

	this->set_titlebar(*header);

	m_treeview = Gtk::manage(new GtkTorrentTreeView());
	this->add(*m_treeview);

	// Let's add some DnD goodness

	vector<Gtk::TargetEntry> listTargets;
	listTargets.push_back(Gtk::TargetEntry("STRING"));
	listTargets.push_back(Gtk::TargetEntry("text/plain"));
	listTargets.push_back(Gtk::TargetEntry("text/uri-list"));
	listTargets.push_back(Gtk::TargetEntry("application/x-bittorrent"));

	m_treeview->drag_dest_set(listTargets, Gtk::DEST_DEFAULT_MOTION | Gtk::DEST_DEFAULT_DROP, Gdk::ACTION_COPY | Gdk::ACTION_MOVE | Gdk::ACTION_LINK | Gdk::ACTION_PRIVATE);
	m_treeview->signal_drag_data_received().connect(sigc::mem_fun(*this, &GtkMainWindow::onFileDropped));

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &GtkMainWindow::onSecTick), 10);
	this->signal_delete_event().connect(sigc::mem_fun(*this, &GtkMainWindow::onDestroy));

	this->show_all();
}

static inline string &rtrim(string &s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

void GtkMainWindow::onFileDropped(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time)
{
	string str = selection_data.get_data_as_string();
	string str2 = Glib::filename_from_uri(str);
	string str3 = rtrim(str2);
	bool want_uncertain = true;
	string content_type = Gio::content_type_guess(str3, selection_data.get_data_as_string(), want_uncertain);
	if(content_type == "application/x-bittorrent") {
		shared_ptr<Torrent> t = m_core->addTorrent(str3);
		m_treeview->addCell(t);
	}
}

bool GtkMainWindow::onSecTick()
{
	m_treeview->updateCells();
	return true;
}

void GtkMainWindow::onAddBtnClicked()
{
	Gtk::FileChooserDialog fc("Browse for torrent file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	fc.set_select_multiple();
	fc.set_transient_for(*this);
	fc.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fc.add_button("Select", Gtk::RESPONSE_OK);

  	Glib::RefPtr<Gtk::FileFilter> filter_t = Gtk::FileFilter::create();
	filter_t->set_name("Torrent Files");
	filter_t->add_mime_type("application/x-bittorrent");
	fc.add_filter(filter_t);

	int result = fc.run();

	switch (result) {
		case Gtk::RESPONSE_OK:
			for (auto &f : fc.get_filenames())
			{
				shared_ptr<Torrent> t = m_core->addTorrent(f.c_str());
				m_treeview->addCell(t);
			}
		break;
	}
}

void GtkMainWindow::onAddMagnetBtnClicked()
{
	GtkAddMagnetLinkWindow d;
	d.set_transient_for(*this);
	int r = d.run();

	switch (r) {
		case Gtk::RESPONSE_OK:
			shared_ptr<Torrent> t = m_core->addTorrent(d.getMagnetURL());
			m_treeview->addCell(t);
		break;
	}
}

bool GtkMainWindow::onDestroy(GdkEventAny *event)
{
	m_core->shutdown();
	return false;
}
