/**********************************************************************

Audacity: A Digital Audio Editor

PopupMenuTable.h

Paul Licameli

This file defines PopupMenuTable, which inherits from wxEventHandler,

associated macros simplifying the population of tables,

and PopupMenuTable::Menu which is buildable from one or more such
tables, and automatically attaches and detaches the event handlers.

**********************************************************************/

#ifndef __AUDACITY_POPUP_MENU_TABLE__
#define __AUDACITY_POPUP_MENU_TABLE__

class wxCommandEvent;
class wxString;
#include <vector>
#include <wx/menu.h> // to inherit wxMenu
#include "../MemoryX.h"

#include "../Internat.h"

class PopupMenuTable;

struct PopupMenuTableEntry
{
   enum Type { Item, RadioItem, CheckItem, Separator, SubMenu, Invalid };

   Type type;
   int id;
   TranslatableString caption;
   wxCommandEventFunction func;
   PopupMenuTable *subTable;

   PopupMenuTableEntry(Type type_, int id_, TranslatableString caption_,
      wxCommandEventFunction func_, PopupMenuTable *subTable_)
      : type(type_)
      , id(id_)
      , caption(caption_)
      , func(func_)
      , subTable(subTable_)
   {}

   bool IsItem() const { return type == Item || type == RadioItem || type == CheckItem; }
   bool IsSubMenu() const { return type == SubMenu; }
   bool IsValid() const { return type != Invalid; }
};

class PopupMenuTable : public wxEvtHandler
{
public:
   using Entry = PopupMenuTableEntry;

   // Supply a nonempty caption for sub-menu tables
   PopupMenuTable( const Identifier &id, const TranslatableString &caption = {} )
      : mCaption{ caption }
   {}

   // Called before the menu items are appended.
   // Store user data, if needed.
   virtual void InitUserData(void *pUserData) = 0;

   // Called when the menu is about to pop up.
   // Your chance to enable and disable items.
   // Default implementation does nothing.
   virtual void InitMenu(wxMenu *pMenu);

   // Called when menu is destroyed.
   virtual void DestroyMenu() = 0;

   // Optional pUserData gets passed to the InitUserData routines of tables.
   // No memory management responsibility is assumed by this function.
   static std::unique_ptr<wxMenu> BuildMenu
      (wxEvtHandler *pParent, PopupMenuTable *pTable, void *pUserData = NULL);

   const TranslatableString &Caption() const { return mCaption; }

   // menu must have been built by BuildMenu
   // More items get added to the end of it
   static void ExtendMenu( wxMenu &menu, PopupMenuTable &otherTable );

   using Entries = std::vector<PopupMenuTableEntry>;
   const Entries& Get()
   {
      if (mContents.empty())
         Populate();
      return mContents;
   }

protected:
   virtual void Populate() = 0;
   void Clear() { mContents.clear(); }
   
   Entries mContents;
   TranslatableString mCaption;
};

/*
The following macros make it easy to attach a popup menu to a window.

Exmple of usage:

In class MyTable (maybe in the private section),
which inherits from PopupMenuTable,

DECLARE_POPUP_MENU(MyTable);
virtual void InitUserData(void *pUserData);
virtual void InitMenu(wxMenu *pMenu);
virtual void DestroyMenu();

Then in MyTable.cpp,

void MyTable::InitUserData(void *pUserData)
{
   // Remember pData
   auto pData = static_cast<MyData*>(pUserData);
}

void MyTable::InitMenu(wxMenu *pMenu)
{
   // enable or disable menu items
}

void MyTable::DestroyMenu()
{
   // Do cleanup
}

BEGIN_POPUP_MENU(MyTable)
   // This is inside a function and can contain arbitrary code.  But usually
   // you only need a sequence of macro calls:

   POPUP_MENU_ITEM("Cut", OnCutSelectedTextID,     XO("Cu&t"),          OnCutSelectedText)
   // etc.
 
END_POPUP_MENU()

where OnCutSelectedText is a (maybe private) member function of MyTable.

Elswhere,

MyTable myTable;
MyData data;
auto pMenu = PopupMenuTable::BuildMenu(pParent, &myTable, &data);

// Optionally:
OtherTable otherTable;
PopupMenuTable::ExtendMenu( *pMenu, otherTable );

pParent->PopupMenu(pMenu.get(), event.m_x, event.m_y);

That's all!
*/

#define DECLARE_POPUP_MENU(HandlerClass) \
   void Populate() override;

// begins function
#define BEGIN_POPUP_MENU(HandlerClass) \
void HandlerClass::Populate() { \
   using My = HandlerClass;

#define POPUP_MENU_APPEND(stringId, type, id, string, memFn, subTable) \
   mContents.push_back( Entry { \
      type, \
      id, \
      string, \
      memFn, \
      subTable \
   } );

#define POPUP_MENU_APPEND_ITEM(stringId, type, id, string, memFn) \
   POPUP_MENU_APPEND( stringId, \
      type, \
      id, \
      string, \
      (wxCommandEventFunction) (&My::memFn), \
      nullptr )

#define POPUP_MENU_ITEM(stringId, id, string, memFn) \
   POPUP_MENU_APPEND_ITEM(stringId, Entry::Item, id, string, memFn);

#define POPUP_MENU_RADIO_ITEM(stringId, id, string, memFn) \
   POPUP_MENU_APPEND_ITEM(stringId, Entry::RadioItem, id, string, memFn);

#define POPUP_MENU_CHECK_ITEM(stringId, id, string, memFn) \
   POPUP_MENU_APPEND_ITEM(stringId, Entry::CheckItem, id, string, memFn);

// classname names a class that derives from MenuTable and defines Instance()
#define POPUP_MENU_SUB_MENU(stringId, classname) \
   POPUP_MENU_APPEND( stringId, \
      Entry::SubMenu, -1, classname::Instance().Caption(), nullptr, &classname::Instance() );

#define BEGIN_POPUP_MENU_SECTION( name ) \
   POPUP_MENU_APPEND( "", \
      Entry::Separator, -1, {}, nullptr, nullptr );

#define END_POPUP_MENU_SECTION()

// ends function
#define END_POPUP_MENU() \
   POPUP_MENU_APPEND( "", \
      Entry::Invalid, -1, {}, nullptr, nullptr ) \
   }

#endif
