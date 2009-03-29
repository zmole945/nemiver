// -*- c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-'
//Author: Jonathon Jongsma
//        Hubert Figuiere
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
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */

#include <vector>
#include <glib/gi18n.h>
#include <libglademm.h>
#include <gtkmm/dialog.h>
#include "common/nmv-exception.h"
#include "common/nmv-env.h"
#include "common/nmv-ustring.h"
#include "nmv-set-breakpoint-dialog.h"
#include "nmv-ui-utils.h"

using namespace std;
using namespace nemiver::common;

namespace nemiver {

class EventComboModelColumns
    : public Gtk::TreeModel::ColumnRecord
{
public:
    EventComboModelColumns()
        {
            add(m_label);
            add(m_command);
        }
    Gtk::TreeModelColumn<Glib::ustring> m_label;
    Gtk::TreeModelColumn<UString> m_command;
};

class SetBreakpointDialog::Priv {
public:
    Gtk::ComboBox *combo_event;
    EventComboModelColumns combo_event_col_model;
    Glib::RefPtr<Gtk::TreeStore> combo_event_model;
    Gtk::Entry *entry_filename;
    Gtk::Entry *entry_line;
    Gtk::Entry *entry_function;
    Gtk::Entry *entry_condition;
    Gtk::RadioButton *radio_source_location;
    Gtk::RadioButton *radio_function_name;
    Gtk::RadioButton *radio_event;
    Gtk::Button *okbutton;

public:
    Priv (Gtk::Dialog &a_dialog,
          const Glib::RefPtr<Gnome::Glade::Xml> &a_glade) :
        combo_event (0),
        entry_filename (0),
        entry_line (0),
        entry_function (0),
        radio_source_location (0),
        radio_function_name (0),
        radio_event (0),
        okbutton (0)
    {
        a_dialog.set_default_response (Gtk::RESPONSE_OK);

        okbutton =
            ui_utils::get_widget_from_glade<Gtk::Button> (a_glade,
                                                          "okbutton");
        THROW_IF_FAIL (okbutton);
        okbutton->set_sensitive (false);

        combo_event =
            ui_utils::get_widget_from_glade<Gtk::ComboBox>
            (a_glade, "combo_event");
        combo_event_model = Gtk::TreeStore::create(combo_event_col_model);
        combo_event->set_model(combo_event_model);
        Gtk::TreeModel::Row row;

        row = *(combo_event_model->append());
        row[combo_event_col_model.m_label] = _("Throw Exception");
        row[combo_event_col_model.m_command] = "throw";
        row = *(combo_event_model->append());
        row[combo_event_col_model.m_label] = _("Catch Exception");
        row[combo_event_col_model.m_command] = "catch";

        combo_event->set_active (0);

        entry_filename =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "filenameentry");
        entry_filename->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal));

        entry_line =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "lineentry");
        entry_line->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal));
        entry_line->set_activates_default ();

        entry_function =
            ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "functionentry");
        entry_function->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal));
        entry_function->set_activates_default ();

        entry_condition = ui_utils::get_widget_from_glade<Gtk::Entry>
                (a_glade, "conditionentry");
        entry_condition->signal_changed ().connect (sigc::mem_fun
                (*this, &Priv::on_text_changed_signal));
        entry_condition->set_activates_default ();

        radio_source_location =
            ui_utils::get_widget_from_glade<Gtk::RadioButton>
                (a_glade, "sourcelocationradio");
        radio_source_location->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_radiobutton_changed));

        radio_function_name =
            ui_utils::get_widget_from_glade<Gtk::RadioButton>
                (a_glade, "functionnameradio");
        radio_function_name->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_radiobutton_changed));

        radio_event =
            ui_utils::get_widget_from_glade<Gtk::RadioButton>
            (a_glade, "eventradio");
        radio_event->signal_clicked ().connect (sigc::mem_fun
                (*this, &Priv::on_radiobutton_changed));

        // set the 'function name' mode active by default
        mode (MODE_FUNCTION_NAME);
        // hack to ensure that the correct text entry fields
        // get insensitive at startup since if the glade file
        // initializes MODE_FUNCTION_NAME to
        // active, the 'changed' signal won't be emitted here
        // (is there a better way to do this?)
        on_radiobutton_changed ();
    }

    void update_ok_button_sensitivity ()
    {
        THROW_IF_FAIL (entry_filename);
        THROW_IF_FAIL (entry_line);

        SetBreakpointDialog::Mode a_mode = mode ();

        switch (a_mode) {
            case MODE_SOURCE_LOCATION:
                // make sure there's something in the line number entry,
                // at least, and that something is a valid number.
                if (!entry_line->get_text ().empty ()
                    && atoi (entry_line->get_text ().c_str ())) {
                    okbutton->set_sensitive (true);
                } else {
                    okbutton->set_sensitive (false);
                }
                break;
            case MODE_FUNCTION_NAME:
                if (!entry_function->get_text ().empty ()) {
                    okbutton->set_sensitive (true);
                } else {
                    okbutton->set_sensitive (false);
                }
                break;
            default:
                okbutton->set_sensitive (true);
                break;
        }
    }

    void on_text_changed_signal ()
    {
        NEMIVER_TRY
        update_ok_button_sensitivity ();
        NEMIVER_CATCH
    }

    void on_radiobutton_changed ()
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;
        NEMIVER_TRY

        THROW_IF_FAIL (entry_filename);
        THROW_IF_FAIL (entry_line);
        THROW_IF_FAIL (entry_function);

        SetBreakpointDialog::Mode a_mode = mode ();

        entry_function->set_sensitive (a_mode == MODE_FUNCTION_NAME);
        entry_filename->set_sensitive (a_mode == MODE_SOURCE_LOCATION);
        entry_line->set_sensitive (a_mode == MODE_SOURCE_LOCATION);
        combo_event->set_sensitive (a_mode == MODE_EVENT);
        entry_condition->set_sensitive (a_mode != MODE_EVENT);

        switch (a_mode) {
            case MODE_SOURCE_LOCATION:
            LOG_DD ("Setting Sensitivity for SOURCE_LOCATION");
            break;
        case MODE_FUNCTION_NAME:
            LOG_DD ("Setting Sensitivity for FUNCTION_NAME");
            break;
        case MODE_EVENT:
            LOG_DD ("Setting Sensitivity for EVENT");
            break;
        }
        update_ok_button_sensitivity ();
        NEMIVER_CATCH
    }

    void mode (SetBreakpointDialog::Mode a_mode)
    {
        LOG_FUNCTION_SCOPE_NORMAL_DD;

        THROW_IF_FAIL (radio_source_location);
        THROW_IF_FAIL (radio_function_name);
        THROW_IF_FAIL (entry_line);
        THROW_IF_FAIL (entry_filename);
        THROW_IF_FAIL (entry_function);

        switch (a_mode) {
            case MODE_SOURCE_LOCATION:
                LOG_DD ("Changing Mode to SOURCE_LOCATION");
                radio_source_location->set_active ();
                entry_filename->grab_focus ();
                break;
            case MODE_FUNCTION_NAME:
                LOG_DD ("Changing Mode to FUNCTION_NAME");
                radio_function_name->set_active ();
                entry_function->grab_focus ();
                break;
            case MODE_EVENT:
                LOG_DD ("Changing Mode to EVENT");
                radio_event->set_active ();
                combo_event->grab_focus ();
                break;
            default:
                THROW ("Should not be reached");
        }
    }

    UString get_active_event () const
    {
        Gtk::TreeModel::iterator iter = combo_event->get_active ();
        return (*iter)[combo_event_col_model.m_command];
    }

    void set_active_event (const UString &) const
    {
        //TODO
    }

    SetBreakpointDialog::Mode mode () const
    {
        THROW_IF_FAIL (radio_source_location);
        THROW_IF_FAIL (radio_function_name);


        if (radio_source_location->get_active ()) {
            return MODE_SOURCE_LOCATION;
        } else if (radio_event->get_active ()) {
            return MODE_EVENT;
        } else {
            return MODE_FUNCTION_NAME;
        }
    }
};//end class SetBreakpointDialog::Priv

