#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define XXX() x_undefined(__FILE__,__FUNCTION__,__LINE__)
#define new(t) ((t *)malloc(sizeof(t)))

#define TOKEN_DELTA 16

#define caar(x) car(car(x))
#define cadr(x) car(cdr(x))
#define cdar(x) cdr(car(x))
#define cadar(x) car(cdr(car(x)))
#define caddr(x) car(cdr(cdr(x)))
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
int do_output;

Cell *symbol_list;
Cell *environment;

Cell *and_symbol;
Cell *atom_symbol;
Cell *car_symbol;
Cell *cdr_symbol;
Cell *cond_symbol;
Cell *cons_symbol;
Cell *eq_symbol;
Cell *label_symbol;
Cell *lambda_symbol;
Cell *list_symbol;
Cell *or_symbol;
Cell *quote_symbol;
Cell *rplaca_symbol;
Cell *rplacd_symbol;
Cell *set_symbol;
Cell *t_symbol;
Cell *end_of_file_symbol;

FILE *input;

void initialize_symbols(void);
void repl(void);

Cell *and(Cell *, Cell *);
Cell *or(Cell *, Cell *);
Cell *assoc(Cell *, Cell *);
Cell *passoc(Cell *, Cell *);
Cell *car(Cell *);
Cell *cdr(Cell *);
Cell *apply(Cell *, Cell *, Cell *);
Cell *eval(Cell *, Cell *);
Cell *eval_cons(Cell *, Cell *);
Cell *eval_list(Cell *, Cell *);
Cell *eval_cond(Cell *, Cell *);
Cell *pair_list(Cell *, Cell *, Cell *);
Cell *print(Cell *);
void print_list(Cell *);
Cell *lisp_read(void);
Cell *rplaca(Cell *, Cell*);
Cell *rplacd(Cell *, Cell *);
Cell *set(Cell *, Cell *);

int main(int argc, char **argv) {
  initialize_symbols();
  do_output = 0;
  input = fopen("stdlib.l", "r");
  if (input) {
    repl();
    fclose(input);
  }
  do_output = 1;
  switch (argc) {
  case 1:
    input = stdin;
    repl();
    break;
  case 2:
    input = fopen(argv[1], "r");
    if (input == 0) {
      printf("unable to open %s\n", argv[1]);
      return 1;
    }
    repl();
    break;
  default:
    printf("usage: mlisp [source.l]");
    return 1;
  }
  return 0;
}

void repl() {
  Cell *x;
  for (;;) {
    if (isatty(fileno(input)) && do_output) {
      printf("\n>");
      fflush(stdout);
    }
    x = lisp_read();
    if (x == end_of_file_symbol)
      break;
    x = eval(x, environment);
    if (do_output) {
      print(x);
      printf("\n");
    }
  }
}

void x_undefined(char *file, char const *func, int line) {
  printf("%s() undefined in %s at line %d\n", func, file, line);
  exit(1);
}

Cell *and(Cell *x, Cell *e) {
  if (x == 0)
    return t_symbol;
  if (eval(car(x), e) == 0)
    return 0;
  return and(cdr(x), e);
}

Cell *or(Cell *x, Cell *e) {
  if (x == 0)
    return 0;
  if (eval(car(x), e) != 0)
    return t_symbol;
  return or(cdr(x), e);
}

Cell *assoc(Cell *x, Cell *y) {
  if (y == 0)
    return 0;
  if (caar(y) == x)
    return cadar(y);
  return assoc(x, cdr(y));
}

Cell *passoc(Cell *x, Cell *y) {
  if (y == 0)
    return 0;
  if (caar(y) == x)
    return car(y);
  return passoc(x, cdr(y));
}

Cell *atom(Cell *x) {
  if (x && x->cell_type != CONS)
    return t_symbol;
  return 0;
}

Cell *car(Cell *x) {
  if (x && !atom(x))
    return x->car;
  printf("car of non list\n");
  return 0;
}

Cell *cdr(Cell *x) {
  if (x && !atom(x))
    return x->cdr;
  printf("cdr of non list\n");
  return 0;
}

Cell *cons(Cell *a, Cell *d) {
  Cell *x = new(Cell);
  x->cell_type = CONS;
  x->car = a;
  x->cdr = d;
  return x;
}

Cell *eval(Cell *f, Cell *e) {
  /*
  printf("(eval ");
  print(f);
  printf(" ");
  print(e);
  printf(")\n");
  fflush(stdout);
  */
  if (f) {
    if (atom(f))
      return assoc(f, e);
    return eval_cons(f, e);
  }
  return 0;
}

Cell *eval_cons(Cell *f, Cell *e) {
  if (atom(car(f))) {
    if (car(f) == and_symbol)
      return and(cdr(f), e);
    if (car(f) == or_symbol)
      return or(cdr(f), e);
    if (car(f) == cond_symbol)
      return eval_cond(cdr(f), e);
    if (car(f) == quote_symbol)
      return cadr(f);
    if (car(f) == lambda_symbol)
      return f;
    return apply(car(f), eval_list(cdr(f), e), e);
  }
  return apply(car(f), eval_list(cdr(f), e), e);
}

