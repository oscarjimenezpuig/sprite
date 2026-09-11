# ============================================================
# PROYECTO
# ============================================================

TARGET = sprite
SRCS   = sprite.c

# Archivo que almacena el tipo de compilación
# N = normal
# R = release
# D = debug
COMPILATION_FILE = compilation


# ============================================================
# COMPILADOR
# ============================================================

CC = gcc

CFLAGS = -Wall -Wextra
LDFLAGS =
LDLIBS = -lX11

# Ejemplos:
# LDLIBS += -lm
# LDLIBS += -lX11


# ============================================================
# LIBRERIA ESTATICA
# ============================================================

AR      = ar
ARFLAGS = rcs


# ============================================================
# TIPO DE COMPILACION
# ============================================================

MODE = N

ifeq ($(MAKECMDGOALS),release)
	MODE = R
	CFLAGS += -O3
endif

ifeq ($(MAKECMDGOALS),debug)
	MODE = D
	CFLAGS += -g -fsanitize=address,undefined
	LDFLAGS += -fsanitize=address,undefined
endif


# ============================================================
# OBJETOS
# ============================================================

OBJS = $(SRCS:.c=.o)


# ============================================================
# TARGETS
# ============================================================

.PHONY: all release debug package clean clear_screen


# ============================================================
# COMPILACION NORMAL
# ============================================================

all: clear_screen check_compilation $(TARGET)
	@echo "N" > $(COMPILATION_FILE)
	@echo "Normal compilation"


# ============================================================
# RELEASE
# ============================================================

release: clear_screen check_compilation $(TARGET)
	@echo "R" > $(COMPILATION_FILE)
	@echo "Release compilation"


# ============================================================
# DEBUG
# ============================================================

debug: clear_screen check_compilation $(TARGET)
	@echo "D" > $(COMPILATION_FILE)
	@echo "Debug compilation"


# ============================================================
# EJECUTABLE
# ============================================================

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)


# ============================================================
# OBJETOS
# ============================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# ============================================================
# LIBRERIA ESTATICA
# ============================================================

package: $(OBJS)
	$(AR) $(ARFLAGS) lib$(TARGET).a $(OBJS)


# ============================================================
# COMPROBAR TIPO DE COMPILACION
# ============================================================

check_compilation:
	@if [ ! -f $(COMPILATION_FILE) ]; then \
		echo "No previous compilation."; \
		rm -f $(OBJS) $(TARGET) lib$(TARGET).a; \
	elif [ "$$(cat $(COMPILATION_FILE))" != "$(MODE)" ]; then \
		echo "Compilation mode changed: $$(cat $(COMPILATION_FILE)) -> $(MODE)"; \
		rm -f $(OBJS) $(TARGET) lib$(TARGET).a; \
	else \
		echo "Compilation mode unchanged: $(MODE)"; \
	fi


# ============================================================
# LIMPIAR PANTALLA
# ============================================================

clear_screen:
	clear


# ============================================================
# LIMPIEZA
# ============================================================

clean:
	rm -f $(OBJS) $(TARGET) lib$(TARGET).a