SetBreakpointDialog::SetBreakpointDialog (const UString &a_root_path) :
    Dialog (a_root_path, "setbreakpointdialog.glade", "setbreakpointdialog")
{
    m_priv.reset (new Priv (widget (), glade ()));
}

SetBreakpointDialog::~SetBreakpointDialog ()
{
}

UString
SetBreakpointDialog::file_name () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_filename);

    return m_priv->entry_filename->get_text ();
}

void
SetBreakpointDialog::file_name (const UString &a_name)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_filename);
    m_priv->entry_filename->set_text (a_name);
}

int
SetBreakpointDialog::line_number () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_line);
    return atoi (m_priv->entry_line->get_text ().c_str ());
}

void
SetBreakpointDialog::line_number (int a_line)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_line);
    m_priv->entry_line->set_text (UString::from_int(a_line));
}

UString
SetBreakpointDialog::function () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_function);

    return m_priv->entry_function->get_text ();
}

void
SetBreakpointDialog::function (const UString &a_name)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_function);
    m_priv->entry_function->set_text (a_name);
}

UString
SetBreakpointDialog::event () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->combo_event);
    return m_priv->get_active_event();
}

void SetBreakpointDialog::event (const UString &a_event) 
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->combo_event);
    m_priv->set_active_event (a_event);

}

UString
SetBreakpointDialog::condition () const
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_condition);
    return m_priv->entry_condition->get_text ();
}

void
SetBreakpointDialog::condition (const UString &a_cond)
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (m_priv->entry_condition);
    m_priv->entry_condition->set_text (a_cond);
}


SetBreakpointDialog::Mode
SetBreakpointDialog::mode () const
{
    THROW_IF_FAIL (m_priv);
    return m_priv->mode ();
}


void
SetBreakpointDialog::mode (Mode a_mode)
{
    THROW_IF_FAIL (m_priv);
    m_priv->mode (a_mode);
}

}//end namespace nemiver
