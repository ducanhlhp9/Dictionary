#include <gtk/gtk.h>
#include<stdio.h>																															
#include <stdlib.h>
#include <string.h>
#include "inc/btree.h"
#include "jrb.h"
#include <gdk/gdkkeysyms.h>

const gchar *a, *b;

GtkWidget *textView, *view1, *view2, *about_dialog, *entry_search;
GtkListStore *list;
BTA *book=NULL;

void set_textView_text(char * text)  // dua tu vao bo nho dem
{
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	if (buffer == NULL)
		buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, text, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), buffer);
}

static char code[128] = { 0 };
const char* soundex(const char *s) // chuyen sang ma codex
{
	static char out[5];
	int c, prev, i;

	out[0] = out[4] = 0;
	if (!s || !*s) return out;

	out[0] = *s++;

	/* first letter, though not coded, can still affect next letter: Pfister */
	prev = code[(int)out[0]];
	for (i = 1; *s && i < 4; s++) {
		if ((c = code[(int) * s]) == prev) continue;

		if (c == -1) prev = 0;	/* vowel as separator */
		else if (c > 0) {
			out[i++] = c + '0';
			prev = c;
		}
	}
	while (i < 4) out[i++] = '0';
	return out;
}

int prefix(const char *big, const char *small) // tra lai 1 neu giong prefix, 0 neu nguoc lai
{
	int small_len = strlen(small);
	int big_len = strlen(big);
	int i;
	if (big_len < small_len)
		return 0;
	for (i = 0; i < small_len; i++)
		if (big[i] != small[i])
			return 0;
		return 1;    
	}

	int commond_char(char * str1, char * str2, int start)
	{
		int i;
		int slen1 = strlen(str1);
		int slen2 = strlen(str2);
		int slen  = (slen1 < slen2) ? slen1 : slen2;
		for ( i = start; i < slen; i++)
			if (str1[i] != str2[i])
				return i;
			return i;
		}

     void jrb_to_list(JRB nextWordArray, int number) // 
     {
     	GtkTreeIter Iter;
     	JRB tmp;
     	int sochia = number / 9;
     	int max = 8;
     	if (sochia == 0) sochia = 1;
     	jrb_traverse(tmp, nextWordArray) {
     		if ((number--) % sochia == 0)  {
     			gtk_list_store_append(GTK_LIST_STORE(list), &Iter);
     			gtk_list_store_set(GTK_LIST_STORE(list), &Iter, 0, jval_s(tmp->key), -1 );
     			if (max-- < 1)
     				return;
     		}
     	}
     }

int insert_insoundexlist(char * soundexlist , char * newword,  char * word, char * soundexWord) // tao newword voi ma soundex
{
	if (strcmp(soundexWord, soundex(newword)) == 0) {
		if (strcmp(newword, word) != 0) {
			strcat(soundexlist, newword);
			strcat(soundexlist, "\n");
			return 1;
		}
	}
	else
		return 0;
}

