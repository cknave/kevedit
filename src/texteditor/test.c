
#include "lineeditor.h"
#include "display/display.h"
#include "display/colours.h"

int main(int argc, char *argv[])
{
	displaymethod * d;
	lineeditor * editor;

	RegisterDisplays();
	d = &display;

	d->init();

	editor = createlineeditor(2, 5, "Foo bar", 16, d);
	editor->visiblewidth = 10;
	editor->colour = BLUE_B | WHITE_F | BRIGHT_F;
	editline(editor);
	deletelineeditor(editor);

	d->end();
	
	return 0;
}

