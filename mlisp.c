#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define XXX() x_undefined(__FILE__,__FUNCTION__,__LINE__)
#define new(t) ((t *)malloc(sizeof(t)))

#define TOKEN_DELTA 16

#define car(x) x->car
#define cdr(x) x->cdr
#define list2(x,y) cons(x, cons(y, 0))

typedef struct Cell {
  enum { CONS, SYMBOL } cell_type;
  union {
    struct {
      struct Cell *car;
      struct Cell *cdr;
    };
    char *symbol;
  };
} Cell;

char *token;
int token_size;
int token_index;

Cell *symbol_list;
Cell *environment;

Cell *quote_symbol;
Cell *t_symbol;

void x_undefined(char *file, char const *func, int line) {
  printf("%s() undefined in %s at line %d\n", func, file, line);
  exit(1);
}

Cell *atom(Cell *x) {
  if (x && x->cell_type != CONS)
    return t_symbol;
  return 0;
}

Cell *cons(Cell *a, Cell *d) {
  Cell *x = new(Cell);
  x->cell_type = CONS;
  x->car = a;
  x->cdr = d;
  return x;
}

Cell *eval_atom(Cell *f, Cell *e) {
  XXX();
  return 0;
}

Cell *eval_cons(Cell *f, Cell *e) {
  XXX();
  return 0;
}

Cell *eval(Cell *f, Cell *e) {
  if (f) {
    if (atom(f))
      return eval_atom(f, e);
    return eval_cons(f, e);
  }
  return 0;
}

Cell *read(void);
Cell *read_more(void);

Cell *print(Cell *x) {
  XXX();
  return x;
}

void expand_token() {
  token_size += TOKEN_DELTA;
  token = realloc(token, token_size);
}

void reset_token() {
  if (token_size == 0)
    expand_token();
  token_index = 0;
  token[0] = '\0';
}

void add_token(int c) {
  if (token_size <= token_index + 1)
    expand_token();
  token[token_index++] = c;
  token[token_index] = '\0';
}

void white_space() {
  int c;

  for (;;) {
    c = getchar();
    if (c == ';') {
      do {
	c = getchar();
      } while (c != '\n' && c != EOF);
    }
    else if (isgraph(c)) {
      ungetc(c, stdin);
      return;
    }
  }
}

void read_token() {
  int c;

  reset_token();
  white_space();
  c = getchar();
  add_token(c);
  if (strchr("()'", c))
    return;
  for (;;) {
    c = getchar();
    if (strchr("()", c) || !isgraph(c)) {
      ungetc(c, stdin);
      return;
    }
    add_token(c);
  }
}

Cell *new_symbol(char *s) {
  Cell *x = new(Cell);
  x->cell_type = SYMBOL;
  x->symbol = strdup(s);
  symbol_list = cons(x, symbol_list);
  return x;
}

Cell *read_symbol(Cell *list) {
  if (list == 0)
    return new_symbol(token);
  if (strcmp(car(list)->symbol, token) == 0)
    return car(list);
  return read_symbol(cdr(list));
}

Cell *read_list_more() {
  Cell *x;

  if (strcmp(token, ")") == 0)
    return 0;
  x = read_more();
  read_token();
  if (strcmp(token, ".") == 0) {
    x = cons(x, read());
    read_token();
    return x;
  }
  return cons(x, read_list_more());
}

Cell *read_list() {
  read_token();
  return read_list_more();
}

Cell *read_more() {
  if (strcmp(token, "(") == 0)
    return read_list();
  if (strcmp(token, "'") == 0)
    return list2(quote_symbol, read());
  return read_symbol(symbol_list);
}

Cell *read() {
  read_token();
  return read_more();
}

void repl() {
  for (;;) 
    print(eval(read(), environment));
}

void initialize_symbols() {
  quote_symbol = new_symbol("quote");
  t_symbol = new_symbol("t");
}

int main() {
  initialize_symbols();
  repl();
  return 0;
}