void suggest(char * word, gboolean Tab_pressed) // suggest, dua vao prefix, dung JRB to list ~
{

	char nextword[100], prevword[100];
	int i, NumOfCommondChar, minNumOfCommondChar = 1000;
	int max;
	GtkTreeIter Iter;
	JRB tmp, nextWordArray = make_jrb();
	BTint value, existed = 0;
	strcpy(nextword, word);
	int wordlen = strlen(word);
	gtk_list_store_clear(GTK_LIST_STORE(list));
	if (bfndky(book, word, &value) ==  0) { // tim word trong book, value la gia tri cua 'word' tim duoc
		existed = 1;
		gtk_list_store_append(GTK_LIST_STORE(list), &Iter);
		gtk_list_store_set(GTK_LIST_STORE(list), &Iter, 0, nextword, -1 ); // neu dung thi ok
	}
	if (!existed)
		btins(book, nextword, "", 1); // chen key va data vao B-tree, o lay chen blank vao !

	for (i = 0; i < 1945; i++) {
		bnxtky(book, nextword, &value);  // tim 'key' tiep theo
		if (prefix(nextword, word)) { // tim nhung tu co prefix giong
			jrb_insert_str(nextWordArray, strdup(nextword), JNULL);  // va chen vao array nextword (de show ra list -> jrb_to_list)
		}
		else break;
	}

	if (!existed && Tab_pressed) { // an tab de ra tu goi y xD !
		if (jrb_empty(nextWordArray)) {
			char soundexlist[1000] = "Ý bạn là:\n";
			char soundexWord[50];
			strcpy(nextword, word);
			strcpy(prevword, word);
			strcpy(soundexWord, soundex(word)); // soundex dung de tim tu 'xung quanh'
			max = 5;
			for (i = 0; (i < 10000 ) && max; i++) {
				if (bprvky(book , prevword, &value) == 0)
					if (insert_insoundexlist(soundexlist, prevword, word, soundexWord))
						max--;
				}
				max = 5;
				for (i = 0; (i < 10000 ) && max; i++) {
					if (bnxtky(book, nextword, &value) == 0)
						if (insert_insoundexlist(soundexlist, nextword, word, soundexWord))
							max--;
					}
					set_textView_text(soundexlist);
				}
				else {
					strcpy(nextword, jval_s(jrb_first(nextWordArray)->key));
					jrb_traverse(tmp, nextWordArray) {
						NumOfCommondChar = commond_char(nextword, jval_s(tmp->key), wordlen);
						if (minNumOfCommondChar > NumOfCommondChar)
							minNumOfCommondChar = NumOfCommondChar;
					}

					if ((minNumOfCommondChar  != 1000) && (minNumOfCommondChar > wordlen)) {
						nextword[NumOfCommondChar] = '\0';
						gtk_entry_set_text(GTK_ENTRY(entry_search), nextword);
						gtk_editable_set_position(GTK_EDITABLE(entry_search), NumOfCommondChar);
					}
				}

			}
			else
				jrb_to_list(nextWordArray, i);
			if (!existed)
				btdel(book, word);
			jrb_free_tree(nextWordArray);

		}

gboolean search_suggest(GtkWidget * entry, GdkEvent * event, gpointer No_need) // gioi han ky tu, chi nhan alphabelt va tab
{
	GdkEventKey *keyEvent = (GdkEventKey *)event;
	char word[50];
	int len;
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(entry_search)));
	if (keyEvent->keyval == GDK_KEY_Tab) {
		suggest(word,  TRUE);
	}
	else {
		if (keyEvent->keyval != GDK_KEY_BackSpace) {
			len = strlen(word);
			word[len] = keyEvent->keyval;
			word[len + 1] = '\0';
		}
		else {
			len = strlen(word);
			word[len - 1] = '\0';
		}
		suggest(word, FALSE);
	}
	return FALSE;
}

void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) // dua ra thong bao 
{
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		type,
		GTK_BUTTONS_OK,
		"%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void close_window(GtkWidget *widget, gpointer window) {
	gtk_widget_destroy(GTK_WIDGET(window));
}

int btfind(char * word) // dung cho ham search
{
	char mean[100000];
	int size;

	if(btsel(book,word,mean,100000,&size)==0)
	{
        //gtk_label_set_text(label_print, mean);
		set_textView_text(mean);
		g_print(mean);
		return 1;
	}
    else //gtk_label_set_text(label_print, "Không tồn tại ");
    {
        //set_textView_text("Không tồn tại ");
    	return 0;
    }
}

// VIEW DOWN HERE !!!

