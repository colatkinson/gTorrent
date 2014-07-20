#include "GtkAddMagnetLinkWindow.hpp"
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/hvseparator.h>
#include "GtkMainWindow.hpp"
#include <Application.hpp>
#include <gtkmm/stock.h>
#include <glibmm.h>
#include <giomm.h>

GtkMainWindow::GtkMainWindow() :
	m_core(Application::getSingleton()->getCore())
{
	//TODO:This needs to be refactored
	this->set_position(Gtk::WIN_POS_CENTER);
	this->set_default_size(800, 500);
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

	header = Gtk::manage(new Gtk::HeaderBar());
	header->set_show_close_button(true);
	//TODO: add max/minimise buttons, next to the close button
	//header->set_decoration_layout(
	//		btn_connect,btn_add_torrent,btn_add_link,btn_up,btn_down,btn_pause,
	//		btn_remove:maximise,minimize,close);
	//TODO:Here's a nifty connect button to hang connection settings, rate limiter from.
	Gtk::Button *btn_add_link    = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_add_torrent = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_connect     = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_down        = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_pause       = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_properties  = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_remove      = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_resume      = Gtk::manage(new Gtk::Button());
	Gtk::Button *btn_up          = Gtk::manage(new Gtk::Button());
	Gtk::VSeparator *separator0  = Gtk::manage(new Gtk::VSeparator());
	Gtk::VSeparator *separator1  = Gtk::manage(new Gtk::VSeparator());
	Gtk::VSeparator *separator2  = Gtk::manage(new Gtk::VSeparator());

	btn_add_link->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onAddMagnetBtnClicked));
	btn_add_torrent->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onAddBtnClicked));
	btn_pause->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onPauseBtnClicked));
	btn_resume->signal_clicked().connect(sigc::mem_fun(*this, &GtkMainWindow::onResumeBtnClicked));

	btn_add_link->set_image_from_icon_name    ("edit-paste");
	btn_add_torrent->set_image_from_icon_name ("gtk-add");
	btn_connect->set_image_from_icon_name     ("gtk-directory");
	btn_down->set_image_from_icon_name        ("gtk-go-down");
	btn_pause->set_image_from_icon_name       ("media-playback-pause");
	btn_properties->set_image_from_icon_name  ("gtk-properties");
	btn_remove->set_image_from_icon_name      ("gtk-cancel");
	btn_resume->set_image_from_icon_name      ("media-playback-start");
	btn_up->set_image_from_icon_name          ("gtk-go-up");

	//TODO:align properties button to right of top bar
	//btn_properties->set_alignment(1.0f,0.0f);
	header->add(*btn_add_torrent);
	header->add(*btn_add_link);
	header->add(*btn_connect);
	header->add(*separator0);
	header->add(*btn_pause);
	header->add(*btn_resume);
	header->add(*btn_remove);
	header->add(*separator1);
	header->add(*btn_down);
	header->add(*btn_up);
	header->add(*btn_properties);
	header->add(*separator2);

	this->set_titlebar(*header);
	//status = Gtk::manage(new Gtk::StatusBar());
	//this->set_decorated(FALSE);
	//this->set_hide_titlebar_when_maximized(TRUE);
	this->maximize();
	this->show_all();
	//status = Gtk::manage(new Gtk::StatusBar());
	//this->get_window().set_decoration(64);//WMDecoration.BORDER
	//this.get_window().set_decorations(Gdk.WMDecoration.BORDER);
	//this->set_decorations(FALSE);
	//this->set_decorated(FALSE);
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
	fc.set_default_size(256, 256);
	//fc.set_window_position(0, 128);
	fc.set_select_multiple();
	fc.set_transient_for(*this);
	fc.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fc.add_button("Select", Gtk::RESPONSE_OK);

	Glib::RefPtr<Gtk::FileFilter> filter_t = Gtk::FileFilter::create();
	filter_t->set_name("Torrent Files");
	filter_t->add_mime_type("application/x-bittorrent");
	fc.add_filter(filter_t);

	int result = fc.run();

	switch (result)
	{
	case Gtk::RESPONSE_OK:
		for (auto & f : fc.get_filenames())
		{
			shared_ptr<Torrent> t = m_core->addTorrent(f.c_str());
			if (t)//Checks if t is not null
				m_treeview->addCell(t);
			//TODO Add error dialogue if torrent add is unsuccessful
		}
		break;
	}
}

void GtkMainWindow::onAddMagnetBtnClicked()
{
	GtkAddMagnetLinkWindow d;
	d.set_transient_for(*this);
	int r = d.run();

	switch (r)
	{
	case Gtk::RESPONSE_OK:
		shared_ptr<Torrent> t = m_core->addTorrent(d.getMagnetURL());
		if (t)//Checks if t is not null
			m_treeview->addCell(t);
		//TODO Add error dialogue if torrent add is unsuccessful
		break;
	}
}

void GtkMainWindow::onPauseBtnClicked()
{
	m_treeview->setSelectedPaused(true);
}

void GtkMainWindow::onResumeBtnClicked()
{
	m_treeview->setSelectedPaused(false);
}

void GtkMainWindow::onRemoveBtnClicked()
{
	//get the torrent selected in treeview

	//remove the torrent from treeview
	//torrent.remove();
}
/*
void GtkMainWindow::onPropertiesBtnClicked()
{
	GtkPropertiesWindow d;
	d.set_transient_for(*this);
	int r = d.run();

	switch (r)
	{
	case Gtk::RESPONSE_OK:
		//TODO: Store slected settings to .config file
		break;
	}
}*/

bool GtkMainWindow::onDestroy(GdkEventAny *event)
{
	m_core->shutdown();
	return false;
}
