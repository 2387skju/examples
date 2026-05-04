/***
* Example / Demo:
* To get all characers with the same width.
* e.g. convert proportional font to monospace font.
*
* Addational here (for the self-written part):
* - This Version here only make a-Z0-9 to monospace. Or alternative optional also all characters.
*
* Here a two solutions:
* One build-in from font, and other self-written.
*  1. Use of tabular numbers:
*     - works only when supported by the font (will not be checked here)
*     - this font-feature is named: tabular numbers (or tnum=1)
*     - all digits will have the same width
*     - the digits may differ from the normal ones
*     - you should prefer this solution (i think)
*  2. Manipulate the glpyh's to convert the proportional font to a monospace font:
*     - all digits have the same width
*     - the digits does NOT differ from the normal ones
*
* This Version here only make the digits to monospace.
* To run this example you must have install font "Cantarell Regular", or set any other proportional font as your Application default font.
*
* Licence (if needed): GPLv3 or higher
**/

/**
* JUST A DEMO :
*   This is just an Example / Demonstration :
*   So it's currently more or less a draft. It may contain memory leaks, or other problems in detail.
**/


// compile && run:
// gcc force_monospace_abc123_v3-1.c `pkg-config --libs --cflags gtk+-3.0` && ./a.out

// valgrind --tool=memcheck --leak-check=full ./a.out


#include <gtk/gtk.h>

// test-string the measure the maximal possible length of a character
#define TEST_STRING        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define TEST_STRING_LENGTH        62
#define FORCE_CONVERT_ALL_CHAR        FALSE //(default FALSE) ; TRUE = use the width from the test-string for ANY character.


static void
font_check_teststring_same_width (PangoLayout *layout,
                        int *max_width,
                        guint32 glyphsNumIds[TEST_STRING_LENGTH],
                        gboolean *process_is_needed )
{
  PangoLayout *layout_copy = pango_layout_copy (layout);
  *max_width = 0;
  *process_is_needed = FALSE;
  GSList *l, *r;
  pango_layout_set_text (layout_copy, TEST_STRING, -1);
  PangoGlyphString *glyphs;
  for (l = pango_layout_get_lines_readonly (layout_copy); l; l = l->next)
  {
    PangoLayoutLine *line = l->data;
    for (r = line->runs; r; r = r->next)
	  {
	    glyphs = ((PangoGlyphItem *)r->data)->glyphs;
	    int i;
	    for (i = 0; i < glyphs->num_glyphs; i++)
	    {
	      int *width = &glyphs->glyphs[i].geometry.width;
	      // g_print("digit width:%i\n",*width);
	      if (*width != *max_width)
	      {
	        if (*max_width > 0) *process_is_needed = TRUE; //at least two different width exists
	        if (*width > *max_width)
	          *max_width = *width;
	      }
	      *glyphsNumIds = glyphs->glyphs[i].glyph;
	      if (i < TEST_STRING_LENGTH)
	        glyphsNumIds++;
	    }
	  }
  }

  g_print("Max width of the abc123 is: %i\n", *max_width);
  g_print("Process for this font is:");
  if ( *process_is_needed == TRUE) g_print(" needed.\n");
  if ( *process_is_needed == FALSE) g_print(" NOT needed.\n");

  g_object_unref (layout_copy);
}


static void
convert_to_same_width (GtkWidget *label,
                        const char align,
                        int *max_width,
                        guint32 glyphsNumIds[TEST_STRING_LENGTH])
{

  PangoLayout *layout;
  const char *text = gtk_label_get_text (GTK_LABEL (label));
  layout = gtk_label_get_layout (GTK_LABEL (label));

  if (max_width <= 0 ) return;

  GSList *l, *r;
  for (l = pango_layout_get_lines (layout); l; l = l->next)
  {
    PangoLayoutLine *line = l->data;

    for (r = line->runs; r; r = r->next)
    {
      PangoGlyphString *glyphs = ((PangoGlyphItem *)r->data)->glyphs;
      int i;

      for (i = 0; i < glyphs->num_glyphs; i++)
      {
        gboolean test_string_char_found = FALSE;
        int k;
        for (k = 0; k < TEST_STRING_LENGTH; k++)
          if ( glyphsNumIds[k] == glyphs->glyphs[i].glyph) test_string_char_found = TRUE;
        if (test_string_char_found == FALSE && !FORCE_CONVERT_ALL_CHAR) continue;

        int *width = &glyphs->glyphs[i].geometry.width;
        if (max_width > 0 &&
            *width < *max_width)
        {
          char align_cur = align;
          int diff = *max_width - *width;
          if (align_cur == 'c' || align_cur == 'r' )
          {
            if (diff > 0)
            {
              if (align_cur == 'r')
                glyphs->glyphs[i].geometry.x_offset += diff;
              if (align_cur == 'c' &&
                  diff/2 >0 )
                glyphs->glyphs[i].geometry.x_offset += diff/2;
            }
          }
          if( *max_width > 0)
            *width = *max_width;
        }
      }
    }
  }

    // update size (in case the modified font is more or less width, then before)
    // int width;
    // int height;
    // pango_layout_get_pixel_size (layout, &width, &height);
    // gtk_widget_set_size_request (GTK_WIDGET (label), width, height);
    // or:
    gtk_widget_queue_resize(GTK_WIDGET (label));

}