static void search(GtkWidget *w, gpointer data) // ham chuc nang search
{
	GtkWidget *entry1= ((GtkWidget**)data)[0];
	GtkWidget *window1=((GtkWidget**)data)[1];

	a = gtk_entry_get_text(GTK_ENTRY(entry1));
	g_print("%s\n",a);
	char word[50];
	BTint x;

	strcpy(word,a);
	if (word[0] == '\0')
		Show_message(GTK_WINDOW(window1), GTK_MESSAGE_WARNING, "Cảnh báo!", "Cần nhập từ để tra.");
	else
	{
		int result = btfind(word);
		if (result==0)
			Show_message(GTK_WINDOW(window1),GTK_MESSAGE_ERROR, "Xảy ra lỗi!","Không tìm thấy từ này trong từ điển.");
	}
	return;
}

static void edit( GtkWidget *w, gpointer data ) // ham chuc nang edit
{
	GtkWidget *entry1= ((GtkWidget**)data)[0];
	GtkWidget *window1=((GtkWidget**)data)[1];
	GtkWidget *edit_view=((GtkWidget**)data)[2];

	BTint x;

	if (gtk_entry_get_text(GTK_ENTRY(entry1))[0] == 0 || bfndky(book, (char*)gtk_entry_get_text(GTK_ENTRY(entry1)), &x) != 0)
	{
		Show_message(window1, GTK_MESSAGE_INFO, "Cách dùng:", "Phải tìm kiếm trước khi sửa."); // show_message o tren
		return;
	}

	a = gtk_entry_get_text(GTK_ENTRY(entry1));
	g_print("%s\n",a);
	char word[50],mean[100000];
	strcpy(word,a);

	GtkTextBuffer *buffer2;
	GtkTextIter start, end,iter;

	buffer2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(edit_view)));
	gtk_text_buffer_get_iter_at_offset(buffer2, &iter, 0);

	gtk_text_buffer_insert(buffer2, &iter, "", -1);
	gtk_text_buffer_get_bounds (buffer2, &start, &end);
	b = gtk_text_buffer_get_text (buffer2, &start, &end, FALSE);

	strcpy(mean,b);

	if (word[0] == '\0' || mean[0] == '\0')
		Show_message(GTK_WINDOW(window1), GTK_MESSAGE_WARNING, "Cảnh báo!", "Không được bỏ trống phần nào.");
	else if (bfndky(book, word, &x ) != 0)
		Show_message(GTK_WINDOW(window1), GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập không có trong từ điển.");
	else
	{
		if( btupd(book, word, mean, strlen(mean) + 1)==1)
			Show_message(GTK_WINDOW(window1),GTK_MESSAGE_ERROR, "Xảy ra lỗi!","Không thể cập nhật từ.");
		else
			Show_message(GTK_WINDOW(window1),GTK_MESSAGE_INFO, "Thành công!","Đã cập nhật từ.");
	}

}

static void add(GtkWidget *w, gpointer data)
{

	GtkTextBuffer *buffer1,*buffer2;
	GtkTextIter start, end,iter;
	char mean[10000], word[50];
	buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(view1)));
	gtk_text_buffer_get_iter_at_offset(buffer1, &iter, 0);

	gtk_text_buffer_insert(buffer1, &iter, "", -1);
	gtk_text_buffer_get_bounds (buffer1, &start, &end);
	b = gtk_text_buffer_get_text (buffer1, &start, &end, FALSE);

	strcpy(word,b);
	g_print(word);

	buffer2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GTK_TEXT_VIEW(view2)));
	gtk_text_buffer_get_iter_at_offset(buffer2, &iter, 0);

	gtk_text_buffer_insert(buffer2, &iter, "", -1);
	gtk_text_buffer_get_bounds (buffer2, &start, &end);
	b = gtk_text_buffer_get_text (buffer2, &start, &end, FALSE);

	strcpy(mean,b);
	printf("\n");
	g_print(mean);

	BTint x;

	if (word[0] == '\0' || mean[0] == '\0')
		Show_message(GTK_WINDOW(data), GTK_MESSAGE_WARNING, "Cảnh báo!", "Không được bỏ trống phần nào.");
	else if (bfndky(book, word, &x ) == 0)
		Show_message(GTK_WINDOW(data), GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập đã có trong từ điển.");
	else
	{
		if(btins(book,word, mean,10000))
			Show_message(GTK_WINDOW(data),GTK_MESSAGE_ERROR, "Xảy ra lỗi!","Không thể thêm vào từ điển.");
		else
			Show_message(GTK_WINDOW(data),GTK_MESSAGE_INFO, "Thành công!","Đã thêm vào từ điển.");
	}

	return;
}

