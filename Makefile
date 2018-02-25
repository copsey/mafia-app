CXXFLAGS=-std=c++1z

src = \
	cli/main.cpp \
	interface/command.cpp \
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

objs = $(addprefix build/,$(src:.cpp=.o))


mafia: $(objs)
	$(CXX) $(CXXFLAGS) $(objs) -o mafia

$(objs): build/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@


clean:
	rm -r ./build
	rm ./mafia


.PHONY: clean