int main (int argc, char **argv) {

  GtkWidget *window;
  GtkWidget *hpane, *vpane, *box;
  GtkWidget *labelWelcomeHead, *labelHead, *labelRaw, *label0, *label1, *label2, *label3, *label_desc;
  PangoLayout *layout;
  gtk_init (&argc, &argv);
  const char* text = "01234iiw589\n115iiw;:12iwd12";//"_11:11:11_";//"_17:07:21_";;//"17:01:14";//"Fr Apr 24\n14:35:14";//01234iiw589\n115iiw12iwd12";

  /*
    // Here an alternative way to set tabular numbers ("tnum"):
    label0 = gtk_label_new (text);
    const char* text_markup;
    text_markup = g_strconcat ("<span font_features='tnum=1'>", text, "</span>", NULL); //NULL terminate needed
    works: gtk_label_set_markup(GTK_LABEL(label0),text_markup);

    //or other alternative: via css  #my_label { font-feature-settings: "tnum";  }
  */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  box = gtk_list_box_new();
  hpane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  vpane = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

  labelWelcomeHead = gtk_label_new ("Example demo: \nConvert proportional font to monospace font \n(= with the same width), but only for A-Z and digits");
  labelHead = gtk_label_new ("compare:");
  labelRaw = gtk_label_new (text);
  label0 = gtk_label_new (text);
  label1 = gtk_label_new (text);
  label2 = gtk_label_new (text);
  label3 = gtk_label_new (text);
  label_desc = gtk_label_new ("\nRaw\nRaw + tnum=1\nAlign Left\nAlign Center\nAlign Right");

  gtk_container_add (GTK_CONTAINER(box), labelHead);
  gtk_container_add (GTK_CONTAINER(box), labelRaw);
  gtk_container_add (GTK_CONTAINER(box), label0);
  gtk_container_add (GTK_CONTAINER(box), label1);
  gtk_container_add (GTK_CONTAINER(box), label2);
  gtk_container_add (GTK_CONTAINER(box), label3);

  gtk_paned_add1 (GTK_PANED(hpane), box);
  gtk_paned_add2 (GTK_PANED(hpane), label_desc);

  gtk_paned_add1 (GTK_PANED(vpane), labelWelcomeHead);
  gtk_paned_add2 (GTK_PANED(vpane), hpane);
  gtk_container_add (GTK_CONTAINER(window), vpane);
  gtk_widget_show_all (window);

  // select a font, where digits have different widths e.g. the font Cantarell:
  PangoFontDescription *desc = pango_font_description_from_string ("Cantarell 11");// NOT: "Monospace 11" or "Ubuntu Regular 11"
  PangoFontDescription *descWithTnum = pango_font_description_copy (desc);

  /* set tabular numbers (tnum) */
  pango_font_description_set_features (descWithTnum, "tnum=1"); // or for testing only: pango_font_description_set_features_static
  /*
    // or, here an alternative way to set tabular numbers (tnum) (without set the font):
    // set tabular numbers (tnum) (alternative)
    PangoAttribute *attr;
    PangoAttrList *alist;
    attr = pango_attr_font_features_new ("tnum=1");
    alist = pango_attr_list_new ();
    pango_attr_list_insert (alist, attr);
    gtk_label_set_attributes (GTK_LABEL (label0), alist);
    pango_attr_list_unref (alist);
  */

  PangoContext *context;

  context = gtk_widget_get_pango_context (labelHead);
  pango_context_set_font_description (context, desc);
  context = gtk_widget_get_pango_context (labelRaw);
  pango_context_set_font_description (context, desc);

  context = gtk_widget_get_pango_context (label0);
  pango_context_set_font_description (context, descWithTnum);

  context = gtk_widget_get_pango_context (label1);
  pango_context_set_font_description (context, desc);
  context = gtk_widget_get_pango_context (label2);
  pango_context_set_font_description (context, desc);
  context = gtk_widget_get_pango_context (label3);
  pango_context_set_font_description (context, desc);


  /*
  * check a label the test-text-string and detect the width
  */
  int max_width = 0;
  gboolean process_is_needed = FALSE;
  guint32 glyphsNumIds[ TEST_STRING_LENGTH ];
  layout = gtk_label_get_layout (GTK_LABEL (label1));
  font_check_teststring_same_width (layout, &max_width, glyphsNumIds, &process_is_needed);

  if (process_is_needed == FALSE)
  {
    const char *textNotWork = "Warning!!! : This Demo does NOT work your system.\nYou need to have install the font Cantarell. \nAlternative: set your application default font to a proportional font.";
    g_print(textNotWork);
    gtk_label_set_text (GTK_LABEL(labelWelcomeHead),textNotWork);
  }


  if (process_is_needed == TRUE)
  {
    convert_to_same_width (label1, 'l', &max_width, glyphsNumIds);
    convert_to_same_width (label2, 'c', &max_width, glyphsNumIds);
    convert_to_same_width (label3, 'r', &max_width, glyphsNumIds);
  }

  g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_main ();
  return 0;
}
