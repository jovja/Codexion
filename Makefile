NAME		= codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -I.

SRCS		= main.c initializer.c cleaners.c helpers.c helpers2.c helpers3.c \
              heapers.c dongle-helpers.c dongles-cycle.c cyclers.c time-helpers.c

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