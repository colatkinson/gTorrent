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
	//bool onFileDropped(const Glib::RefPtr<Gdk::DragContext> &, int, int, const Gtk::SelectionData &, unsigned int, unsigned int);
	//bool onFileDropped(bool, GtkMainWindow, const Glib::RefPtr<Gdk::DragContext> &, int, int, const Gtk::SelectionData &, unsigned int, unsigned int) const volatile;
	/*void on_button_drag_data_get(
	const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time);*/
	void onFileDropped(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
};

