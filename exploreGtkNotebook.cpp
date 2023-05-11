#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

GtkNotebook *g_Notebook;
GtkLabel *g_lblPageMessage;

GtkWidget *g_pNotebookPages[256];

typedef struct
{
	int iPageId;
	GtkEntry *txtPageEntryField;
}PageInfo;

int g_iPageCount = 0;
PageInfo *g_pPageInfo[256];
GtkEventBox *g_gtkEventBoxes[256];
GtkLabel *g_gtkLabels[256];

GtkWidget *g_mnuRemovePage[256];
GtkWidget *g_mnuItemRemove[256];

int g_iAvailablePageNumbers[256];
int g_iAvailablePageIdx=-1;	//Using zero based indexing
GError *g_Error = nullptr;

void OnMnuRemovePage(GtkWidget* self, gpointer pageInfo)
{
	PageInfo *p = (PageInfo*)pageInfo;
	gtk_notebook_remove_page(g_Notebook, p->iPageId);
	g_iAvailablePageNumbers[++g_iAvailablePageIdx] = p->iPageId;

	//g_iPageCount--;

	delete(g_pPageInfo[p->iPageId]);
	//gtk_widget_destroy((GtkWidget*)g_gtkLabels[p->iPageId]);
	//gtk_widget_destroy((GtkWidget*)g_gtkEventBoxes[p->iPageId]);
	//gtk_widget_destroy(g_mnuRemovePage[p->iPageId]);
	//gtk_widget_destroy(g_mnuItemRemove[p->iPageId]);
}

gboolean OnButtonPress(GtkWidget* self, GdkEvent *event, gpointer pPageInfo)
{
	if(event->button.button == 3)
	{
		PageInfo *p = (PageInfo*)pPageInfo;

		gtk_menu_popup_at_widget((GtkMenu*)g_mnuRemovePage[p->iPageId], (GtkWidget*)g_gtkLabels[p->iPageId], GDK_GRAVITY_EAST, GDK_GRAVITY_WEST, NULL);
	}
	return false;
}

static void OnPageButtonClicked(GtkButton *button, gpointer pPageInfo)
{
	PageInfo *p = (PageInfo*)pPageInfo;
	gtk_label_set_text(g_lblPageMessage, gtk_entry_get_text(p->txtPageEntryField));
}

void ForEachWidgetOnThePage(gpointer pageWidget, gpointer pPageInfo)
{
	const gchar *name = gtk_widget_get_name((GtkWidget*)pageWidget);

	//Set the label text on the Action Button
	if(!strcmp(name, "btnPageAction"))
	{
		PageInfo *p = (PageInfo*)pPageInfo;

		char szPageId[10];
		sprintf(szPageId, "Page %d", p->iPageId);
		gtk_button_set_label((GtkButton*)pageWidget, szPageId);

		PageInfo *pc = new PageInfo();
		pc->iPageId = p->iPageId;
		pc->txtPageEntryField = p->txtPageEntryField;
		g_signal_connect(pageWidget, "clicked", G_CALLBACK(OnPageButtonClicked), pc);
	}
}

