src = \
	cli/main.cpp \
	interface/console.cpp \
	interface/events.cpp \
	interface/game_log.cpp \
	interface/help_screens.cpp \
	interface/names.cpp \
	interface/questions.cpp \
	interface/setup_screen.cpp \
	interface/styled_string.cpp \
	logic/game.cpp \
	logic/player.cpp \
	logic/role.cpp \
	logic/rulebook.cpp \
	logic/wildcard.cpp \
	riketi/random.cpp \
	riketi/string.cpp \


build:
	c++ -std=c++1z $(src) -o mafia

clean:
	rm -f ./mafia


.PHONY: clean
