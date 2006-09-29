//Author: Jonathon Jongsma
/*
 *This file is part of the Nemiver project
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_SAVED_SESSIONS_DIALOG_H__
#define __NEMIVER_SAVED_SESSIONS_DIALOG_H__

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "nmv-dialog.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-sess-mgr.h"

namespace nemiver {

namespace common {
class UString ;
}

using nemiver::common::UString ;
using nemiver::common::SafePtr ;

class SavedSessionsDialog : public Dialog {
    Gtk::TreeView* m_treeview_sessions;

    struct SessionModelColumns : public Gtk::TreeModel::ColumnRecord
    {
        // I tried using UString here, but it didn't want to compile... jmj
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<gint64> id;
        Gtk::TreeModelColumn<ISessMgr::Session> session;
        SessionModelColumns() { add (name); add (id); add (session); }
    };
    SessionModelColumns m_session_columns;
    Glib::RefPtr<Gtk::ListStore> m_model;

public:
    SavedSessionsDialog (const UString &a_root_path, ISessMgr *a_sesssion_manager) ;
    virtual ~SavedSessionsDialog () ;

    ISessMgr::Session session () const;
    void session (const ISessMgr::Session &a_session) const;

};//end class nemiver

}//end namespace nemiver

#endif //__NEMIVER_SAVED_SESSIONS_DIALOG_H__