static void OnBtnNewPageClicked(GtkButton *button)
{
	if(g_iPageCount > 255)
		return;

	const gchar *name = gtk_widget_get_name((GtkWidget*)button);
	if(!strcmp(name, "btnNewPage"))
	{
		//Create a new Notebook page
	  GtkBuilder *bldrPage = gtk_builder_new ();
	  gtk_builder_add_from_file(bldrPage, "notebookPage.xml", &g_Error);
	  GtkWidget *pNotebookPage = (GtkWidget*)gtk_builder_get_object(bldrPage, "NotebookPage");

	  //Enables a popup menu that is populated with the page
	  //titles so as to allow the user to switch between pages
	  //gtk_notebook_popup_enable(g_Notebook);

	  GtkEventBox *gtkEventBox = (GtkEventBox*)gtk_event_box_new();
	  GtkLabel *gtkLabel = (GtkLabel*)gtk_label_new("Page New");

	  int iNewPageId;
	  if(g_iAvailablePageIdx >= 0)
	  	iNewPageId = g_iAvailablePageNumbers[g_iAvailablePageIdx];
	  else
	  	iNewPageId = g_iPageCount++;

	  //iNewPageId should be = -1 under two conditions:
	  // 1.) There are no available Page Numbers because no pages have ever been
	  //     added or all previously allocated pages have been deleted. g_iPageCount == 0
	  //
	  // 2.) There are no available Page Numbers because no pages
	  //     have been deleted that have not been reallocated. g_iAvailablePageIdx == -1
	  gtk_notebook_insert_page(g_Notebook, pNotebookPage, (GtkWidget*)gtkEventBox, g_iAvailablePageIdx < 0?-1:g_iAvailablePageNumbers[g_iAvailablePageIdx--]);
	  g_gtkEventBoxes[iNewPageId] = gtkEventBox;
	  g_gtkLabels[iNewPageId] = gtkLabel;
	  g_pNotebookPages[iNewPageId] = pNotebookPage;

	  gtk_container_add((GtkContainer*)g_gtkEventBoxes[iNewPageId], (GtkWidget*)g_gtkLabels[iNewPageId]);
	  gtk_widget_show_all((GtkWidget*)g_gtkLabels[iNewPageId]);

	  char szPageId[10];
	  sprintf(szPageId, "Page %d", iNewPageId);

	  gtk_widget_set_name(pNotebookPage, szPageId);
	  gtk_label_set_text((GtkLabel*)gtkLabel, szPageId);

	  GList *pageWidgetSet = gtk_container_get_children(GTK_CONTAINER(pNotebookPage));

	  g_mnuRemovePage[iNewPageId] = gtk_menu_new();
	  g_mnuItemRemove[iNewPageId] = gtk_menu_item_new_with_label("Remove");

	  g_pPageInfo[iNewPageId] = new PageInfo();
	  g_pPageInfo[iNewPageId]->iPageId = iNewPageId;
	  g_pPageInfo[iNewPageId]->txtPageEntryField = (GtkEntry*)gtk_builder_get_object(bldrPage, "txtPageEntryField");

	  g_signal_connect(g_mnuItemRemove[iNewPageId], "activate", G_CALLBACK(OnMnuRemovePage), g_pPageInfo[iNewPageId]);
	  gtk_menu_attach((GtkMenu*)g_mnuRemovePage[iNewPageId], (GtkWidget*)g_mnuItemRemove[iNewPageId], 0, 1, 0, 1);
	  gtk_widget_show_all(g_mnuRemovePage[iNewPageId]);

	  g_signal_connect(g_gtkEventBoxes[iNewPageId], "button_press_event", G_CALLBACK(OnButtonPress), g_pPageInfo[iNewPageId]);

	  g_list_foreach(pageWidgetSet, ForEachWidgetOnThePage, g_pPageInfo[iNewPageId]);

	  g_object_unref(bldrPage);
	}
	const gchar *lbl = gtk_button_get_label(button);
	gtk_label_set_label(g_lblPageMessage, lbl);
}

static void OnPageAdded(GtkNotebook* self, GtkWidget* newPage, guint page_num, gpointer user_data)
{
	//Function is called before the gtk_notebook_append_page function completes and returns the new Page Id
}

static void activate(GtkApplication *app, gpointer user_data)
{
  //Construct a GtkBuilder instance and load the UI description
  GtkBuilder *bldrMain = gtk_builder_new ();

  gtk_builder_add_from_file(bldrMain, "exploreGtkNotebook.xml", &g_Error);

  //Connect signal handlers to the constructed widgets.
  GtkWidget *AppWindow = (GtkWidget*)gtk_builder_get_object(bldrMain, "appExploreGtkNotebook");
  gtk_window_set_application(GTK_WINDOW(AppWindow), app);

  //Place a button that can be used to add pages to the Notebook object
  GtkButton *btnNewPage = (GtkButton*)gtk_builder_get_object(bldrMain, "btnNewPage");
  g_signal_connect(btnNewPage, "clicked", G_CALLBACK(OnBtnNewPageClicked), NULL);

  //Place a label on the Main Window that can be populated by the Notebook Pages.
  //Note that "lblPageMessage" is what is defined in the Glade ID field (without quotes)
  g_lblPageMessage = (GtkLabel*)gtk_builder_get_object(bldrMain, "lblPageMessage");

  //Place an empty Notebook on the Main Page
  g_Notebook = (GtkNotebook*)gtk_builder_get_object(bldrMain, "gtkNotebook");
  gtk_notebook_set_scrollable(g_Notebook, true);
  g_signal_connect(g_Notebook, "page-added", G_CALLBACK(OnPageAdded), NULL);

  //Present the Main Window
  gtk_window_set_position((GtkWindow*)AppWindow, GTK_WIN_POS_CENTER);
  gtk_window_move((GtkWindow*)AppWindow, 300, 300);
  gtk_widget_show(GTK_WIDGET(AppWindow));

  //Note that this function retrieves the name of object as specified in the Glade Widget Name
  //field. The Widget Name can be specified or changed with the gtk_widget_set_name() function
  //const char *c = gtk_widget_get_name((GtkWidget*)g_lblPageMessage);

  gtk_widget_show_all((GtkWidget*)g_Notebook);

  //We do not need the builder object any more
  g_object_unref(bldrMain);
}

int main (int argc, char *argv[])
{
  GtkApplication *app = gtk_application_new("com.jamworkspro.exploregtknotebook", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);

  int status = g_application_run(G_APPLICATION (app), argc, argv);
  g_object_unref(app);

  return status;
}
