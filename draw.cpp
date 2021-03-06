#include <algorithm>
#include <cstdio>
#include <iostream>
#include <SDL/SDL.h>
#include <vector>
#include "domain.h"
using namespace std;

SDL_Surface* screen;

#define BLACK 0x000000
#define RED   0xff0000
#define MGNTA 0xffff00
#define GREEN 0x00ff00
#define BLUE  0x0000ff
#define WHITE 0xffffff

int perm[] = {
	0,
	40,
	11,
	44,
	1,
	10,
	28,
	43,
	39,
	47,
	29,
	38,
	2,
	20,
	19,
	37,
	3,
	60,
	13,
	64,
	4,
	12,
	26,
	63,
	36,
	67,
	27,
	35,
	5,
	21,
	18,
	34,
	6,
	15,
	7,
	14,
	24,
	33,
	25,
	32,
	8,
	9,
	16,
	17,
	22,
	23,
	30,
	31,
	65,
	66,
	61,
	62,
	72,
	73,
	74,
	75,
	78,
	79,
	68,
	69,
	70,
	71,
	76,
	77,
	45,
	46,
	41,
	42,
	52,
	53,
	54,
	55,
	58,
	59,
	48,
	49,
	50,
	51,
	56,
	57,
};

void print_rect(const SDL_Rect& rect) {
	cout << rect.x << ", " << rect.y << " => " << rect.x << " x " << rect.y << endl;
}

enum Type {
	FULL, EDGE, VERTEX
};

void draw_element(int x, int y, int w, int h, int scale, int contour) {
	SDL_Rect rect = { Sint16(x*scale), Sint16(y*scale), Uint16(w*scale), Uint16(h*scale) };

	Type type;

	Uint32 color;
	if (w == 0 && h == 0) {
		type = VERTEX;
		rect.x -= 2;
		rect.y -= 2;
		rect.w = 4;
		rect.h = 4;
		color = BLUE;
	} else if (w == 0 || h == 0) {
		type = EDGE;
		(w == 0 ? rect.w : rect.h) = 1;
		color = WHITE;
	} else {
		rect.x++;
		rect.y++;
		rect.w -= 2;
		rect.h -= 2;
		type = FULL;
		color = RED;
	}
	SDL_FillRect(screen, &rect, color);

	if (type == FULL) {
		rect.x += contour;
		rect.y += contour;
		rect.w -= contour * 2;
		rect.h -= contour * 2;
		SDL_FillRect(screen, &rect, BLACK);
	}
}

void draw_element_clear(int x, int y, int w, int h, int scale, int contour) {
	SDL_Rect rect = { Sint16(x*scale), Sint16(y*scale), Uint16(w*scale), Uint16(h*scale) };

	Uint32 color;
	Type type;
	
	if (w == 0 && h == 0) {
		/*type = VERTEX;
		rect.x -= 2;
		rect.y -= 2;
		rect.w = 4;
		rect.h = 4;
		color = BLUE;*/
	} else if (w == 0 || h == 0) {
		type = EDGE;
		(w == 0 ? rect.w : rect.h) = 2;
		(w == 0 ? rect.x : rect.y) += 4;
		color = BLACK;
	} else {
		//rect.x++;
		//rect.y++;
		//rect.w -= 2;
		//rect.h -= 2;
		type = FULL;
		color = BLACK;
	}
	SDL_FillRect(screen, &rect, color);

	if (type == FULL) {
		rect.x += contour;
		rect.y += contour;
		rect.w -= contour * 2;
		rect.h -= contour * 2;
		SDL_FillRect(screen, &rect, WHITE);
	}
}

bool was_key_up(SDL_Event& event, int key) {
	return event.type == SDL_KEYUP && event.key.keysym.sym == key;
}

void put_pixel(int x, int y, Uint32 color) {
	SDL_Rect rect = { (Sint16)x, (Sint16)y, (Uint16)1, (Uint16)1 };
	SDL_FillRect(screen, &rect, color);
}

void draw_line(int x1, int y1, int x2, int y2, int scale) {
	x1 *= scale;
	y1 *= scale;
	x2 *= scale;
	y2 *= scale;
	for (int i = 50; i < 450; i++) {
		int x = x1 + (x2 - x1) * i / 500;
		int y = y1 + (y2 - y1) * i / 500;
		put_pixel(x, y, BLUE);
	}
}

bool wait_until_key_or_quit(int key) {
	while (true) {
		SDL_Event event;
		int res = SDL_PollEvent(&event);
		if (res && was_key_up(event, key)) {
			return false;
		}
		if (res && was_key_up(event, SDLK_ESCAPE)) {
			SDL_Quit();
			exit(0);
		}
		SDL_Delay(50);
	}
}

struct RectDef {
	int x, y, w, h;
};

vector<RectDef> rects;
int scale;

void redraw() {
	SDL_FillRect(screen, NULL, BLACK);
	for (auto& rect: rects) {
		draw_element(rect.x, rect.y, rect.w, rect.h, scale, 2);
	}
}

void redraw_clear() {
	SDL_FillRect(screen, NULL, WHITE);
	for (auto& rect: rects) {
		draw_element_clear(rect.x, rect.y, rect.w, rect.h, scale, 1);
	}
}

