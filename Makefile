NAME		= codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -I.

SRCS		= main.c runner.c initializer.c heapers.c cleaners.c cyclers.c request.c \
              dongle-helpers.c general-helpers.c heap-helpers.c threads-helpers.c time-helpers.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re