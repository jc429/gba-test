#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct struct_Vector2 {
	int x;
	int y;
} Vector2;

inline void vec2_set(Vector2 *v, int x, int y) { v->x = x; v->y = y; }

inline void vec2_clear(Vector2 *v) { v->x = 0; v->y = 0; }

inline void vec2_copy(Vector2 *dest, Vector2 *src) { dest->x = src->x; dest->y = src->y; }

#endif //VECTOR2_H