void draw_inside(int index) {
	const auto& r = rects[index];
	SDL_Rect inside = { Sint16(r.x*scale + 3), Sint16(r.y*scale + 3), Uint16(r.w*scale - 6), Uint16(r.h*scale - 6) };
	SDL_FillRect(screen, &inside, GREEN);
}

int main(int argc, char** argv) {
	int width = 512;
	int height = 512;

	enum InputFormat {
		PLAIN,
		NEIGHBORS,
		SUPPORTS,
		CLEAR,
		MATRIX
	} mode = PLAIN;

	if (argc >= 2) {
		bool any_format = true;
		string opt(argv[1]);
		if (opt == "-c" || opt == "--clear")
			mode = CLEAR;
		else if (opt == "-m" || opt == "--matrix")
			mode = MATRIX;
		else if (opt == "-n" || opt == "--neighbors")
			mode = NEIGHBORS;
		else if (opt == "-s" || opt == "--supports")
			mode = SUPPORTS;
		else
			any_format = false;
		if (any_format) {
			argc--;
			argv++;
		}
	}

	MeshShape mesh_shape = QUADRATIC;

	if (argc >= 2) {
		bool any_shape = true;
		string mesh(argv[1]);
		if (mesh == "--quadratid" || mesh == "-q")
			mesh_shape = QUADRATIC;
		else if (mesh == "--rectangular" || mesh == "-r")
			mesh_shape = RECTANGULAR;
		else
			any_shape = false;
		if (any_shape) {
			argc--;
			argv++;
		}
	}

	if (mesh_shape == RECTANGULAR)
		width = width * 3 / 2;

	int depth = argc == 1 ? 3 : atoi(argv[1]);
	if (mode == NEIGHBORS)
		scale = 32 >> depth;
	else
		scale = 256 >> depth;

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(width, height, 0, SDL_ANYFORMAT);
	SDL_WM_SetCaption("Esc to exit", NULL);

	int N;
	cin >> N;
	for (int i = 0; i < N; i++) {
		int left, right, up, down;
		cin >> left >> right >> up >> down;

		int w = right - left;
		int h = down - up;
		RectDef rect = { left, up, w, h };
		rects.push_back(rect);
	}
	if (mode == CLEAR) {
		redraw_clear();
		char bmp[100];
		sprintf(bmp, "mesh-%i.bmp", depth);
		SDL_SaveBMP(screen, bmp);
	} else if (mode != MATRIX) {
		redraw();
	}

	if (mode == NEIGHBORS) {
		int M;
		cin >> M;
		for (int i = 0; i < M; i++) {
			int left, up, right, down;
			cin >> left >> up >> right >> down;
			draw_line(left, up, right, down, scale);
		}
	} else if (mode == SUPPORTS) {
		int M;
		cin >> M;
		for (int i = 0; i < M; i++) {
			int x, y, cnt;
			cin >> x >> y >> cnt;
			cout << i << endl;
			for (int j = 0; j < cnt; j++) {
				int index;
				cin >> index;
				draw_inside(index);
			}
			SDL_Rect mid = { Sint16(x*scale - 3), Sint16(y*scale - 3), 6, 6};
			SDL_FillRect(screen, &mid, MGNTA);
			SDL_Flip(screen);
			if (wait_until_key_or_quit(SDLK_SPACE)) {
				SDL_Quit();
				return 0;
			}
			redraw();
		}
	} else if (mode == MATRIX) {
		int M;
		cin >> M;
		vector<vector<int>> supports(M);

		for (int i = 0; i < M; i++) {
			int x, y, cnt;
			cin >> x >> y >> cnt;		
			for (int j = 0; j < cnt; j++) {
				int index;
				cin >> index;
				supports[i].push_back(index);
			}
		}
		vector<vector<bool>> matrix(M, vector<bool>(M));
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < M; j++) {
				vector<int> common(M);
				auto iter = set_intersection(
					supports[i].begin(), supports[i].end(),
					supports[j].begin(), supports[j].end(),
					common.begin());
				matrix[i][j] = iter == common.begin();
				int sz = height / M;
				SDL_Rect rect = { Sint16(j*sz), Sint16(i*sz), Uint16(sz-1), Uint16(sz-1) };
				SDL_FillRect(screen, &rect, matrix[i][j] ? WHITE : RED);
			}
		}
		if (depth == 3) {
			SDL_Flip(screen);
			wait_until_key_or_quit(SDLK_SPACE);
			vector<vector<bool>> matrix2(M, vector<bool>(M));
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < M; j++) {
					matrix2[perm[i]][perm[j]] = matrix[i][j];
				}
			}
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < M; j++) {
					int sz = height / M;
					SDL_Rect rect = { Sint16(j*sz), Sint16(i*sz), Uint16(sz-1), Uint16(sz-1) };
					SDL_FillRect(screen, &rect, matrix2[i][j] ? WHITE : GREEN);
				}
			}
		}
	}

	SDL_Flip(screen);
	wait_until_key_or_quit(SDLK_ESCAPE);
	SDL_Quit();
	return 0;
}

