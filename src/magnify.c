/*
 * Lightweight screen magnifier
 *
 * Reference documentation
 *
 * The Xlib manual: https://tronche.com/gui/x/xlib/
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

bool running = true;
int screen;
int display_width;
int display_height;
int depth;
Window root;
Window window;
Visual *visual;
Display *display;
GC gc;

int x = 0;
int y = 0;

uint8_t ratio = 4;
int32_t capture = 128;

#define padding 2
#define box_size 512
#define total_box_size (box_size + (padding * 2))

Window target;

uint8_t image_data[box_size * box_size * 4];

void put_image(void)
{
	int lx = x;
	int ly = y;

	if (lx > (capture / 2)) {
		lx -= (capture / 2);
	} else {
		lx = 0;
	}

	if (ly > (capture / 2)) {
		ly -= (capture / 2);
	} else {
		ly = 0;
	}

	lx = lx + capture >= display_width ? display_width - capture : lx;
	ly = ly + capture >= display_height ? display_height - capture : ly;

	XImage *img = XGetImage(display, root, lx, ly, (unsigned int)capture,
				(unsigned int)capture, AllPlanes, ZPixmap);

	for (int sy = 0; sy < capture; sy++) {
		for (int sx = 0; sx < capture; sx++) {
			uint32_t i = (unsigned int)(sy * capture + sx) * 4;

			uint32_t base_px =
				(unsigned int)(sy * box_size * ratio) +
				(unsigned int)(sx * ratio);
			for (uint8_t ry = 0; ry < ratio; ry++) {
				uint32_t bn = (base_px + (ry * box_size)) * 4;
				for (uint8_t rx = 0; rx < ratio; rx++) {
					image_data[bn + 0] =
						(unsigned char)img->data[i + 0];
					image_data[bn + 1] =
						(unsigned char)img->data[i + 1];
					image_data[bn + 2] =
						(unsigned char)img->data[i + 2];
					bn += 4;
				}
			}
		}
	}

	XImage *img_2 = XCreateImage(display, visual, (unsigned int)depth,
				     ZPixmap, 0, (char *)image_data, box_size,
				     box_size, 32, 0);

	XPutImage(display, window, gc, img_2, 0, 0, padding, padding, box_size,
		  box_size);

	XDestroyImage(img);
}

int main(void)
{
	if ((display = XOpenDisplay(NULL)) == NULL) {
		printf("Can't open display...\n");
		return -1;
	}

	screen = DefaultScreen(display);
	display_width = DisplayWidth(display, screen);
	display_height = DisplayHeight(display, screen);
	visual = DefaultVisual(display, screen);
	depth = DefaultDepth(display, screen);
	root = DefaultRootWindow(display);

	window = XCreateSimpleWindow(display, root,
				     display_width - (total_box_size + 10),
				     display_height - (total_box_size + 10),
				     box_size + padding, box_size + padding, 0,
				     0, 0x000000);

	gc = XCreateGC(display, window, 0, NULL);

	XSizeHints sizehint;
	sizehint.min_width = sizehint.max_width = total_box_size;
	sizehint.min_height = sizehint.max_height = total_box_size;
	sizehint.flags = PMinSize | PMaxSize;
	XSetWMNormalHints(display, window, &sizehint);

	XStoreName(display, window, "Magnify");
	XMapWindow(display, window);
	XSelectInput(display, window,
		     KeyPressMask | ExposureMask | PointerMotionMask |
			     ButtonPressMask);
	XFlush(display);

	Cursor cursor = XCreateFontCursor(display, 68);

	// Attempt to grab the pointer
	int grab_result = XGrabPointer(display, root, True,
				       PointerMotionMask | ButtonPressMask,
				       GrabModeAsync, GrabModeAsync, None,
				       cursor, CurrentTime);

	if (grab_result != GrabSuccess) {
		fprintf(stderr,
			"Warning: Failed to grab pointer (%d). Continuing anyway.\n",
			grab_result);
		// continue running without pointer grab
	}

	// Optionally, grab the key
	XGrabKey(display, 24, 0, root, True, GrabModeAsync, GrabModeAsync);

	Window mag_window;
	int i;
	uint32_t m = 0;

	XQueryPointer(display, root, &mag_window, &target, &x, &y, &i, &i, &m);
	XGrabPointer(display, root, true, PointerMotionMask | ButtonPressMask,
		     GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);

	XGrabKey(display, 24, 0, root, true, GrabModeAsync, GrabModeAsync);

	XEvent ev;
	int last_x = -1, last_y = -1;

	while (running) {
		// 1. Poll cursor position every loop
		Window child;
		unsigned int mask;
		int root_x, root_y, win_x, win_y;

		XQueryPointer(display, root, &root, &child, &root_x, &root_y,
			      &win_x, &win_y, &mask);
		if (root_x != last_x || root_y != last_y) {
			x = root_x;
			y = root_y;
			put_image();
			last_x = root_x;
			last_y = root_y;
		}

		// 2. Handle all pending events
		while (XPending(display)) {
			XNextEvent(display, &ev);
			switch (ev.type) {
			case KeyPress:
			case KeyRelease:
				switch (ev.xkey.keycode) {
				case 9: // ESC
				case 24: // q
					running = false;
					break;
				case 46:
				case 114:
					x++;
					put_image();
					break; // Right
				case 43:
				case 113:
					x--;
					put_image();
					break; // Left
				case 45:
				case 111:
					y--;
					put_image();
					break; // Up
				case 44:
				case 116:
					y++;
					put_image();
					break; // Down
				}
				break;

			case Expose:
				put_image();
				break;

			case ButtonPress:
				x = ev.xbutton.x_root;
				y = ev.xbutton.y_root;

				if (ev.xbutton.button == 4) { // scroll up
					if (capture <= 4)
						break;
					ratio *= 2;
					capture = box_size / ratio;
					put_image();
				} else if (ev.xbutton.button ==
					   5) { // scroll down
					if (ratio < 2)
						break;
					ratio /= 2;
					capture = box_size / ratio;
					put_image();
				} else if (ev.xbutton.button ==
					   3) { // right click center
					int32_t mx = ev.xbutton.x_root -
						     box_size / 2;
					int32_t my = ev.xbutton.y_root -
						     box_size / 2;
					if (mx < 10)
						mx = 10;
					if (my < 10)
						my = 10;
					if (mx > display_width - box_size - 10)
						mx = display_width - box_size -
						     10;
					if (my > display_height - box_size - 10)
						my = display_height - box_size -
						     10;
					XMoveWindow(display, window, mx, my);
				} else {
					running = false;
				}
				break;
			}
		}

		// Tiny sleep to prevent 100% CPU
		usleep(1000);
	}

	XDestroyWindow(display, window);
	XCloseDisplay(display);
	return 0;
}