static void del(GtkWidget *w, gpointer data)
{
	GtkWidget *entry1= ((GtkWidget**)data)[0];
	GtkWidget *window1=((GtkWidget**)data)[1];

	a = gtk_entry_get_text(entry1);
	g_print("%s\n",a);
	char mean[10000], word[50];
	int size;
	BTint x;
	strcpy(word,a);
	if (word[0] == '\0')
		Show_message(GTK_WINDOW(window1), GTK_MESSAGE_WARNING, "Cảnh báo!", "Cần nhập từ muốn xoá.");
	else if (bfndky(book, word, &x ) != 0)
		Show_message(GTK_WINDOW(window1), GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập không có trong từ điển.");
	else
		if(btsel(book,word,mean,100000,&size)==0)
		{
			btdel(book,word);
			Show_message(GTK_WINDOW(window1),GTK_MESSAGE_INFO, "Thành công!","Đã xoá từ khỏi từ điển.");

		}
		else
			Show_message(GTK_WINDOW(window1),GTK_MESSAGE_ERROR, "Xảy ra lỗi!","Không thể xoá từ khỏi từ điển.");


		return;
	}

	void tra_tu(GtkWidget widget, gpointer window)
	{
		GtkWidget *table,*fixed;
		GtkWidget *btn1,*window1,*label,*entry,*btn2,*btn3,*label2,*label3;

		window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window1), "Tra từ");
		gtk_window_set_default_size(GTK_WINDOW(window1), 750, 400);
		gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

		fixed = gtk_fixed_new();
		gtk_container_add(GTK_CONTAINER(window1), fixed);

		table = gtk_table_new(2,4,FALSE);
		gtk_container_add(GTK_CONTAINER(fixed), table);

		label = gtk_label_new("Nhập:");
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		entry_search = gtk_entry_new();
		gtk_widget_set_size_request(entry_search, 300, 30);
		gtk_entry_set_max_length(GTK_ENTRY(entry_search),100);

		gtk_table_attach(GTK_TABLE(table), entry_search, 1, 2, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		GtkWidget *comple = gtk_entry_completion_new();
		gtk_entry_completion_set_text_column(comple, 0);
		list = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING);
		gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
		gtk_entry_set_completion(GTK_ENTRY(entry_search), comple);

		btn1 = gtk_button_new_with_label("Tra");
		gtk_fixed_put(GTK_FIXED(fixed), btn1, 450, 15);
		gtk_widget_set_size_request(btn1, 80, 30);

		label2 = gtk_label_new("Nghĩa:");
		gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 1, 2,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 200);

		textView = gtk_text_view_new();
		gtk_widget_set_size_request(textView, 300, 300);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD);

		GtkWidget *scrolling = gtk_scrolled_window_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(scrolling), textView);

		gtk_table_attach(GTK_TABLE(table), scrolling, 1, 2, 1, 2,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		btn2 = gtk_button_new_with_label("Trở về");
		gtk_fixed_put(GTK_FIXED(fixed), btn2, 550, 15);
		gtk_widget_set_size_request(btn2, 80, 30);

		btn3 = gtk_button_new_with_label("Cập nhật nghĩa của từ");
		gtk_fixed_put(GTK_FIXED(fixed), btn3, 450, 100);
		gtk_widget_set_size_request(btn3, 180, 30);

		label3 = gtk_label_new("Cách dùng: \n\t Tra từ \n\t-> Sửa nghĩa của từ \n\t-> Cập nhật từ");
		gtk_fixed_put(GTK_FIXED(fixed), label3, 450, 150);

		GtkWidget *data[3];
		data[0]= entry_search;
		data[1]= window1;
		data[2]= textView;

		g_signal_connect(entry_search, "key-press-event", G_CALLBACK(search_suggest), NULL);

		g_signal_connect(G_OBJECT(entry_search), "activate", G_CALLBACK(search), data);

		g_signal_connect(G_OBJECT(btn1), "clicked", G_CALLBACK(search), data);

		g_signal_connect(G_OBJECT(btn2), "clicked", G_CALLBACK(close_window), window1);

		g_signal_connect(G_OBJECT(btn3), "clicked", G_CALLBACK(edit), data);

		g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all(window1);

		gtk_main();

		return;

	}

	void them_tu(GtkWidget widget, gpointer window)
	{
		GtkWidget *table,*fixed, *btn2;
		GtkWidget *btn1,*window1,*label1,*entry1,*label2,*entry2;

		window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window1), "Thêm từ");
		gtk_window_set_default_size(GTK_WINDOW(window1), 700, 400);
		gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

		fixed = gtk_fixed_new();
		gtk_container_add(GTK_CONTAINER(window1), fixed);

		table = gtk_table_new(2,4,FALSE);
		gtk_container_add(GTK_CONTAINER(fixed), table);

		label1 = gtk_label_new("Từ:");
		gtk_table_attach(GTK_TABLE(table), label1, 0, 1, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		view1 = gtk_text_view_new();
		gtk_widget_set_size_request(view1, 300, 20);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view1), GTK_WRAP_WORD);
		gtk_table_attach(GTK_TABLE(table), view1, 1, 2, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		label2 = gtk_label_new("Nghĩa:");
		gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 1, 2,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 200);

		view2 = gtk_text_view_new();
		gtk_widget_set_size_request(view2, 300, 300);
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view2), GTK_WRAP_WORD);

		GtkWidget *scrolling = gtk_scrolled_window_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(scrolling), view2);
		gtk_table_attach(GTK_TABLE(table), scrolling, 1, 2, 1, 2,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		btn1 = gtk_button_new_with_label("Thêm");
		gtk_fixed_put(GTK_FIXED(fixed), btn1, 450, 15);
		gtk_widget_set_size_request(btn1, 80, 30);

		g_signal_connect(G_OBJECT(btn1), "clicked", G_CALLBACK(add), NULL);

		btn2 = gtk_button_new_with_label("Trở về");
		gtk_fixed_put(GTK_FIXED(fixed), btn2, 550, 15);
		gtk_widget_set_size_request(btn2, 80, 30);

		g_signal_connect(G_OBJECT(btn2), "clicked", G_CALLBACK(close_window), window1);

		g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all(window1);

		gtk_main();

		return;

	}

	void xoa_tu(GtkWidget widget, gpointer window)
	{
		GtkWidget *table,*fixed;
		GtkWidget *btn1,*window1,*label,*entry,*btn2;

		window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window1), "Xoá từ");
		gtk_window_set_default_size(GTK_WINDOW(window1), 700, 150);
		gtk_window_set_position(GTK_WINDOW(window1), GTK_WIN_POS_CENTER);

		fixed = gtk_fixed_new();
		gtk_container_add(GTK_CONTAINER(window1), fixed);

		table = gtk_table_new(1,4,FALSE);
		gtk_container_add(GTK_CONTAINER(fixed), table);

		label = gtk_label_new("Nhập:");
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		entry = gtk_entry_new();
		gtk_widget_set_size_request(entry, 300, 30);
		gtk_entry_set_max_length(GTK_ENTRY(entry),100);
		gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 0, 1,GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 15, 15);

		btn1 = gtk_button_new_with_label("Xoá");
		gtk_fixed_put(GTK_FIXED(fixed), btn1, 450, 15);
		gtk_widget_set_size_request(btn1, 80, 30);

		GtkWidget *data[2];
		data[0]= entry;
		data[1]= window1;

		g_signal_connect(G_OBJECT(btn1), "clicked", G_CALLBACK(del), data);

		g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(del), data);

		btn2 = gtk_button_new_with_label("Trở về");
		gtk_fixed_put(GTK_FIXED(fixed), btn2, 550, 15);
		gtk_widget_set_size_request(btn2, 80, 30);

		g_signal_connect(G_OBJECT(btn2), "clicked", G_CALLBACK(close_window), window1);

		g_signal_connect(G_OBJECT(window1), "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all(window1);

		gtk_main();

		return;

	}

	void about(GtkWidget widget, gpointer window)
	{
		about_dialog = gtk_about_dialog_new();
		gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Từ điển Anh Việt");
		gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), "Hoang Duc Anh 20176688\n Vu Minh Hoang Anh 20176689\n ");
		gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),
			"https://www.facebook.com/hoangduc.anh.1420");
		gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about_dialog), "Contact for work.");     
		gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), "113 LHP");
		gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), NULL);
		gtk_dialog_run(GTK_DIALOG(about_dialog));
		gtk_widget_destroy(about_dialog);
	}

	int main(int argc, char *argv[])
	{
		btinit();
		book= btopn("words.dat",0,1);

    //GTK+
		GtkWidget *window;
		GtkWidget *fixed,*image;
		GtkWidget *btn1,*btn2,*btn3,*btn4,*btn5;

		gtk_init(&argc, &argv);

    //tao cua so
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window), "E-V Dictionary");
		gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //tao nen
		fixed = gtk_fixed_new();
		gtk_container_add(GTK_CONTAINER(window), fixed);

    //anh nen
		image = gtk_image_new_from_file("anh.jpg");
		gtk_container_add(GTK_CONTAINER(fixed), image);

		btn1 = gtk_button_new_with_label("Tra từ");
		gtk_fixed_put(GTK_FIXED(fixed), btn1, 20, 20);
		gtk_widget_set_size_request(btn1, 120, 50);
		gtk_widget_set_tooltip_text(btn1, "Search a word");
		g_signal_connect(G_OBJECT(btn1), "clicked", G_CALLBACK(tra_tu), NULL);

		btn2 = gtk_button_new_with_label("Thêm từ");
		gtk_fixed_put(GTK_FIXED(fixed), btn2, 20, 90);
		gtk_widget_set_size_request(btn2, 120, 50);
		gtk_widget_set_tooltip_text(btn2, "Add a word");
		g_signal_connect(G_OBJECT(btn2), "clicked", G_CALLBACK(them_tu), NULL);

		btn3 = gtk_button_new_with_label("Xoá từ");
		gtk_fixed_put(GTK_FIXED(fixed), btn3, 20, 160);
		gtk_widget_set_size_request(btn3, 120, 50);
		gtk_widget_set_tooltip_text(btn3, "Delete a word");
		g_signal_connect(G_OBJECT(btn3), "clicked", G_CALLBACK(xoa_tu), NULL);

		btn4 = gtk_button_new_with_label("Thông tin");
		gtk_fixed_put(GTK_FIXED(fixed), btn4, 20, 230);
		gtk_widget_set_size_request(btn4, 120, 50);
		g_signal_connect(G_OBJECT(btn4), "clicked", G_CALLBACK(about), NULL);

		btn5 = gtk_button_new_with_label("Thoát");
		gtk_fixed_put(GTK_FIXED(fixed), btn5, 20, 300);
		gtk_widget_set_size_request(btn5, 120, 50);
		gtk_widget_set_tooltip_text(btn5, "Exit");
		g_signal_connect(G_OBJECT(btn5), "clicked", G_CALLBACK(close_window), window);

		g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all(window);

		gtk_main();

		btcls(book);

		return 0;
	}
