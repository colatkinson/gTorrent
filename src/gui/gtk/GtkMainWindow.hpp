#pragma once

#include "GtkTorrentTreeView.hpp"
#include <gtkmm/headerbar.h>
#include <gtkmm/window.h>
#include <core/Core.hpp>
#include <gtkmm/main.h>

class GtkMainWindow : public Gtk::Window
{
private:
	shared_ptr<gt::Core> &m_core;

	Gtk::HeaderBar *header;
	GtkTorrentTreeView *m_treeview;

	// Signal Responders
	
	void onAddBtnClicked();
	void onAddMagnetBtnClicked();

public:
	GtkMainWindow();

	bool onDestroy(GdkEventAny *event);
	bool onSecTick();
	void onFileDropped(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
};