Cell *apply(Cell *fn, Cell *x, Cell *a) {
  /*
  printf("(apply ");
  print(fn);
  printf(" ");
  print(x);
  printf(" ");
  print(a);
  printf(")\n");
  fflush(stdout);
  */
  if (atom(fn)) {
    if (fn == atom_symbol)
      return atom(car(x)); 
    if (fn == car_symbol)
      return caar(x);
    if (fn == cdr_symbol)
      return cdar(x);
    if (fn == cons_symbol)
      return cons(car(x), cadr(x));
    if (fn == eq_symbol)
      return car(x) == cadr(x) ? t_symbol : 0;
    if (fn == list_symbol)
      return x;
    if (fn == rplaca_symbol)
      return rplaca(car(x), cadr(x));
    if (fn == rplacd_symbol)
      return rplacd(car(x), cadr(x));
    if (fn == set_symbol)
      return set(car(x), cadr(x));
    fn = eval(fn, a);
    if (fn == 0) {
      printf("function not defined\n");
      return 0;
    }
    return apply(fn, x, a);
  }
  if (car(fn) == label_symbol)
    return apply(caddr(fn), x, cons(cons(cadr(fn), caddr(fn)), a));
  if (car(fn) == lambda_symbol)
    return eval(caddr(fn), pair_list(cadr(fn), x, a));
  printf("illegal form\n");
  return 0;
}

Cell *pair_list(Cell *x, Cell *y, Cell *a) {
  if (x == 0)
    return a;
  return cons(list2(car(x), car(y)), pair_list(cdr(x), cdr(y), a));
}

Cell *eval_list(Cell *x, Cell *y) {
  if (x == 0)
    return 0;
  return cons(eval(car(x), y), eval_list(cdr(x), y));
}

Cell *eval_cond(Cell *c, Cell *e) {
  if (c == 0)
    return 0;
  if (eval(caar(c), e))
    return eval(cadar(c), e);
  return eval_cond(cdr(c), e);
}

Cell *read_more(void);

Cell *print(Cell *x) {
  if (x == 0)
    printf("nil");
  else if (atom(x))
    printf(x->symbol);
  else {
    printf("(");
    print_list(x);
    printf(")");
  }
  return x;
}

void print_list(Cell *x) {
  print(car(x));
  if (cdr(x) == 0)
    ;
  else if (atom(cdr(x))) {
    printf(" . ");
    print(cdr(x));
  }
  else {
    printf(" ");
    print_list(cdr(x));
  }
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

int white_space() {
  int c;
  
  for (;;) {
    c = getc(input);
    if (c == EOF)
      return c;
    if (c == ';') {
      do {
	c = getc(input);
      } while (c != '\n' && c != EOF);
      if (c == EOF)
	return c;
    }
    else if (isgraph(c)) {
      return c;
    }
  }
}

void read_token() {
  int c;

  reset_token();
  c = white_space();
  if (c == EOF)
    return;
  add_token(c);
  if (strchr("()'", c))
    return;
  for (;;) {
    c = getc(input);
    if (strchr("()", c) || !isgraph(c)) {
      ungetc(c, input);
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
  if (strcmp(token, "") == 0) {
    printf("missing ')' at end of file\n");
    exit(1);
  }
  if (strcmp(token, ".") == 0) {
    x = cons(x, lisp_read());
    read_token();
    if (strcmp(token, ")") != 0)
      printf("illegal dotted list\n");
    return x;
  }
  return cons(x, read_list_more());
}

Cell *read_list() {
  read_token();
  if (strcmp(token, "") == 0) {
    printf("missing ')' at end of file\n");
    exit(1);
  }
  return read_list_more();
}

Cell *read_more() {
  if (strcmp(token, "(") == 0)
    return read_list();
  if (strcmp(token, "'") == 0)
    return list2(quote_symbol, lisp_read());
  return read_symbol(symbol_list);
}

Cell *lisp_read() {
  read_token();
  if (strcmp(token, "") == 0)
    return end_of_file_symbol;
  return read_more();
}

Cell *rplaca(Cell *x, Cell *y) {
  if (x == 0 || atom(x))
    printf("rplaca not a cons\n");
  else
    x->car = y;
  return 0;
}

Cell *rplacd(Cell *x, Cell *y) {
  if (x == 0 || atom(x))
    printf("rplacd not a cons\n");
  else
    x->cdr = y;
  return 0;
}

Cell *set(Cell *x, Cell *y) {
  Cell *z = passoc(x, environment);
  if (z == 0)
    environment = cons(list2(x, y), environment);
  else {
    printf("->");
    print(z);
    printf("\n");
    rplaca(cdr(z), y);
  }
  return x;
}

void initialize_symbols() {
  and_symbol = new_symbol("and");
  atom_symbol = new_symbol("atom");
  car_symbol = new_symbol("car");
  cdr_symbol = new_symbol("cdr");
  cond_symbol = new_symbol("cond");
  cons_symbol = new_symbol("cons");
  eq_symbol = new_symbol("eq");
  label_symbol = new_symbol("label");
  lambda_symbol = new_symbol("lambda");
  list_symbol = new_symbol("list");
  or_symbol = new_symbol("or");
  quote_symbol = new_symbol("quote");
  rplaca_symbol = new_symbol("rplaca");
  rplacd_symbol = new_symbol("rplacd");
  set_symbol = new_symbol("set");
  t_symbol = new_symbol("t");

  end_of_file_symbol = new_symbol("end-of-file");
}

